#include "opt.hh"

#include <array>
#include <cstring>
#include <ctime>

#include <fildesh/fildesh.h>
#include <fildesh/ofstream.hh>

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
    opt.rolling_prompt.insert(
        opt.rolling_prompt.end(),
        &slice.at[0], &slice.at[slice.size]);
    opt.rolling_prompt += '\n';

    slice = until_char_FildeshX(&slice, ':');
    if (slice.at) {
      skipchrs_FildeshX(&slice, " ");
      std::string name;
      name.insert(name.end(), &slice.at[slice.off], &slice.at[slice.size]);
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
  std::string
parse_quoted_string(FildeshX* in)
{
  std::string s;
  skipchrs_FildeshX(in, " ");
  if (peek_char_FildeshX(in, '"')) {
    in->off += 1;
    FildeshX slice = until_char_FildeshX(in, '"');
    if (slice.at) {
      s.insert(s.end(), &slice.at[slice.off], &slice.at[slice.size]);
    }
  }
  return s;
}

static
  bool
maybe_parse_bool_option(
    bool* b, FildeshX* in,
    const char* name)
{
  int tmp_b = 0;
  if (!skipstr_FildeshX(in, name)) {
    return false;
  }
  if (parse_int_FildeshX(in, &tmp_b)) {
    *b = (tmp_b != 0);
  }
  else {
    fildesh_log_warning("Need a 1 or 0.");
  }
  return true;
}

static
  bool
parse_options_sxproto(
    rendezllama::ChatOptions& opt,
    const std::string& filename)
{
  bool all_good = true;
  unsigned line_count = 0;
  FildeshX* in = open_FildeshXF(filename.c_str());
  if (!in) {
    fildesh_log_errorf("Cannot open %s.", filename.c_str());
    return false;
  }
  fildesh::ofstream nullout("/dev/null");
  FildeshX slice;
  for (slice = sliceline_FildeshX(in); slice.at;
       slice = sliceline_FildeshX(in))
  {
    line_count += 1;
    skipchrs_FildeshX(&slice, " ");
    if (peek_char_FildeshX(&slice, ';')) {
      // Line is comment. Do nothing.
    }
    else if (slice.off == slice.size) {
      // Line is empty. Do nothing.
    }
    else if (!skipstr_FildeshX(&slice, "(")) {
      fildesh_log_errorf(
          "Line %u of %s: Unrecognized char.\n",
          line_count, filename.c_str());
      all_good = false;
    }
    else if (skipstr_FildeshX(&slice, "context_token_limit ")) {
      int n = 0;
      if (parse_int_FildeshX(&slice, &n) && n > 0) {
        opt.context_token_limit = n;
      }
      else {
        fildesh_log_errorf(
            "Line %u of %s: Need positive int.\n",
            line_count, filename.c_str());
        all_good = false;
      }
    }
    else if (skipstr_FildeshX(&slice, "x_priming ")) {
      std::string priming_filename = parse_quoted_string(&slice);
      FildeshX* priming_in = NULL;
      if (!priming_filename.empty()) {
        priming_in = open_sibling_FildeshXF(
            filename.c_str(), priming_filename.c_str());
      }
      const char* content = NULL;
      if (priming_in) {
        content = slurp_FildeshX(priming_in);
      }
      if (!content) {
        fildesh_log_errorf(
            "Line %u of %s: Cannot read given file %s.\n",
            line_count, filename.c_str(), priming_filename.c_str());
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
    else if (skipstr_FildeshX(&slice, "x_answer ")) {
      std::string answer_filename = parse_quoted_string(&slice);
      FildeshX* answer_in = NULL;
      if (!answer_filename.empty()) {
        answer_in = open_sibling_FildeshXF(
            filename.c_str(), answer_filename.c_str());
      }
      const char* content = NULL;
      if (answer_in) {
        content = slurp_FildeshX(answer_in);
      }
      if (!content) {
        fildesh_log_errorf(
            "Line %u of %s: Cannot read given file %s.\n",
            line_count, filename.c_str(), answer_filename.c_str());
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
    else if (skipstr_FildeshX(&slice, "model ")) {
      opt.model_filename = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "lora ")) {
      opt.lora_filename = parse_quoted_string(&slice);
      opt.mmap_on = false;  // mmap() is incompatible.
    }
    else if (skipstr_FildeshX(&slice, "lora_base_model ") ||
             skipstr_FildeshX(&slice, "lora_base ")) {
      opt.lora_base_model_filename = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "x_rolling ")) {
      std::string rolling_filename = parse_quoted_string(&slice);
      FildeshX* rolling_in = NULL;
      if (!rolling_filename.empty()) {
        rolling_in = open_sibling_FildeshXF(
            filename.c_str(), rolling_filename.c_str());
      }
      parse_rolling_prompt(rolling_in, opt);
      close_FildeshX(rolling_in);
    }
    else if (skipstr_FildeshX(&slice, "o_rolling ")) {
      opt.transcript_sibling_filename = filename;
      opt.transcript_filename = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "protagonist ")) {
      opt.protagonist = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "confidant ")) {
      opt.confidant = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "template_protagonist ")) {
      opt.template_protagonist = parse_quoted_string(&slice);
    }
    else if (skipstr_FildeshX(&slice, "template_confidant ")) {
      opt.template_confidant = parse_quoted_string(&slice);
    }
    else if (
        maybe_parse_bool_option(&opt.linespace_on, &slice, "linespace_on") ||
        maybe_parse_bool_option(&opt.mlock_on, &slice, "mlock_on") ||
        maybe_parse_bool_option(&opt.mmap_on, &slice, "mmap_on")) {
      // Success!
    }
    else if (rendezllama::maybe_parse_option_command(opt, &slice, nullout)) {
      // Success!
    }
    else {
      fildesh_log_errorf(
          "Line %u of %s: Failed to parse field.\n",
          line_count, filename.c_str());
      all_good = false;
    }
  }
  close_FildeshX(in);
  return all_good;
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
      s.insert(s.end(), &slice.at[slice.off], &slice.at[slice.size]);
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
    << '\n'
    << "Sampling: temp=" << opt.temp
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

static bool parse_truthy(const char* arg) {
  return (
      0 == strcmp("true", arg) ||
      0 == strcmp("on", arg) ||
      0 == strcmp("1", arg));
}

  int
rendezllama::parse_options(rendezllama::ChatOptions& opt, int argc, char** argv)
{
  int exstatus = 0;
  int argi;

  opt.seed = time(NULL);

  opt.antiprompts.push_back("!");
  opt.antiprompts.push_back(".");
  opt.antiprompts.push_back("?");
  opt.antiprompts.push_back("…");
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
    else if (0 == strcmp("--template_protagonist", argv[argi])) {
      argi += 1;
      opt.template_protagonist = argv[argi];
    }
    else if (0 == strcmp("--template_confidant", argv[argi])) {
      argi += 1;
      opt.template_confidant = argv[argi];
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
    else if (0 == strcmp("--linespace_on", argv[argi])) {
      argi += 1;
      opt.linespace_on = parse_truthy(argv[argi]);
    }
    else if (0 == strcmp("--command_prefix_char", argv[argi])) {
      argi += 1;
      opt.command_prefix_char = argv[argi][0];
    }
    else if (0 == strcmp("--thread_count", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n >= 0) {
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
      if (fildesh_parse_int(&n, argv[argi]) && n >= 0) {
        opt.batch_count = n;
      }
      else {
        fildesh_log_error("--batch_count needs positive arg");
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
    else if (0 == strcmp("--context_token_limit", argv[argi])) {
      int n = 0;
      argi += 1;
      if (fildesh_parse_int(&n, argv[argi]) && n > 0) {
        opt.context_token_limit = n;
        if (opt.context_token_limit > 2048) {
          fildesh_log_warning("--context_token_limit is above 2048. Expect poor results.");
        }
      }
      else {
        fildesh_log_error("--context_token_limit needs positive arg");
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
  if (exstatus == 0 && opt.protagonist.empty()) {
    fildesh_log_error("Please provide a --protagonist name.");
    exstatus = 64;
  }
  if (exstatus == 0 && opt.confidant.empty()) {
    fildesh_log_error("Please provide a --confidant name.");
    exstatus = 64;
  }
  if (exstatus == 0 && opt.priming_prompt.empty()) {
    fildesh_log_error("Please provide a priming prompt with --x_priming.");
    exstatus = 64;
  }
  if (exstatus == 0 && opt.rolling_prompt.empty()) {
    fildesh_log_error("Please provide a rolling prompt with --x_rolling.");
    exstatus = 64;
  }
  if (exstatus == 0) {
    if (!opt.template_protagonist.empty()) {
      replace_in_prompts(opt, opt.template_protagonist, opt.protagonist);
    }
    if (!opt.template_confidant.empty()) {
      replace_in_prompts(opt, opt.template_confidant, opt.confidant);
    }
    ensure_linespace(opt.priming_prompt, opt.startspace_on, opt.linespace_on);
    ensure_linespace(opt.rolling_prompt, opt.linespace_on, opt.linespace_on);
    ensure_linespace(opt.answer_prompt, opt.linespace_on, opt.linespace_on);
    if (opt.linespace_on) {opt.rolling_prompt += ' ';}
    opt.rolling_prompt += opt.confidant + ':';
  }
  return exstatus;
}

static
  bool
maybe_parse_float(
    float* f, FildeshX* in, std::ostream& out,
    const char* name,
    const char* command_delim_chars)
{
  double tmp_f = 0;
  if (!skipstr_FildeshX(in, name)) {
    return false;
  }
  if (!skipchrs_FildeshX(in, command_delim_chars)) {
    out << name << "=" << *f << '\n'; out.flush();
  }
  else if (parse_double_FildeshX(in, &tmp_f) && tmp_f >= 0) {
    *f = (float) tmp_f;
  }
  else {
    fildesh_log_warning("Need a float.");
  }
  return true;
}

static
  bool
maybe_parse_nat(
    unsigned* n, FildeshX* in, std::ostream& out,
    const char* name,
    const char* command_delim_chars)
{
  int tmp_n = 0;
  if (!skipstr_FildeshX(in, name)) {
    return false;
  }
  if (!skipchrs_FildeshX(in, command_delim_chars)) {
    out << name << "=" << *n << '\n'; out.flush();
  }
  else if (parse_int_FildeshX(in, &tmp_n) && tmp_n >= 0) {
    *n = (unsigned) tmp_n;
  }
  else {
    fildesh_log_warning("Need a non-negative int.");
  }
  return true;
}

static
  bool
maybe_parse_positive(
    unsigned* n, FildeshX* in, std::ostream& out,
    const char* name,
    const char* command_delim_chars)
{
  int tmp_n = 0;
  if (!skipstr_FildeshX(in, name)) {
    return false;
  }
  if (!skipchrs_FildeshX(in, command_delim_chars)) {
    out << name << "=" << *n << '\n'; out.flush();
  }
  else if (parse_int_FildeshX(in, &tmp_n) && tmp_n > 0) {
    *n = (unsigned) tmp_n;
  }
  else {
    fildesh_log_warning("Need a positive int.");
  }
  return true;
}

  bool
rendezllama::maybe_parse_option_command(
    rendezllama::ChatOptions& opt,
    FildeshX* in,
    std::ostream& out)
{
  const char* const delims = opt.command_delim_chars;
  if (
      maybe_parse_float(&opt.frequency_penalty, in, out, "frequency_penalty", delims) ||
      maybe_parse_float(&opt.presence_penalty, in, out, "presence_penalty", delims) ||
      maybe_parse_float(&opt.repeat_penalty, in, out, "repeat_penalty", delims) ||
      maybe_parse_nat(&opt.repeat_last_count, in, out, "repeat_window", delims) ||
      maybe_parse_nat(&opt.repeat_last_count, in, out, "repeat_last_n", delims) ||
      maybe_parse_nat(&opt.repeat_last_count, in, out, "repeat_last_count", delims)) {
    // Success!
  }
  else if (
      maybe_parse_positive(&opt.top_k, in, out, "top_k", delims) ||
      maybe_parse_float(&opt.top_p, in, out, "top_p", delims) ||
      maybe_parse_float(&opt.tfs_z, in, out, "tfs_z", delims) ||
      maybe_parse_float(&opt.typical_p, in, out, "typical_p", delims) ||
      maybe_parse_float(&opt.temp, in, out, "temp", delims)) {
    // Success!
  }
  else if (maybe_parse_nat(&opt.mirostat_sampling, in, out, "mirostat", delims)) {
    if (opt.mirostat_sampling > 2) {
      fildesh_log_error("Mirostat must be <= 2. Resetting to 0 (off).");
      opt.mirostat_sampling = 0;
    }
  }
  else if (
      maybe_parse_float(&opt.mirostat_tau, in, out, "mirostat_tau", delims) ||
      maybe_parse_float(&opt.mirostat_eta, in, out, "mirostat_eta", delims)) {
    // Success!
  }
  else if (
      maybe_parse_positive(&opt.thread_count, in, out, "thread_count", delims) ||
      maybe_parse_positive(&opt.batch_count, in, out, "batch_count", delims)) {
    // Success!
  }
  else if (
      maybe_parse_nat(&opt.sentence_limit, in, out, "sentence_limit", delims) ||
      maybe_parse_nat(&opt.sentence_token_limit, in, out, "sentence_token_limit", delims)) {
    // Success!
  }
  else if (skipstr_FildeshX(in, "opt")) {
    print_options(out, opt);
  }
  else {
    return false;
  }
  return true;
}

  struct llama_context*
rendezllama::make_llama_context(const rendezllama::ChatOptions& opt)
{
  llama_context_params params = llama_context_default_params();
  params.n_ctx = opt.context_token_limit;
  params.n_parts = -1;
  params.seed = opt.seed;
  params.f16_kv = true;
  params.use_mlock = opt.mlock_on;
  params.use_mmap = opt.mmap_on;

  struct llama_context* ctx = llama_init_from_file(
      opt.model_filename.c_str(), params);
  if (!ctx) {
    fildesh_log_error("Failed to open model.");
  }
  return ctx;
}

