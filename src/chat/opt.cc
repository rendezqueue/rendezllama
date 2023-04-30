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

static
  void
print_options(std::ostream& out, const rendezllama::ChatOptions& opt)
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
    else if (0 == strcmp("--linespace", argv[argi])) {
      argi += 1;
      opt.linespace_on = parse_truthy(argv[argi]);
    }
    else if (0 == strcmp("--command_prefix_char", argv[argi])) {
      argi += 1;
      opt.command_prefix_char = argv[argi][0];
    }
    else if (0 == strcmp("--thread_count", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.thread_count, argv[argi]) || opt.thread_count <= 0) {
        fildesh_log_error("--thread_count needs positive arg");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--batch_count", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.batch_count, argv[argi]) || opt.batch_count <= 0) {
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
      argi += 1;
      if (!fildesh_parse_int(&opt.context_token_limit, argv[argi])) {
        fildesh_log_error("--context_token_limit needs int");
        exstatus = 64;
      }
      else if (opt.context_token_limit > 2048) {
        fildesh_log_warning("--context_token_limit is above 2048. Expect poor results.");
      }
    }
    else if (0 == strcmp("--sentence_limit", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.sentence_limit, argv[argi])) {
        fildesh_log_error("--sentence_limit needs int");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--sentence_token_limit", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.sentence_token_limit, argv[argi])) {
        fildesh_log_error("--sentence_token_limit needs int");
        exstatus = 64;
      }
    }
    // Original stuff.
    else if (0 == strcmp("--repeat_last_n", argv[argi]) ||
             0 == strcmp("--repeat_window", argv[argi]))
    {
      argi += 1;
      if (!fildesh_parse_int(&opt.repeat_last_count, argv[argi])) {
        fildesh_log_error("--repeat_window needs int");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--repeat_penalty", argv[argi])) {
      argi += 1;
      double f = 0;
      if (!fildesh_parse_double(&f, argv[argi])) {
        fildesh_log_error("--repeat_penalty needs float");
        exstatus = 64;
      }
      else {
        opt.repeat_penalty = f;
      }
    }
    else if (0 == strcmp("--temp", argv[argi])) {
      argi += 1;
      double f = 0;
      if (!fildesh_parse_double(&f, argv[argi])) {
        fildesh_log_error("--temp needs float");
        exstatus = 64;
      }
      else {
        opt.temp = f;
      }
    }
    else if (0 == strcmp("--top_k", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.top_k, argv[argi])) {
        fildesh_log_error("--top_k needs int");
        exstatus = 64;
      }
    }
    else if (0 == strcmp("--top_p", argv[argi])) {
      argi += 1;
      double f = 0;
      if (!fildesh_parse_double(&f, argv[argi])) {
        fildesh_log_error("--top_p needs float");
        exstatus = 64;
      }
      else {
        opt.top_p = f;
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
    ensure_linespace(opt.priming_prompt, opt.startspace_on, opt.linespace_on);
    ensure_linespace(opt.rolling_prompt, opt.linespace_on, opt.linespace_on);
    if (opt.linespace_on) {opt.rolling_prompt += ' ';}
    opt.rolling_prompt += opt.confidant + ':';
  }
  return exstatus;
}

  bool
rendezllama::maybe_parse_option_command(
    rendezllama::ChatOptions& opt,
    FildeshX* in,
    std::ostream& eout)
{
  if (skipstr_FildeshX(in, "repeat_last_n") ||
      skipstr_FildeshX(in, "repeat_last_count") ||
      skipstr_FildeshX(in, "repeat_window"))
  {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "repeat_window=" << opt.repeat_last_count << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n >= 0) {
      opt.repeat_last_count = n;
    }
    else {
      fildesh_log_warning("Need an int.");
    }
  }
  else if (skipstr_FildeshX(in, "repeat_penalty")) {
    double f = 0;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "repeat_penalty=" << opt.repeat_penalty << '\n'; eout.flush();
    }
    else if (parse_double_FildeshX(in, &f) && f >= 0) {
      opt.repeat_penalty = f;
    }
    else {
      fildesh_log_warning("Need a float.");
    }
  }
  else if (skipstr_FildeshX(in, "temp")) {
    double f = 0;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "temp=" << opt.temp << '\n'; eout.flush();
    }
    else if (parse_double_FildeshX(in, &f) && f >= 0) {
      opt.temp = f;
    }
    else {
      fildesh_log_warning("Need a float.");
    }
  }
  else if (skipstr_FildeshX(in, "top_k")) {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "top_k=" << opt.top_k << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n > 0) {
      opt.top_k = n;
    }
    else {
      fildesh_log_warning("Need an int.");
    }
  }
  else if (skipstr_FildeshX(in, "top_p")) {
    double f = 0;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "top_p=" << opt.top_p << '\n'; eout.flush();
    }
    else if (parse_double_FildeshX(in, &f) && f >= 0) {
      opt.top_p = f;
    }
    else {
      fildesh_log_warning("Need a float.");
    }
  }
  else if (skipstr_FildeshX(in, "thread_count")) {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "thread_count=" << opt.thread_count << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n > 0) {
      opt.thread_count = n;
    }
    else {
      fildesh_log_warning("Need a positive int.");
    }
  }
  else if (skipstr_FildeshX(in, "batch_count")) {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "batch_count=" << opt.batch_count << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n > 0) {
      opt.batch_count = n;
    }
    else {
      fildesh_log_warning("Need a positive int.");
    }
  }
  else if (skipstr_FildeshX(in, "mlock_on")) {
    int n = 0;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "mlock_on=" << (opt.mlock_on ? 1 : 0) << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n)) {
      opt.mlock_on = (n != 0);
    }
    else {
      fildesh_log_warning("Need a 1 or 0.");
    }
  }
  else if (skipstr_FildeshX(in, "mmap_on")) {
    int n = 0;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "mmap_on=" << (opt.mmap_on ? 1 : 0) << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n)) {
      opt.mmap_on = (n != 0);
    }
    else {
      fildesh_log_warning("Need a 1 or 0.");
    }
  }
  else if (skipstr_FildeshX(in, "sentence_limit")) {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "sentence_limit=" << opt.sentence_limit << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n >= 0) {
      opt.sentence_limit = n;
    }
    else {
      fildesh_log_warning("Need a non-negative int.");
    }
  }
  else if (skipstr_FildeshX(in, "sentence_token_limit"))
  {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "sentence_token_limit=" << opt.sentence_token_limit << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n >= 0) {
      opt.sentence_token_limit = n;
    }
    else {
      fildesh_log_warning("Need a non-negative int.");
    }
  }
  else if (skipstr_FildeshX(in, "opt")) {
    print_options(eout, opt);
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

  void
rendezllama::print_initialization(
    std::ostream& out,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt,
    const std::vector<llama_token>& tokens)
{
  if (opt.verbose_prompt && ctx && tokens.size() > 0) {
    out
      << "Number of tokens in priming prompt: " << opt.priming_token_count << "\n"
      << "Number of tokens in full prompt: " << tokens.size() << "\n";
    for (size_t i = 0; i < tokens.size(); i++) {
      out << tokens[i] << " -> '" << llama_token_to_str(ctx, tokens[i]) << "'\n";
    }
    out << "\n\n";
  }

  for (auto antiprompt : opt.antiprompts) {
    out << "Reverse prompt: " << antiprompt << "\n";
  }

  print_options(out, opt);
  out << "\n\n";
  out.flush();
}
