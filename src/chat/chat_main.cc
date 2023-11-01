#include <cassert>

#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "src/chat/chat.hh"
#include "src/chat/display.hh"
#include "src/chat/cmd.hh"
#include "src/chat/guide.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

using rendezllama::Vocabulary;

static
  void
print_initialization(
    std::ostream& out,
    const Vocabulary& vocabulary,
    const rendezllama::ChatOptions& opt,
    const rendezllama::ChatTrajectory& chat_traj)
{
  if (opt.verbose_prompt && chat_traj.token_count() > 0) {
    out
      << "Number of tokens in priming prompt: " << chat_traj.priming_token_count_ << "\n"
      << "Number of tokens in full prompt: " << chat_traj.token_count() << "\n";
    for (size_t i = 0; i < chat_traj.token_count(); i++) {
      out << chat_traj.token_at(i) << " -> '";
      vocabulary.detokenize_to(out, chat_traj.token_at(i));
      out << "'\n";
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
  rendezllama::GlobalScope rendezllama_global_scope;
  fildesh::ofstream eout("/dev/stderr");
  FildeshX* in = NULL;
  int exstatus = 0;
  rendezllama::ChatOptions opt;
  exstatus = parse_options(opt, argc, argv);

  llama_context* ctx = NULL;
  llama_model* model = NULL;
  if (exstatus == 0) {
    std::tie(model, ctx) = rendezllama::make_llama_context(opt);
    if (!ctx) {exstatus = 1;}
  }

  if (exstatus == 0 && !opt.lora_filename.empty()) {
    const char* base_model_filename = NULL;
    if (!opt.lora_base_model_filename.empty()) {
      base_model_filename = opt.lora_base_model_filename.c_str();
    }
    const float scale = 1.0f;
    int istat = llama_model_apply_lora_from_file(
        model, opt.lora_filename.c_str(), scale,
        base_model_filename,
        opt.thread_count);
    if (istat != 0) {exstatus = 1;}
  }

  Vocabulary vocabulary(model);
  rendezllama::ChatDisplay chat_disp;
  if (exstatus == 0) {
    if (!opt.bos_token_alias.empty()) {
      vocabulary.assign_substitution(
          opt.bos_token_alias, vocabulary.bos_token_id());
    }
    if (!opt.eos_token_alias.empty()) {
      vocabulary.assign_substitution(
          opt.eos_token_alias, vocabulary.eos_token_id());
    }
    for (const auto& name : opt.special_token_names) {
      Vocabulary::Token_id token_id = vocabulary.tokenize_special(name);
      if (token_id < static_cast<Vocabulary::Token_id>(vocabulary.cardinality())) {
        vocabulary.assign_substitution(name, token_id);
      }
      else {
        exstatus = 65;
        fildesh_log_errorf("Unknown special token: %s", name.c_str());
      }
    }
    chat_disp.out_ = open_FildeshOF("/dev/stdout");
    if (!opt.answer_prompt.empty()) {
      vocabulary.tokenize_to(
          chat_disp.answer_prompt_tokens_,
          opt.answer_prompt);
    }
  }

  rendezllama::ChatTrajectory chat_traj(vocabulary.bos_token_id());
  if (exstatus == 0) {
    chat_traj.mirostat_mu() = 2 * opt.mirostat_tau;
    chat_traj.transcript_out_ = open_transcript_outfile(
        exstatus, opt.transcript_sibling_filename, opt.transcript_filename);
  }

  rendezllama::ChatGuide chat_guide(vocabulary, chat_traj, opt);
  // Tokenize the prompt.
  const std::vector<llama_token>& chat_tokens = chat_traj.tokens();
  if (exstatus == 0) {
    chat_traj.tokenize_append(opt.priming_prompt, vocabulary);
    // No need for --keep, we just directly compute the priming prompt number of tokens.
    chat_traj.priming_token_count_ = chat_traj.token_count();
    chat_traj.tokenize_append(opt.rolling_prompt, vocabulary);
    chat_guide.begin_turn(1);
    print_initialization(eout, vocabulary, opt, chat_traj);
  }

  if (exstatus == 0) {
    assert((int)opt.context_token_limit == llama_n_ctx(ctx));
    // It's convenient to save a long transcript and reload it later,
    // so we allow the full prompt to exceed context limit with the expectation
    // that the earlier part of the rolling prompt won't even be evaluated.
    if (chat_traj.priming_token_count_ + 2 > opt.context_token_limit) {
      fildesh_log_error("Priming prompt is longer than context_token_limit - 2.");
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
  fildesh::ostringstream oss;

  in = open_FildeshXF("/dev/stdin");
  while (exstatus == 0) {
    if (opt.coprocess_mode_on) {
      // Print nothing except for prompted.
      chat_traj.display_token_count_ = chat_traj.token_count();
    }
    chat_disp.maybe_insert_answer_prompt(chat_traj, vocabulary);
    if (!rendezllama::commit_to_context(ctx, chat_disp, chat_traj, vocabulary, opt)) {
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
          preventing_newline, extra_penalized_tokens,
          vocabulary, opt);
      preventing_newline = false;

      chat_disp.show_new(chat_traj, vocabulary);

      oss.truncate();
      chat_disp.displaystring_to(oss.c_struct(), chat_traj.token(), vocabulary);
      const std::string_view s = oss.view();
      line_byte_count += s.size();
      // Check if each of the reverse prompts appears at the end of the output.
      // We use single-character antiprompts, so they aren't split across tokens.
      // (If we used longer antiprompts, they could be split across iterations.)
      matched_antiprompt = rendezllama::antiprompt_suffix(s, opt.antiprompts);
    }

    if (line_byte_limit > 0 && line_byte_count >= line_byte_limit) {
      inputting = true;
      chat_guide.end_turn();
      if (matched_antiprompt != "\n") {
        chat_disp.show_new(chat_traj, vocabulary);
      }
    }
    else if (chat_guide.maybe_yield_turn()) {
      if (matched_antiprompt != "\n") {
        matched_antiprompt = "\n";
      }
      if (chat_traj.message_prefix_id_ == 0) {
        inputting = true;
      }
      chat_disp.show_new(chat_traj, vocabulary);
      sentence_count = 0;
      sentence_token_count = 0;
    }
    else if (!matched_antiprompt.empty()) {
      if (sentence_count + 1 == opt.sentence_limit) {
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
            buffer += fildesh::make_string_view(slice);
            continue;
          }
          if (slice.at[0] == ' ' && buffer.empty() && matched_antiprompt == "\n") {
            // Prepare to append to the previous message.
            chat_guide.maybe_erase_trailing_message_prefix();
            chat_guide.maybe_erase_trailing_message_suffix();
            matched_antiprompt.clear();
          }
          buffer += fildesh::make_string_view(slice);
          break;
        }

        if (!buffer.empty()) {
          fildesh_log_warning("Pending input cleared. Cannot mix with commands.");
        }
        buffer.clear();

        slice.off += 1;
        if (peek_char_FildeshX(&slice, '(')) {
          rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(&slice, opt);
        }
        else if (skipstr_FildeshX(&slice, "opt")) {
          rendezllama::print_options(eout, opt);
        }
        else if (skipstr_FildeshX(&slice, "dropless")) {
          extra_penalized_tokens.clear();
        }
        else if (skipstr_FildeshX(&slice, "less")) {
          if (skipchrs_FildeshX(&slice, opt.command_delim_chars) &&
              slice.off < slice.size)
          {
            std::vector<llama_token> tmp;
            vocabulary.tokenize_to(tmp, fildesh::make_string_view(slice));
            extra_penalized_tokens.insert(
                extra_penalized_tokens.end(),
                tmp.begin(), tmp.end());
          }
          else {
            fildesh_log_warning("Need some content for less=.");
          }
        }
        else if (
            skipstr_FildeshX(&slice, "forget") ||
            skipstr_FildeshX(&slice, "rollforget"))
        {
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
            if (vocabulary.last_char_of(chat_tokens[i]) == '\n') {
              n -= 1;
              if (n == 0) {
                chat_traj.rollforget(i+1, vocabulary);
                break;
              }
            }
          }
          if (!rendezllama::commit_to_context(
                  ctx, chat_disp, chat_traj,
                  vocabulary, opt))
          {
            exstatus = 1;
            break;
          }
        }
        else if (maybe_do_head_command(&slice, eout, vocabulary, chat_traj, opt)) {
          // Nothing else.
        }
        else if (maybe_do_tail_command(&slice, eout, vocabulary, chat_traj, opt)) {
          // Nothing else.
        }
        else if (rendezllama::maybe_do_back_command(
                chat_traj, &slice, eout, vocabulary, opt))
        {
          oss.truncate();
          vocabulary.detokenize_to(oss, chat_tokens.back());
          matched_antiprompt = rendezllama::antiprompt_suffix(
              oss.view(),
              opt.antiprompts);
        }
        else if (skipstr_FildeshX(&slice, "puts ") ||
                 (slice.off + 4 == slice.size &&
                  skipstr_FildeshX(&slice, "puts")))
        {
          chat_traj.tokenize_append(
              fildesh::make_string(slice) + '\n',
              vocabulary);
          matched_antiprompt = '\n';
          // Might as well process now.
          chat_traj.display_token_count_ = chat_traj.token_count();
          if (!rendezllama::commit_to_context(
                  ctx, chat_disp, chat_traj, vocabulary, opt))
          {
            exstatus = 1;
            break;
          }
        }
        else if (skipstr_FildeshX(&slice, "gets ") ||
                 (slice.off + 4 == slice.size &&
                  skipstr_FildeshX(&slice, "gets")))
        {
          preventing_newline = true;
          matched_antiprompt.clear();  // For clarity.
          line_byte_limit = 0;
          int tmp_n = 0;
          if (parse_int_FildeshX(&slice, &tmp_n) && tmp_n > 0) {
            line_byte_limit = (unsigned)tmp_n;
          }
          skipchrs_FildeshX(&slice, " ");
          // Prefix with user text.
          chat_traj.tokenize_append(
              fildesh::make_string_view(slice),
              vocabulary);
          // Set this index so token generation stops after 1 line.
          chat_traj.message_prefix_id_ = opt.message_opts.size();
          // Not printing any inserted text.
          chat_traj.display_token_count_ = chat_traj.token_count();
          break;
        }
        else if (rendezllama::maybe_do_delete_command(&slice, chat_traj, opt)) {
          matched_antiprompt = '\n';
        }
        else if (rendezllama::maybe_do_delete_inline_command(
                &slice, chat_traj, vocabulary, opt)) {
          matched_antiprompt = '\n';
        }
        else if (rendezllama::maybe_do_regen_command(&slice, chat_traj, opt)) {
          preventing_newline = true;
          matched_antiprompt.clear();  // For clarity.
          break;
        }
        else if (rendezllama::maybe_do_regen_inline_command(
                &slice, chat_traj, opt)) {
          preventing_newline = true;
          matched_antiprompt.clear();  // For clarity.
          break;
        }
        else if (rendezllama::maybe_parse_yield_command(buffer, &slice, opt)) {
          break;
        }
        else {
          eout << "Unknown command: "
            << fildesh::make_string_view(slice) << '\n';
          eout.flush();
        }
      }
      // Break out of main loop when no more input.
      if (exstatus != 0 || !slice.at) {break;}

      if (buffer.length() > 0) {
        rendezllama::augment_tokenize_chat_input(
            chat_guide,
            chat_traj,
            preventing_newline,
            buffer,
            vocabulary,
            opt);
      }
    }
  }

  close_FildeshX(in);
  if (exstatus == 0) {
    chat_traj.rollforget(chat_traj.token_count(), vocabulary);
  }
  if (ctx) {llama_free(ctx);}
  if (model) {llama_free_model(model);}
  return exstatus;
}
