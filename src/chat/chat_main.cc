#include <cassert>

#include <fildesh/ofstream.hh>

#include "src/chat/chat.hh"
#include "src/chat/cmd.hh"
#include "src/chat/opt.hh"
#include "src/tokenize/tokenize.hh"


static
  FildeshO*
open_transcript_outfile(
    int& exstatus,
    const std::string& sibling_filename,
    const std::string& transcript_filename)
{
  FildeshO* transcript_out = NULL;
  if (exstatus == 0) {
    if (!transcript_filename.empty()) {
      transcript_out = open_sibling_FildeshOF(
              sibling_filename.c_str(), transcript_filename.c_str());
      if (!transcript_out) {
        fildesh_log_error("cannot open --o_rolling file for writing");
        exstatus = 1;
      }
    }
  }
  return transcript_out;
}


int main(int argc, char** argv)
{
  fildesh::ofstream eout("/dev/stderr");
  fildesh::ofstream out("/dev/stdout");
  FildeshX* in = NULL;
  int exstatus = 0;
  rendezllama::ChatOptions opt;
  exstatus = parse_options(opt, argc, argv);

  llama_context* ctx = NULL;
  if (exstatus == 0) {
    ctx = rendezllama::make_llama_context(opt);
    if (!ctx) {exstatus = 1;}
  }

  if (exstatus == 0 && !opt.lora_filename.empty()) {
    const char* base_model_filename = NULL;
    if (!opt.lora_base_model_filename.empty()) {
      base_model_filename = opt.lora_base_model_filename.c_str();
    }
    int istat = llama_apply_lora_from_file(
        ctx, opt.lora_filename.c_str(), base_model_filename, opt.thread_count);
    if (istat != 0) {exstatus = 1;}
  }

  fildesh::ofstream transcript_out(open_transcript_outfile(
          exstatus, opt.transcript_sibling_filename, opt.transcript_filename));

  // tokenize the prompt
  std::vector<llama_token> chat_tokens;
  if (exstatus == 0) {
    chat_tokens.push_back(llama_token_bos());
    rendezllama::tokenize_extend(chat_tokens, ctx, opt.priming_prompt);
    // No need for --keep, we just directly compute the priming prompt number of tokens.
    opt.priming_token_count = chat_tokens.size();
    if (opt.priming_token_count < 0) {
      fildesh_log_error("bad priming tokenization");
      exstatus = 1;
    }
    else {
      rendezllama::tokenize_extend(chat_tokens, ctx, opt.rolling_prompt);
      rendezllama::print_initialization(eout, ctx, opt, chat_tokens);
    }
  }

  if (exstatus == 0) {
    assert(opt.context_token_limit == llama_n_ctx(ctx));
    if ((int) chat_tokens.size() > opt.context_token_limit - 4) {
      fildesh_log_error("Prompt is longer than context_token_limit - 4.");
      exstatus = 1;
    }
  }

  if (exstatus == 0) {
    eout
      << "=== Chat CLI ===\n"
      << "- Token generation will frequently wait for input.\n"
      << "  Press enter to let it continue.\n"
      << "- See README.md for other commands.\n\n"
      ;
    eout.flush();
  }

  std::vector<llama_token> extra_penalized_tokens;
  unsigned sentence_count = 0;
  unsigned sentence_token_count = 0;
  unsigned context_token_count = 0;
  bool preventing_newline = false;

  in = open_FildeshXF("/dev/stdin");
  while (exstatus == 0) {
    context_token_count = rendezllama::commit_to_context(
        ctx, out, transcript_out,
        chat_tokens, context_token_count, opt);
    if (context_token_count == 0) {exstatus = 1; break;}
    assert(context_token_count == (int)chat_tokens.size());

    std::string matched_antiprompt;
    {
      llama_token id = rendezllama::generate_next_token(
          ctx, preventing_newline, extra_penalized_tokens, chat_tokens, opt);
      preventing_newline = false;

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
    }

    bool inputting = false;
    if (!matched_antiprompt.empty()) {
      if (matched_antiprompt == "\n") {
        inputting = true;
      }
      else if (sentence_count + 1 == opt.sentence_limit) {
        // Reached the limit on number of sentences.
        inputting = true;
      }
      else {
        sentence_count += 1;
        sentence_token_count = 0;
      }
    }
    else {
      if (sentence_token_count + 1 == opt.sentence_token_limit) {
        // Reached the limit on number of tokens in a sentence.
        inputting = true;
      }
      else {
        sentence_token_count += 1;
      }
    }

    if (inputting) {
      sentence_token_count = 0;
      sentence_count = 0;

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
          if (slice.at[0] == ' ' && buffer.empty() && matched_antiprompt == "\n") {
            // Remove preceeding newline
            rendezllama::trim_recent_chat_history(
                chat_tokens, context_token_count,
                chat_tokens.size()-1);
            matched_antiprompt.clear();
          }
          buffer.insert(buffer.end(), slice.at, &slice.at[slice.size]);
          break;
        }

        slice.off += 1;
        if (rendezllama::maybe_parse_option_command(opt, &slice, eout)) {
          // Nothing.
        }
        else if (slice.off + 1 == slice.size && skipstr_FildeshX(&slice, "r")) {
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          buffer.clear();
          matched_antiprompt.clear();  // For clarity.
          size_t offset = rendezllama::prev_newline_start_index(
              ctx, chat_tokens, chat_tokens.size());
          for (size_t i = offset; i < chat_tokens.size(); ++i) {
            if (rendezllama::token_endswith(ctx, chat_tokens[i], ':')) {
              offset = i+1;
              break;
            }
          }
          if (offset >= opt.priming_token_count) {
            rendezllama::trim_recent_chat_history(
                chat_tokens, context_token_count, offset);
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
        else if (skipstr_FildeshX(&slice, "forget")) {
          unsigned n = 10;
          {
            int tmp_n = 0;
            if (skipchrs_FildeshX(&slice, opt.command_delim_chars) &&
                parse_int_FildeshX(&slice, &tmp_n) &&
                tmp_n > 0)
            {
              n = tmp_n;
            }
            else {
              eout << "Ignoring /forget command without line count.\n"; eout.flush();
              continue;
            }
          }
          bool copying = false;
          size_t dst_index = opt.priming_token_count;
          for (size_t i = opt.priming_token_count; i < chat_tokens.size(); ++i) {
            if (copying) {
              chat_tokens[dst_index] = chat_tokens[i];
              dst_index += 1;
            }
            else if (rendezllama::token_endswith(ctx, chat_tokens[i], '\n')) {
              n -= 1;
              copying = (n == 0);
            }
          }
          fildesh::ofstream nullout("/dev/null");
          chat_tokens.resize(dst_index);
          context_token_count = rendezllama::commit_to_context(
              ctx, nullout, transcript_out,
              chat_tokens, opt.priming_token_count, opt);
          if (context_token_count == 0) {exstatus = 1; break;}
          assert(context_token_count == (int)chat_tokens.size());
        }
        else if (maybe_do_head_command(&slice, eout, ctx, chat_tokens, opt)) {
          // Nothing else.
        }
        else if (maybe_do_tail_command(&slice, eout, ctx, chat_tokens, opt)) {
          // Nothing else.
        }
        else if (rendezllama::maybe_do_back_command(
                chat_tokens, context_token_count,
                &slice, eout, ctx, opt))
        {
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          matched_antiprompt = rendezllama::antiprompt_suffix(
              llama_token_to_str(ctx, chat_tokens.back()),
              opt.antiprompts);
        }
        else if (skipstr_FildeshX(&slice, "d")) {
          if (slice.off != slice.size) {
            fildesh_log_warning("Ignoring extra characters after \"d\".");
          }
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          buffer.clear();
          size_t offset = rendezllama::prev_newline_start_index(
              ctx, chat_tokens, chat_tokens.size());
          if (offset <= opt.priming_token_count) {
            offset = opt.priming_token_count;
          }
          else {
            offset -= 1;
          }
          rendezllama::trim_recent_chat_history(
              chat_tokens, context_token_count, offset);
          matched_antiprompt.clear();
          if (chat_tokens.size() == opt.priming_token_count && buffer.empty()) {
            matched_antiprompt = '\n';
          }
        }
        else if (rendezllama::maybe_parse_yield_command(buffer, &slice, opt)) {
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
      if (exstatus != 0 || !slice.at) {break;}

      if (buffer.length() > 0) {
        rendezllama::augment_chat_input(
            buffer, preventing_newline, matched_antiprompt, opt);
        rendezllama::tokenize_extend(chat_tokens, ctx, buffer);
      }
    }
  }

  close_FildeshX(in);
  if (exstatus == 0) {
    rendezllama::print_tokens(
        transcript_out,
        chat_tokens.begin() + opt.priming_token_count,
        chat_tokens.end(),
        ctx);
  }
  if (ctx) {llama_free(ctx);}
  return exstatus;
}
