#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "llama.h"

#include "src/language/vocabulary.hh"

using rendezllama::Vocabulary;

int main(int argc, char** argv)
{
  rendezllama::GlobalScope rendezllama_global_scope;
  const char* count_filename = "/dev/null";
  const char* model_filename = NULL;
  const char* prompt_filename = "-";
  const char* token_filename = "/dev/null";
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
    else if (0 == strcmp("-o", argv[argi])) {
      argi += 1;
      token_filename = argv[argi];
    }
    else {
      exstatus = 64;
    }
  }

  if (exstatus == 0 && !model_filename) {
    fildesh_log_error("Please provide a model file with --model.");
    exstatus = 64;
  }
  if (exstatus != 0) {
    return exstatus;
  }

  // Match original LLaMA tokenizer behavior.
  std::string prompt = " ";

  {
    std::string content;
    if (fildesh::slurp_file_to_string(content, prompt_filename)) {
      prompt += content;
    }
  }

  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_load_model_from_file(model_filename, model_params);

  std::vector<Vocabulary::Token_id> tokens;
  Vocabulary vocabulary(model);
  vocabulary.assign_substitution("<s>", vocabulary.bos_token_id());
  vocabulary.assign_substitution("</s>", vocabulary.eos_token_id());
  tokens.push_back(vocabulary.bos_token_id());
  vocabulary.tokenize_to(tokens, prompt);

  if (tokens.size() == 0) {
    exstatus = 1;
  }

  if (exstatus == 0) {
    fildesh::ofstream out(count_filename);
    out << tokens.size() << '\n';
  }
  if (exstatus == 0) {
    fildesh::ofstream out(token_filename);
    for (auto token_id : tokens) {
      if (token_id == vocabulary.newline_token_id()) {
        out << "\\n";
      }
      else {
        vocabulary.detokenize_to(out, token_id);
      }
      out << '\n';
    }
  }
  return exstatus;
}
