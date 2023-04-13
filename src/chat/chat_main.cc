#include <cassert>
#include <iostream>

#include <fildesh/ofstream.hh>

#include "src/chat/chat.hh"
#include "src/chat/opt.hh"
#include "src/tokenize/tokenize.hh"

int main(int argc, char** argv)
{
  fildesh::ofstream eout("/dev/stderr");
  fildesh::ofstream out("/dev/stdout");
  int exstatus = 0;
  rendezllama::ChatOptions opt;
  exstatus = parse_options(opt, argc, argv);
  if (exstatus != 0) {
    return exstatus;
  }

  llama_context* ctx = rendezllama::make_llama_context(opt);
  if (!ctx) {return 1;}

  // tokenize the prompt
  std::vector<llama_token> chat_tokens;
  chat_tokens.push_back(llama_token_bos());
  rendezllama::tokenize_extend(chat_tokens, ctx, opt.priming_prompt);
  // No need for --keep, we just directly compute the priming prompt number of tokens.
  opt.priming_token_count = chat_tokens.size();
  if (opt.priming_token_count < 0) {
    fildesh_log_error("bad priming tokenization");
    return 1;
  }
  rendezllama::tokenize_extend(chat_tokens, ctx, opt.rolling_prompt);
  rendezllama::print_initialization(eout, ctx, opt, chat_tokens);

  assert(opt.context_token_limit == llama_n_ctx(ctx));
  if ((int) chat_tokens.size() > opt.context_token_limit - 4) {
    fildesh_log_error("Prompt is longer than context_token_limit - 4.");
    return 1;
  }

  eout
    << "== Running in interactive mode. ==\n"
    << " - Press Return to return control to LLaMa.\n"
    << " - If you want to submit another line, end your input in '\\'.\n\n"
    ;
  eout.flush();


  int sentence_token_count = opt.sentence_token_limit;
  int context_token_count = 0;

  while (true) {
    context_token_count = rendezllama::commit_to_context(
        ctx, out, chat_tokens, context_token_count, opt);
    if (context_token_count == 0) {return 1;}
    assert(context_token_count == (int)chat_tokens.size());

    bool inputting = false;
    std::string matched_antiprompt;
    {
      llama_token id = rendezllama::generate_next_token(ctx, chat_tokens, opt);

      // add it to the context
      chat_tokens.push_back(id);

      // Display.
      const std::string s = llama_token_to_str(ctx, id);
      out << s;
      out.flush();

      // Check if each of the reverse prompts appears at the end of the output.
      // We use single-character antiprompts, so they aren't split across tokens.
      // (If we used longer antiprompts, they could be split across iterations.)
      matched_antiprompt = rendezllama::antiprompt_suffix(s, opt.antiprompts);
      if (!matched_antiprompt.empty()) {
        inputting = true;
      }

      // decrement remaining sampling budget
      if (sentence_token_count > 0) {sentence_token_count -= 1;}
    }

    // Max tokens.
    if (sentence_token_count == 0 && opt.sentence_token_limit > 0) {
      inputting = true;
    }

    if (inputting) {
      sentence_token_count = opt.sentence_token_limit;

      std::string buffer;

      std::string line;
      bool another_line = true;
      do {
        if (!std::getline(std::cin, line)) {
          llama_free(ctx);
          // input stream is bad or EOF received
          return 0;
        }
        if (line.empty() || line.back() != '\\') {
          another_line = false;
        } else {
          line.pop_back(); // Remove the continue character
        }
        if (buffer.empty()) {
          buffer = line;
        }
        else {
          buffer += '\n' + line;
        }
      } while (another_line);

      if (buffer.length() > 0) {
        rendezllama::augment_chat_input(buffer, matched_antiprompt, opt);
        rendezllama::tokenize_extend(chat_tokens, ctx, buffer);
      }
    }
  }

  llama_free(ctx);
  return exstatus;
}
