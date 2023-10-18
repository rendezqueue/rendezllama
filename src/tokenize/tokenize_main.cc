#include "tokenize.hh"

#include <fildesh/fildesh.h>
#include <fildesh/ofstream.hh>

#include "llama.h"

#include "src/language/vocabulary.hh"

int main(int argc, char** argv)
{
  rendezllama::GlobalScope rendezllama_global_scope;
  const char* count_filename = NULL;
  const char* model_filename = NULL;
  const char* prompt_filename = "-";
  int exstatus = 0;
  int argi;
  for (argi = 1; exstatus == 0 && argi < argc; ++argi) {
    if (argi + 1 == argc) {
      exstatus = 64;
    }
    else if (0 == strcmp("--model", argv[argi])) {
      argi += 1;
      model_filename = argv[argi];
    }
    else if (0 == strcmp("--x-prompt", argv[argi])) {
      argi += 1;
      prompt_filename = argv[argi];
    }
    else if (0 == strcmp("--o-count", argv[argi])) {
      argi += 1;
      count_filename = argv[argi];
    }
    else {
      exstatus = 64;
    }
  }

  if (exstatus == 0 && !model_filename) {
    fildesh_log_error("Please provide a model file with --model.");
    exstatus = 64;
  }
  if (exstatus == 0 && !count_filename) {
    fildesh_log_error("Please provide a count output file --o-count.");
    exstatus = 64;
  }
  if (exstatus != 0) {
    return exstatus;
  }

  // Match original LLaMA tokenizer behavior.
  std::string prompt = " ";

  {
    FildeshX* prompt_in = open_FildeshXF(prompt_filename);
    if (prompt_in) {
      const char* content = slurp_FildeshX(prompt_in);
      if (content) {
        prompt += content;
      }
    }
    close_FildeshX(prompt_in);
  }

  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_load_model_from_file(model_filename, model_params);

  llama_context_params ctx_params = llama_context_default_params();
  llama_context* ctx = llama_new_context_with_model(model, ctx_params);

  std::vector<llama_token> tokens;
  const rendezllama::Vocabulary vocabulary(ctx);
  tokens.push_back(vocabulary.bos_token_id());
  vocabulary.tokenize_to(tokens, prompt);

  fildesh::ofstream out(count_filename);
  if (tokens.size() > 0) {
    out << tokens.size() << '\n';
  }
  else {
    exstatus = 1;
  }
  return exstatus;
}
