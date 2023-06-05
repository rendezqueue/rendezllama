#include <cassert>

#include <fildesh/ofstream.hh>

#include "src/chat/chat.hh"
#include "src/chat/display.hh"
#include "src/chat/cmd.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/tokenize/tokenize.hh"

static
  void
print_initialization(
    std::ostream& out,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt,
    const rendezllama::ChatTrajectory& chat_traj)
{
  if (opt.verbose_prompt && ctx && chat_traj.token_count() > 0) {
    out
      << "Number of tokens in priming prompt: " << chat_traj.priming_token_count_ << "\n"
      << "Number of tokens in full prompt: " << chat_traj.token_count() << "\n";
    for (size_t i = 0; i < chat_traj.token_count(); i++) {
      out << chat_traj.token_at(i) << " -> '"
        << llama_token_to_str(ctx, chat_traj.token_at(i)) << "'\n";
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
  llama_init_backend();
  fildesh::ofstream eout("/dev/stderr");
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

  rendezllama::ChatDisplay chat_disp;
  if (exstatus == 0) {
    chat_disp.out_ = open_FildeshOF("/dev/stdout");
    if (!opt.answer_prompt.empty()) {
      rendezllama::tokenize_extend(
          chat_disp.answer_prompt_tokens_,
          ctx, opt.answer_prompt);
    }
  }

  rendezllama::ChatTrajectory chat_traj;
  if (exstatus == 0) {
    chat_traj.mirostat_mu() = 2 * opt.mirostat_tau;
    chat_traj.transcript_out_ = open_transcript_outfile(
        exstatus, opt.transcript_sibling_filename, opt.transcript_filename);
  }

  // Tokenize the prompt.
  const std::vector<llama_token>& chat_tokens = chat_traj.tokens();
  if (exstatus == 0) {
    rendezllama::tokenize_extend(chat_traj, ctx, opt.priming_prompt);
    // No need for --keep, we just directly compute the priming prompt number of tokens.
    chat_traj.priming_token_count_ = chat_traj.token_count();
    rendezllama::tokenize_extend(chat_traj, ctx, opt.rolling_prompt);
    print_initialization(eout, ctx, opt, chat_traj);
  }

  if (exstatus == 0) {
    assert((int)opt.context_token_limit == llama_n_ctx(ctx));
    if (chat_tokens.size() > opt.context_token_limit - 4) {
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
  unsigned line_byte_limit = 0;
  unsigned line_byte_count = 0;
  unsigned sentence_count = 0;
  unsigned sentence_token_count = 0;
  bool preventing_newline = false;
  // Skip straight to user input when in coprocess mode.
  bool token_generation_on = !opt.coprocess_mode_on;

  in = open_FildeshXF("/dev/stdin");
  while (exstatus == 0) {
    if (opt.coprocess_mode_on) {
      // Print nothing except for prompted.
      chat_traj.display_token_count_ = chat_traj.token_count();
    }
    chat_disp.maybe_insert_answer_prompt(chat_traj, ctx);
    if (!rendezllama::commit_to_context(ctx, chat_disp, chat_traj, opt)) {
      exstatus = 1;
      break;
    }

    bool inputting = false;
    std::string matched_antiprompt;
    if (!token_generation_on) {
      // Just skip the first token.
      token_generation_on = true;
      inputting = true;
    }
    else {
      rendezllama::generate_next_token(
          chat_traj, ctx,
          preventing_newline, extra_penalized_tokens, opt);
      preventing_newline = false;

      chat_disp.show_new(chat_traj, ctx);

      const std::string s = llama_token_to_str(ctx, chat_traj.token());
      line_byte_count += s.size();
      // Check if each of the reverse prompts appears at the end of the output.
      // We use single-character antiprompts, so they aren't split across tokens.
      // (If we used longer antiprompts, they could be split across iterations.)
      matched_antiprompt = rendezllama::antiprompt_suffix(s, opt.antiprompts);
    }

    if (line_byte_limit > 0 && line_byte_count >= line_byte_limit) {
      inputting = true;
      if (matched_antiprompt != "\n") {
        rendezllama::tokenize_extend(chat_traj, ctx, "\n");
        chat_disp.show_new(chat_traj, ctx);
      }
    }
    else if (!matched_antiprompt.empty()) {
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

    chat_disp.maybe_remove_answer_prompt(chat_traj, inputting);

    if (inputting) {
      line_byte_count = 0;
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
            chat_traj.erase_all_at(chat_traj.token_count()-1);
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
          preventing_newline = true;
          matched_antiprompt.clear();  // For clarity.
          size_t offset = rendezllama::prev_newline_start_index(
              ctx, chat_tokens, chat_tokens.size());
          for (size_t i = offset; i < chat_tokens.size(); ++i) {
            if (rendezllama::token_endswith(ctx, chat_tokens[i], ':')) {
              offset = i+1;
              break;
            }
          }
          if (offset >= chat_traj.priming_token_count_) {
            chat_traj.erase_all_at(offset);
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
          for (unsigned i = chat_traj.priming_token_count_;
               i < chat_traj.token_count();
               ++i)
          {
            if (rendezllama::token_endswith(ctx, chat_tokens[i], '\n')) {
              n -= 1;
              if (n == 0) {
                chat_traj.rollforget(i+1, ctx);
                break;
              }
            }
          }
          if (!rendezllama::commit_to_context(ctx, chat_disp, chat_traj, opt)) {
            exstatus = 1;
            break;
          }
        }
        else if (maybe_do_head_command(&slice, eout, ctx, chat_traj, opt)) {
          // Nothing else.
        }
        else if (maybe_do_tail_command(&slice, eout, ctx, chat_traj, opt)) {
          // Nothing else.
        }
        else if (rendezllama::maybe_do_back_command(
                chat_traj, &slice, eout, ctx, opt))
        {
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          matched_antiprompt = rendezllama::antiprompt_suffix(
              llama_token_to_str(ctx, chat_tokens.back()),
              opt.antiprompts);
        }
        else if (skipstr_FildeshX(&slice, "puts ") ||
                 (slice.off + 4 == slice.size &&
                  skipstr_FildeshX(&slice, "puts")))
        {
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          buffer.clear();
          std::string line;
          line.insert(line.end(), &slice.at[slice.off], &slice.at[slice.size]);
          line += '\n';
          rendezllama::tokenize_extend(chat_traj, ctx, line);
          matched_antiprompt = '\n';
          // Might as well process now.
          chat_traj.display_token_count_ = chat_traj.token_count();
          if (!rendezllama::commit_to_context(ctx, chat_disp, chat_traj, opt)) {
            exstatus = 1;
            break;
          }
        }
        else if (skipstr_FildeshX(&slice, "gets ") ||
                 (slice.off + 4 == slice.size &&
                  skipstr_FildeshX(&slice, "gets")))
        {
          if (!buffer.empty()) {
            fildesh_log_warning("Pending input ignored by command.");
          }
          buffer.clear();
          preventing_newline = true;
          matched_antiprompt.clear();  // For clarity.
          line_byte_limit = 0;
          int tmp_n = 0;
          if (parse_int_FildeshX(&slice, &tmp_n) && tmp_n > 0) {
            line_byte_limit = (unsigned)tmp_n;
          }
          skipchrs_FildeshX(&slice, " ");
          // Prefix with user text.
          std::string line;
          line.insert(line.end(), &slice.at[slice.off], &slice.at[slice.size]);
          rendezllama::tokenize_extend(chat_traj, ctx, line);
          // Not printing any inserted text.
          chat_traj.display_token_count_ = chat_traj.token_count();
          break;
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
          if (offset <= chat_traj.priming_token_count_) {
            offset = chat_traj.priming_token_count_;
          }
          else {
            offset -= 1;
          }
          chat_traj.erase_all_at(offset);
          matched_antiprompt.clear();
          if (chat_traj.token_count() == chat_traj.priming_token_count_) {
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
        rendezllama::augment_tokenize_chat_input(
            chat_traj, ctx,
            preventing_newline,
            buffer,
            matched_antiprompt,
            opt);
      }
    }
  }

  close_FildeshX(in);
  if (exstatus == 0) {
    chat_traj.rollforget(chat_traj.token_count(), ctx);
  }
  if (ctx) {llama_free(ctx);}
  return exstatus;
}
