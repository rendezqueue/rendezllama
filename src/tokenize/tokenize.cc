#include <string>
#include <vector>

#include <fildesh/fildesh.h>
#include <fildesh/ofstream.hh>

#include "llama.h"

int main(int argc, char** argv)
{
  const char* count_filename = NULL;
  const char* model_filename = NULL;
  int exstatus = 0;
  int argi;
  for (argi = 1; exstatus == 0 && argi < argc; ++argi) {
    if (0 == strcmp("--model", argv[argi])) {
      argi += 1;
      if (argi < argc) {
        model_filename = argv[argi];
      }
    }
    else if (0 == strcmp("--o-count", argv[argi])) {
      argi += 1;
      if (argi < argc) {
        count_filename = argv[argi];
      }
    }
    else {
      exstatus = 64;
    }
    const char* arg = argv[argi];
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
    FildeshX* prompt_in = open_FildeshXF("-");
    if (prompt_in) {
      const char* content = slurp_FildeshX(prompt_in);
      if (content) {
        prompt += content;
      }
    }
    close_FildeshX(prompt_in);
  }

  llama_context_params lparams = llama_context_default_params();
  llama_context* ctx = llama_init_from_file(model_filename, lparams);

  std::vector<llama_token> tokens(prompt.size() + 1);
  int token_count = ::llama_tokenize(
      ctx, prompt.c_str(), tokens.data(), (int)prompt.size(), /*add_bos=*/true);

  fildesh::ofstream out(count_filename);
  if (token_count > 0) {
    out << token_count << '\n';
  }
  else {
    exstatus = 1;
  }
  return exstatus;
}
