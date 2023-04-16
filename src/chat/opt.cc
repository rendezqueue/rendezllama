#include "opt.hh"

#include <array>
#include <cstring>
#include <ctime>

#include <fildesh/fildesh.h>

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
    << ", sequent_token_limit=" << opt.sequent_token_limit
    << ", seed=" << opt.seed
    << '\n';
  out.flush();
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
      exstatus == 64;
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
    else if (0 == strcmp("--sequent_token_limit", argv[argi])) {
      argi += 1;
      if (!fildesh_parse_int(&opt.sequent_token_limit, argv[argi])) {
        fildesh_log_error("--sequent_token_limit needs int");
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
    opt.rolling_prompt += ' ' + opt.confidant + ':';
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
  else if (skipstr_FildeshX(in, "sequent_token_limit") ||
           skipstr_FildeshX(in, "sequent_limit"))
  {
    int n = -1;
    if (!skipchrs_FildeshX(in, opt.command_delim_chars)) {
      eout << "sequent_token_limit=" << opt.sequent_token_limit << '\n'; eout.flush();
    }
    else if (parse_int_FildeshX(in, &n) && n > 0) {
      opt.sequent_token_limit = n;
    }
    else {
      fildesh_log_warning("Need a positive int.");
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
