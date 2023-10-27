#include "opt.hh"

#include <array>
#include <cassert>
#include <cstring>
#include <ctime>

#include <fildesh/ostream.hh>
#include <fildesh/string.hh>
#include <fildesh/sxproto.h>

#include "src/chat/opt_schema.hh"

using rendezllama::ChatOptions;

static
  void
parse_rolling_prompt(FildeshX* in, rendezllama::ChatOptions& opt)
{
  std::array<std::string, 2> names;
  FildeshX slice;
  if (!in) {return;}
  for (slice = sliceline_FildeshX(in); slice.at;
       slice = sliceline_FildeshX(in))
  {
    opt.rolling_prompt += fildesh::make_string_view(slice);
    opt.rolling_prompt += '\n';

    slice = until_char_FildeshX(&slice, ':');
    if (slice.at) {
      skipchrs_FildeshX(&slice, " ");
      std::string name;
      name += fildesh::make_string_view(slice);
      if (name != names.back()) {
        names.front() = names.back();
        names.back() = name;
      }
    }
  }
  if (opt.protagonist.empty()) {
    opt.protagonist = names.back();
  }
  if (opt.confidant.empty()) {
    opt.confidant = names.front();
  }
}

static
  void
string_replace(
    std::string& text,
    const std::string& s,
    const std::string& r)
{
  std::string dst;
  size_t b = 0;
  for (size_t i = text.find(s, 0); i != std::string::npos; i = text.find(s, b)) {
    dst.append(text, b, i-b);
    dst.append(r);
    b = i + s.size();
  }
  dst.append(text, b, text.size()-b);
  text = dst;
}

static
  void
replace_in_prompts(
    rendezllama::ChatOptions& opt,
    const std::string& s,
    const std::string& r)
{
  string_replace(opt.priming_prompt, s, r);
  string_replace(opt.rolling_prompt, s, r);
  string_replace(opt.answer_prompt, s, r);
}

static
  void
ensure_linespace(std::string& s, bool startspace_on, bool linespace_on)
{
  if (!startspace_on && !linespace_on) {
    return;
  }

  FildeshX* in = open_FildeshXA();
  memcpy(grow_FildeshX(in, s.size()), &s[0], s.size());
  s = startspace_on ? " " : "";

  FildeshX slice;
  for (slice = sliceline_FildeshX(in); slice.at;
       slice = sliceline_FildeshX(in))
  {
    if (slice.size == 0) {
      s += '\n';
    }
    else {
      if (linespace_on && peek_char_FildeshX(&slice, ' ')) {
        slice.off += 1;
      }
      s += fildesh::make_string_view(slice);
      s += '\n';
      if (linespace_on) {
        s += ' ';
      }
    }
  }
  close_FildeshX(in);
  if (!s.empty() && linespace_on) {
    s.pop_back();
  }
}

  void
rendezllama::print_options(std::ostream& out, const rendezllama::ChatOptions& opt)
{
  out
    << "Characters: protagonist=" << opt.protagonist
    << ", confidant=" << opt.confidant
    << '\n';
  out << "Chat lines start with...\n";
  for (unsigned i = 0; i < opt.chat_prefixes.size(); ++i) {
    out << opt.chat_prefixes[i] << '\n';
  }
  out << '\n';
  out
    << "Sampling: temperature=" << opt.temperature
    << ", top_k=" << opt.top_k
    << ", top_p=" << opt.top_p
    << ", repeat_window=" << opt.repeat_last_count
    << ", repeat_penalty=" << opt.repeat_penalty
    << '\n'
    << "Generate: batch_count=" << opt.batch_count
    << ", thread_count=" << opt.thread_count
    << ", sentence_token_limit=" << opt.sentence_token_limit
    << ", sentence_limit=" << opt.sentence_limit
    << ", seed=" << opt.seed
    << '\n';
  out.flush();
}

static void reinitialize_chat_prefixes(ChatOptions& opt) {
  opt.chat_prefixes = opt.given_chat_prefixes;
  for (unsigned i = 0; i < opt.chat_prefixes.size(); ++i) {
    if (!opt.protagonist_alias.empty()) {
      string_replace(opt.chat_prefixes[i], opt.protagonist_alias, opt.protagonist);
    }
    if (!opt.confidant_alias.empty()) {
      string_replace(opt.chat_prefixes[i], opt.confidant_alias, opt.confidant);
    }
  }
  if (opt.chat_prefixes.size() < 2) {
    opt.given_chat_prefixes.clear();
    opt.chat_prefixes.clear();
    opt.chat_prefixes.resize(2);
    if (opt.linespace_on) {
      opt.chat_prefixes[0] += ' ';
      opt.chat_prefixes[1] += ' ';
    }
    opt.chat_prefixes[0] += opt.protagonist + ':';
    opt.chat_prefixes[1] += opt.confidant + ':';
  }
}

static int initialize_options(ChatOptions& opt) {
  int exstatus = 0;
  if (exstatus == 0 && opt.context_token_limit == 0) {
    opt.context_token_limit = opt.model_token_limit;
  }
  if (exstatus == 0 &&
      opt.given_chat_prefixes.size() < 2 &&
      !opt.coprocess_mode_on)
  {
    if (opt.protagonist.empty()) {
      fildesh_log_error("Please provide a --protagonist name.");
      exstatus = 64;
    }
    if (opt.confidant.empty()) {
      fildesh_log_error("Please provide a --confidant name.");
      exstatus = 64;
    }
  }
  if (exstatus == 0 && opt.priming_prompt.empty()) {
    fildesh_log_error("Please provide a priming prompt with --x_priming.");
    exstatus = 64;
  }
  if (exstatus == 0) {
    if (!opt.protagonist_alias.empty()) {
      replace_in_prompts(opt, opt.protagonist_alias, opt.protagonist);
    }
    if (!opt.confidant_alias.empty()) {
      replace_in_prompts(opt, opt.confidant_alias, opt.confidant);
    }
    ensure_linespace(opt.priming_prompt, opt.startspace_on, opt.linespace_on);
    ensure_linespace(opt.rolling_prompt, opt.linespace_on, opt.linespace_on);
    ensure_linespace(opt.answer_prompt, opt.linespace_on, opt.linespace_on);
    reinitialize_chat_prefixes(opt);
  }
  return exstatus;
}

static
  bool
parse_options_sxproto(ChatOptions& opt, const std::string& filename)
{
  bool all_good = true;
  FildeshX* in = open_FildeshXF(filename.c_str());
  if (!in) {
    fildesh_log_errorf("Cannot open %s.", filename.c_str());
    return false;
  }
  all_good = rendezllama::parse_sxpb_options(
      opt, in, rendezllama::options_sxproto_schema(), filename);
  return all_good;
}

  int
rendezllama::parse_initialize_options_sxproto(
    rendezllama::ChatOptions& opt,
    const std::string& filename)
{
  int exstatus = 0;
  if (!parse_options_sxproto(opt, filename)) {
    exstatus = 1;
  }
  if (exstatus == 0) {
    exstatus = initialize_options(opt);
  }
  return exstatus;
}

  int
rendezllama::parse_options(rendezllama::ChatOptions& opt, int argc, char** argv)
{
  int exstatus = 0;
  int argi;

  opt.seed = INT_MAX & time(NULL);

  opt.antiprompts = opt.sentence_terminals;
  opt.antiprompts.push_back("\n");

  for (argi = 1; exstatus == 0 && argi < argc; ++argi) {
    if (false) {
    }
    else if (argi + 1 == argc) {
      exstatus = 64;
    }
    else if (0 == strcmp("--protagonist", argv[argi])) {
      argi += 1;
      opt.protagonist = argv[argi];
    }
    else if (0 == strcmp("--confidant", argv[argi])) {
      argi += 1;
      opt.confidant = argv[argi];
    }
    else if (0 == strcmp("--model", argv[argi])) {
      argi += 1;
      opt.model_filename = argv[argi];
    }
    else if (0 == strcmp("--lora", argv[argi])) {
      argi += 1;
      opt.lora_filename = argv[argi];
      opt.mmap_on = false;  // mmap() is incompatible.
    }
    else if (0 == strcmp("--lora_base_model", argv[argi]) ||
             0 == strcmp("--lora_base", argv[argi]) ||
             0 == strcmp("--lora-base", argv[argi])) {
      argi += 1;
      opt.lora_base_model_filename = argv[argi];
    }
    else if (0 == strcmp("--x_setting", argv[argi])) {
      argi += 1;
      if (!parse_options_sxproto(opt, argv[argi])) {
        exstatus = 1;
      }
    }
    else if (0 == strcmp("--x_priming", argv[argi])) {
      argi += 1;
      FildeshX* priming_in = open_FildeshXF(argv[argi]);
      const char* content = slurp_FildeshX(priming_in);
      if (content && content[0] != '\0') {
        opt.priming_prompt += content;
        // Ensure newline at end.
        if (opt.priming_prompt.back() != '\n') {
          opt.priming_prompt += '\n';
        }
      }
      close_FildeshX(priming_in);
    }
    else if (0 == strcmp("--x_rolling", argv[argi])) {
      argi += 1;
      FildeshX* rolling_in = open_FildeshXF(argv[argi]);
      parse_rolling_prompt(rolling_in, opt);
      close_FildeshX(rolling_in);
    }
    else if (0 == strcmp("--o_rolling", argv[argi])) {
      argi += 1;
      opt.transcript_sibling_filename.clear();
      opt.transcript_filename = argv[argi];
    }
    else if (0 == strcmp("--x_answer", argv[argi])) {
      argi += 1;
      FildeshX* answer_in = open_FildeshXF(argv[argi]);
      const char* content = slurp_FildeshX(answer_in);
      if (content && content[0] != '\0') {
        opt.answer_prompt += content;
        // Ensure newline at end.
        if (opt.answer_prompt.back() != '\n') {
          opt.answer_prompt += '\n';
        }
      }
      close_FildeshX(answer_in);
    }
    else if (0 == strcmp("--command_prefix_char", argv[argi])) {
      argi += 1;
      opt.command_prefix_char = argv[argi][0];
    }
    else if (0 == strcmp("--thread_count", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n > 0) {
        opt.thread_count = n;
      }
      else {
        fildesh_log_error("--thread_count needs positive arg");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--batch_count", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n > 0) {
        opt.batch_count = n;
      }
      else {
        fildesh_log_error("--batch_count needs positive arg");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--seed", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n >= 0) {
        opt.seed = n;
      }
      else {
        fildesh_log_error("--seed needs non-negative int");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--coprocess_mode_on", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi])) {
        opt.coprocess_mode_on = (n != 0);
      }
      else {
        fildesh_log_error("--coprocess_mode_on needs 1 or 0");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--mlock_on", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi])) {
        opt.mlock_on = (n != 0);
      }
      else {
        fildesh_log_error("--mlock_on needs 1 or 0");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--mmap_on", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi])) {
        opt.mmap_on = (n != 0);
      }
      else {
        fildesh_log_error("--mmap_on needs 1 or 0");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--model_token_limit", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n > 0) {
        opt.model_token_limit = n;
      }
      else {
        fildesh_log_error("--model_token_limit needs positive arg");
        exstatus = 64;
      }
    }
    else {
      exstatus = 64;
    }
  }

  if (exstatus == 0 && opt.model_filename.empty()) {
    fildesh_log_error("Please provide a model file with --model.");
    exstatus = 64;
  }
  if (exstatus == 0) {
    exstatus = initialize_options(opt);
  }
  return exstatus;
}

static
  bool
lone_subfield_at_FildeshSxpb_to_cc_string(
    std::string* s, const FildeshSxpb* sxpb, FildeshSxpbIT it, const char* name)
{
  const char* tmp = NULL;
  if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, it, name)) {
    *s = tmp;
    return true;
  }
  return false;
}

/** Closes `in`.**/
  bool
rendezllama::parse_sxpb_options(
    ChatOptions& opt,
    FildeshX* in,
    const FildeshSxprotoField* schema,
    const std::string& filename)
{
  FildeshO* err_out = open_FildeshOF("/dev/stderr");
  const char* s = NULL;
  bool all_good = true;

  FildeshSxpb* const sxpb = slurp_sxpb_close_FildeshX(in, schema, err_out);
  if (!sxpb) {
    close_FildeshO(err_out);
    return false;
  }

  const FildeshSxpbIT top_it = top_of_FildeshSxpb(sxpb);
  FildeshSxpbIT it;

  lone_subfield_at_FildeshSxpb_to_unsigned(
      &opt.context_token_limit, sxpb, top_it, "context_token_limit");

  lone_subfield_at_FildeshSxpb_to_unsigned(
      &opt.model_token_limit, sxpb, top_it, "model_token_limit");

  if (lone_subfield_at_FildeshSxpb_to_str(&s, sxpb, top_it, "x_priming")) {
    FildeshX* priming_in = open_sibling_FildeshXF(filename.c_str(), s);
    const char* content = NULL;
    if (priming_in) {
      content = slurp_FildeshX(priming_in);
    }
    if (!content) {
      puts_FildeshO(err_out, "Cannot read given x_priming file: ");
      puts_FildeshO(err_out, s);
      putc_FildeshO(err_out, '\n');
      all_good = false;
    }
    else if (content[0] != '\0') {
      opt.priming_prompt += content;
      // Ensure newline at end.
      if (opt.priming_prompt.back() != '\n') {
        opt.priming_prompt += '\n';
      }
    }
    close_FildeshX(priming_in);
  }

  if (lone_subfield_at_FildeshSxpb_to_str(&s, sxpb, top_it, "x_answer")) {
    FildeshX* answer_in = open_sibling_FildeshXF(filename.c_str(), s);
    const char* content = NULL;
    if (answer_in) {
      content = slurp_FildeshX(answer_in);
    }
    if (!content) {
      puts_FildeshO(err_out, "Cannot read given x_answer file: ");
      puts_FildeshO(err_out, s);
      putc_FildeshO(err_out, '\n');
      all_good = false;
    }
    else if (content[0] != '\0') {
      opt.answer_prompt = content;
      // Ensure newline at end.
      if (opt.answer_prompt.back() != '\n') {
        opt.answer_prompt += '\n';
      }
    }
    close_FildeshX(answer_in);
  }

  lone_subfield_at_FildeshSxpb_to_cc_string(
      &opt.model_filename, sxpb, top_it, "model");

  if (lone_subfield_at_FildeshSxpb_to_str(&s, sxpb, top_it, "lora")) {
    opt.lora_filename = s;
    opt.mmap_on = false;  // mmap() is incompatible.
  }

  lone_subfield_at_FildeshSxpb_to_cc_string(
      &opt.lora_base_model_filename, sxpb, top_it, "lora_base_model");

  if (lone_subfield_at_FildeshSxpb_to_str(&s, sxpb, top_it, "x_rolling")) {
    FildeshX* rolling_in = open_sibling_FildeshXF(filename.c_str(), s);
    parse_rolling_prompt(rolling_in, opt);
    close_FildeshX(rolling_in);
  }
  if (lone_subfield_at_FildeshSxpb_to_str(&s, sxpb, top_it, "o_rolling")) {
    opt.transcript_sibling_filename = filename;
    opt.transcript_filename = s;
  }

  if (lone_subfield_at_FildeshSxpb_to_cc_string(&opt.protagonist, sxpb, top_it, "protagonist")) {
    if (filename.empty()) {
      reinitialize_chat_prefixes(opt);
    }
  }
  if (lone_subfield_at_FildeshSxpb_to_cc_string(&opt.confidant, sxpb, top_it, "confidant")) {
    if (filename.empty()) {
      reinitialize_chat_prefixes(opt);
    }
  }

  it = lookup_subfield_at_FildeshSxpb(sxpb, top_it, "substitution");
  if (!nullish_FildeshSxpbIT(it)) {
    FildeshSxpbIT sub_it;
    lone_subfield_at_FildeshSxpb_to_cc_string(&opt.protagonist_alias, sxpb, it, "protagonist_alias");
    lone_subfield_at_FildeshSxpb_to_cc_string(&opt.confidant_alias, sxpb, it, "confidant_alias");
  }

  it = lookup_subfield_at_FildeshSxpb(sxpb, top_it, "chat_prefixes");
  if (!nullish_FildeshSxpbIT(it)) {
    opt.given_chat_prefixes.clear();
    for (it = first_at_FildeshSxpb(sxpb, it); !nullish_FildeshSxpbIT(it);
         it = next_at_FildeshSxpb(sxpb, it)) {
      std::string given_prefix;
      if (!name_at_FildeshSxpb(sxpb, it)) {
        given_prefix = str_value_at_FildeshSxpb(sxpb, it);
      }
      else {
        assert(0 == strcmp(name_at_FildeshSxpb(sxpb, it), "m"));
        lone_subfield_at_FildeshSxpb_to_cc_string(
            &given_prefix, sxpb, it, "prefix");
      }
      opt.given_chat_prefixes.push_back(given_prefix);
    }
    if (filename.empty()) {
      reinitialize_chat_prefixes(opt);
    }
  }

  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.seed, sxpb, top_it, "seed");
  lone_subfield_at_FildeshSxpb_to_bool(&opt.coprocess_mode_on, sxpb, top_it, "coprocess_mode_on");
  lone_subfield_at_FildeshSxpb_to_bool(&opt.startspace_on, sxpb, top_it, "startspace_on");
  lone_subfield_at_FildeshSxpb_to_bool(&opt.linespace_on, sxpb, top_it, "linespace_on");
  lone_subfield_at_FildeshSxpb_to_bool(&opt.mlock_on, sxpb, top_it, "mlock_on");
  lone_subfield_at_FildeshSxpb_to_bool(&opt.mmap_on, sxpb, top_it, "mmap_on");

  /** Command option??*/
  lone_subfield_at_FildeshSxpb_to_float(&opt.frequency_penalty, sxpb, top_it, "frequency_penalty");
  lone_subfield_at_FildeshSxpb_to_float(&opt.presence_penalty, sxpb, top_it, "presence_penalty");
  lone_subfield_at_FildeshSxpb_to_float(&opt.repeat_penalty, sxpb, top_it, "repeat_penalty");
  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.repeat_last_count, sxpb, top_it, "repeat_last_count");
  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.top_k, sxpb, top_it, "top_k");
  lone_subfield_at_FildeshSxpb_to_float(&opt.top_p, sxpb, top_it, "top_p");
  lone_subfield_at_FildeshSxpb_to_float(&opt.tfs_z, sxpb, top_it, "tfs_z");
  lone_subfield_at_FildeshSxpb_to_float(&opt.typical_p, sxpb, top_it, "typical_p");
  lone_subfield_at_FildeshSxpb_to_float(&opt.temperature, sxpb, top_it, "temperature");

  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.mirostat_sampling, sxpb, top_it, "mirostat");
  lone_subfield_at_FildeshSxpb_to_float(&opt.mirostat_tau, sxpb, top_it, "mirostat_tau");
  lone_subfield_at_FildeshSxpb_to_float(&opt.mirostat_eta, sxpb, top_it, "mirostat_eta");

  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.thread_count, sxpb, top_it, "thread_count");
  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.batch_count, sxpb, top_it, "batch_count");
  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.sentence_limit, sxpb, top_it, "sentence_limit");
  lone_subfield_at_FildeshSxpb_to_unsigned(&opt.sentence_token_limit, sxpb, top_it, "sentence_token_limit");

  it = lookup_subfield_at_FildeshSxpb(sxpb, top_it, "sentence_terminals");
  if (!nullish_FildeshSxpbIT(it)) {
    opt.sentence_terminals.clear();
    bool found = false;
    for (it = first_at_FildeshSxpb(sxpb, it); !nullish_FildeshSxpbIT(it);
         it = next_at_FildeshSxpb(sxpb, it)) {
      s = str_value_at_FildeshSxpb(sxpb, it);
      opt.sentence_terminals.push_back(s);
      if (s[0] == '\n' && s[1] == '\0') {found = true;}
    }

    opt.multiline_confidant_on = found;
    opt.antiprompts = opt.sentence_terminals;
    if (!found) {
      opt.antiprompts.push_back("\n");
    }
  }

  close_FildeshO(err_out);
  close_FildeshSxpb(sxpb);

  return all_good;
}


  bool
rendezllama::parse_dynamic_sxpb_options(
    rendezllama::ChatOptions& opt,
    FildeshX* in)
{
  return rendezllama::parse_sxpb_options(
      opt, in, dynamic_options_sxproto_schema(), "");
}
