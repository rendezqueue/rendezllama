#include <cassert>

#include <fildesh/ofstream.hh>

#include "src/chat/chat.hh"
#include "src/chat/opt.hh"
#include "src/tokenize/tokenize.hh"

int main(int argc, char** argv)
{
  fildesh::ofstream eout("/dev/stderr");
  fildesh::ofstream out("/dev/stdout");
  FildeshX* in = NULL;
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

  std::vector<llama_token> extra_penalized_tokens;
  unsigned sequent_token_count = opt.sequent_token_limit;
  unsigned context_token_count = 0;

  in = open_FildeshXF("/dev/stdin");
  while (true) {
    context_token_count = rendezllama::commit_to_context(
        ctx, out, chat_tokens, context_token_count, opt);
    if (context_token_count == 0) {return 1;}
    assert(context_token_count == (int)chat_tokens.size());

    bool inputting = false;
    std::string matched_antiprompt;
    {
      llama_token id = rendezllama::generate_next_token(
          ctx, extra_penalized_tokens, chat_tokens, opt);

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
      if (sequent_token_count > 0) {sequent_token_count -= 1;}
    }

    // Max tokens.
    if (sequent_token_count == 0 && opt.sequent_token_limit > 0) {
      inputting = true;
    }

    if (inputting) {
      sequent_token_count = opt.sequent_token_limit;

      std::string buffer;

      FildeshX slice;
      for (slice = sliceline_FildeshX(in); slice.at;
           slice = sliceline_FildeshX(in))
      {
        if (slice.size == 0) {break;}

        if (!peek_char_FildeshX(&slice, opt.command_prefix_char)) {
          if (slice.at[slice.size-1] == '\\') {
            // Overwrite the continue character.
            slice.at[slice.size-1] = '\n';
            buffer.insert(buffer.end(), slice.at, &slice.at[slice.size]);
            continue;
          }
          buffer.insert(buffer.end(), slice.at, &slice.at[slice.size]);
          break;
        }

        slice.off += 1;
        if (rendezllama::maybe_parse_option_command(opt, &slice, eout)) {
          // Nothing.
        }
        else if (slice.off + 1 == slice.size && skipstr_FildeshX(&slice, "r")) {
          size_t n = buffer.rfind(':');
          if (n < buffer.size()) {
            buffer.resize(n+1);
          }
          else {
            buffer.clear();
            while (chat_tokens.size() > opt.priming_token_count) {
              const char* s = llama_token_to_str(ctx, chat_tokens.back());
              if (s[0] == ':' && s[1] == '\0') {
                break;
              }
              chat_tokens.pop_back();
              context_token_count -= 1;
            }
          }
          break;
        }
        else if (skipstr_FildeshX(&slice, "dropless")) {
          extra_penalized_tokens.clear();
        }
        else if (skipstr_FildeshX(&slice, "less")) {
          if (skipchrs_FildeshX(&slice, opt.command_delim_chars) &&
              slice.off < slice.size)
          {
            std::string line;
            line.insert(line.end(), &slice.at[slice.off], &slice.at[slice.size]);
            rendezllama::tokenize_extend(extra_penalized_tokens, ctx, line);
          }
          else {
            fildesh_log_warning("Need some content for less=.");
          }
        }
        else if (skipstr_FildeshX(&slice, "head")) {
          unsigned n = 10;
          {
            int tmp_n = 0;
            if (skipchrs_FildeshX(&slice, opt.command_delim_chars) &&
                parse_int_FildeshX(&slice, &tmp_n) &&
                tmp_n > 0)
            {
              n = tmp_n;
            }
          }
          size_t i = chat_tokens.size();
          for (size_t i = opt.priming_token_count; i < chat_tokens.size(); ++i) {
            const char* s = llama_token_to_str(ctx, chat_tokens[i]);
            eout << s;
            if (s[0] == '\n') {
              n -= 1;
              if (n == 0) {
                break;
              }
            }
          }
          eout.flush();
        }
        else if (skipstr_FildeshX(&slice, "tail")) {
          unsigned n = 10;
          {
            int tmp_n = 0;
            if (skipchrs_FildeshX(&slice, opt.command_delim_chars) &&
                parse_int_FildeshX(&slice, &tmp_n) &&
                tmp_n > 0)
            {
              n = tmp_n;
            }
          }
          size_t i = chat_tokens.size();
          while (i > 0) {
            i -= 1;
            const char* s = llama_token_to_str(ctx, chat_tokens[i]);
            if (s[0] == '\n' && s[1] == '\0') {
              n -= 1;
              if (n == 0) {
                i += 1;
                break;
              }
            }
          }
          for (; i < chat_tokens.size(); ++i) {
            eout << llama_token_to_str(ctx, chat_tokens[i]);
          }
          eout.flush();
        }
        else if (skipstr_FildeshX(&slice, "d")) {
          if (slice.off != slice.size) {
            fildesh_log_warning("Ignoring extra characters after \"d\".");
          }
          size_t n = buffer.rfind('\n');
          if (n < buffer.size()) {
            buffer.resize(n);
          }
          else {
            buffer.clear();
            while (chat_tokens.size() > opt.priming_token_count) {
              const char* s = llama_token_to_str(ctx, chat_tokens.back());
              chat_tokens.pop_back();
              context_token_count -= 1;
              if (s[0] == '\n' && s[1] == '\0') {
                break;
              }
            }
          }
          matched_antiprompt.clear();
          if (chat_tokens.size() == opt.priming_token_count && buffer.empty()) {
            matched_antiprompt = '\n';
          }
        }
        else if (skipstr_FildeshX(&slice, "yield")) {
          buffer = "\\n";
          break;
        }
        else {
          std::string line;
          line.insert(line.end(), &slice.at[slice.off], &slice.at[slice.size]);
          eout << "Unknown command: " << line << '\n';
          eout.flush();
        }
      }
      // Break out of main loop when no more input.
      if (!slice.at) {break;}

      if (buffer.length() > 0) {
        rendezllama::augment_chat_input(buffer, matched_antiprompt, opt);
        rendezllama::tokenize_extend(chat_tokens, ctx, buffer);
      }
    }
  }

  close_FildeshX(in);
  llama_free(ctx);
  return exstatus;
}
