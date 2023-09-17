#include "cmd.hh"

#include <cassert>
#include <cstring>

#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/tokenize/tokenize.hh"

using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;


static
  bool
skip_cmd_prefix(FildeshX* in, const char* pfx,
                const rendezllama::ChatOptions& opt)
{
  const unsigned n = strlen(pfx);
  if (!peek_bytestring_FildeshX(in, (const unsigned char*)pfx, n)) {
    return false;
  }

  if (!peek_bytestring_FildeshX(in, NULL, n+1)) {
    // No more to read.
    in->off += n;
    return true;
  }

  if (memchr(opt.command_delim_chars, in->at[in->off+n],
             sizeof(opt.command_delim_chars)-1))
  {
    // Caught a delimiter.
    in->off += n+1;
    return true;
  }
  return false;
}

static
  void
print_tail_lines(std::ostream& out, struct llama_context* ctx,
                 const rendezllama::ChatTrajectory& chat_traj,
                 unsigned n)
{
  unsigned i = chat_traj.token_count();
  while (n > 0) {
    i = rendezllama::prev_newline_start_index(
        ctx, chat_traj.tokens(), i);
    n -= 1;
  }
  for (; i < chat_traj.token_count(); ++i) {
    out << llama_token_to_str(ctx, chat_traj.token_at(i));
  }
  out.flush();
}

  bool
rendezllama::maybe_do_back_command(
    rendezllama::ChatTrajectory& chat_traj,
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt)
{
  bool space_delim_on = skip_cmd_prefix(in, "B", opt);
  if (!space_delim_on) {
    if (!skip_cmd_prefix(in, "b", opt)) {
      return false;
    }
  }
  unsigned n = 1;
  {
    int tmp_n = -1;
    if (parse_int_FildeshX(in, &tmp_n) && tmp_n > 0) {
      n = tmp_n;
    }
  }
  bool skipping_contiguous_space = space_delim_on;
  while (n > 0) {
    if (chat_traj.token_count() <= chat_traj.priming_token_count_) {
      break;
    }
    const llama_token token_id = chat_traj.token();
    chat_traj.erase_all_at(chat_traj.token_count()-1);
    if (space_delim_on) {
      const char* s = llama_token_to_str(ctx, token_id);
      if (s && (s[0] == ' ' || s[0] == '\n')) {
        if (!skipping_contiguous_space || s[1] != '\0') {
          n -= 1;
        }
        skipping_contiguous_space = true;
      }
      else {
        skipping_contiguous_space = false;
      }
    }
    else {
      n -= 1;
    }
  }
  print_tail_lines(out, ctx, chat_traj, 1);
  return true;
}

  bool
rendezllama::maybe_do_rollforget_command(
    ChatTrajectory& chat_traj,
    FildeshX* in,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt)
{
  const Vocabulary vocabulary(ctx);
  if (!skip_cmd_prefix(in, "rollforget", opt) &&
      !skip_cmd_prefix(in, "forget", opt)) {
    return false;
  }

  unsigned n = 10;
  {
    int tmp_n = 0;
    if (parse_int_FildeshX(in, &tmp_n)) {
      if (tmp_n >= 0) {
        n = tmp_n;
      }
      else {
        n = chat_traj.priming_token_count_;
      }
    }
  }

  unsigned i = chat_traj.priming_token_count_;
  for (; n > 0 && i < chat_traj.token_count(); ++i) {
    if (rendezllama::token_endswith(ctx, chat_traj.token_at(i), '\n')) {
      n -= 1;
    }
  }
  chat_traj.rollforget(i, vocabulary);
  return true;
}

  bool
rendezllama::maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const rendezllama::ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "head", opt)) {
    return false;
  }
  unsigned n = 10;
  {
    int tmp_n = 0;
    if (parse_int_FildeshX(in, &tmp_n) && tmp_n > 0) {
      n = tmp_n;
    }
  }
  for (size_t i = chat_traj.priming_token_count_; i < chat_traj.token_count(); ++i) {
    out << llama_token_to_str(ctx, chat_traj.token_at(i));
    if (rendezllama::token_endswith(ctx, chat_traj.token_at(i), '\n')) {
      n -= 1;
      if (n == 0) {
        break;
      }
    }
  }
  out.flush();
  return true;
}

  bool
rendezllama::maybe_do_tail_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const rendezllama::ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "tail", opt)) {
    return false;
  }
  unsigned n = 10;
  {
    int tmp_n = 0;
    if (parse_int_FildeshX(in, &tmp_n) && tmp_n > 0) {
      n = tmp_n;
    }
  }
  print_tail_lines(out, ctx, chat_traj, n);
  return true;
}

  bool
rendezllama::maybe_parse_yield_command(
    std::string& s,
    FildeshX* in,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "yield", opt) &&
      !skip_cmd_prefix(in, "y", opt)) {
    return false;
  }
  s = '\n';
  if (in->off < in->size) {
    s.insert(s.end(), &in->at[in->off], &in->at[in->size]);
  }
  else {
    s += opt.confidant + ':';
  }
  return true;
}
