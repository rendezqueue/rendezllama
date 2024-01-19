#include "cmd.hh"

#include <cassert>
#include <cstring>

#include <fildesh/string.hh>

#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"

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
print_tail_lines(std::ostream& out,
                 const Vocabulary& vocabulary,
                 const rendezllama::ChatTrajectory& chat_traj,
                 unsigned n)
{
  unsigned i = chat_traj.token_count();
  while (i > 0 && n > 0) {
    i = chat_traj.rfind_token_at(i-1, vocabulary.newline_token_id());
    n = (i < chat_traj.token_count() ? n-1 : 0);
  }
  i = (i < chat_traj.token_count() ? i+1 : 0);
  for (; i < chat_traj.token_count(); ++i) {
    vocabulary.detokenize_to(out, chat_traj.token_at(i));
  }
  out.flush();
}

  bool
rendezllama::maybe_do_back_command(
    rendezllama::ChatTrajectory& chat_traj,
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const rendezllama::ChatOptions& opt)
{
  bool space_delim_on = skip_cmd_prefix(in, "B", opt);
  if (!space_delim_on) {
    if (!skip_cmd_prefix(in, "b", opt)) {
      return false;
    }
  }
  unsigned n = 1;
  parse_unsigned_FildeshX(in, &n);
  bool skipping_contiguous_space = space_delim_on;
  fildesh::ostringstream oss;
  while (n > 0) {
    if (chat_traj.token_count() <= chat_traj.priming_token_count_) {
      break;
    }
    const Vocabulary::Token_id token_id = chat_traj.token();
    chat_traj.erase_all_at(chat_traj.token_count()-1);
    if (space_delim_on) {
      oss.truncate();
      vocabulary.detokenize_to(oss, token_id);
      const std::string_view s = oss.view();
      if (!s.empty() && (s[0] == ' ' || s[0] == '\n')) {
        if (!skipping_contiguous_space || s.size() != 1) {
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
  print_tail_lines(out, vocabulary, chat_traj, 1);
  return true;
}

  bool
rendezllama::maybe_do_delete_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "d", opt)) {
    return false;
  }
  size_t offset = chat_traj.rfind_last_message_prefix_end_at(chat_traj.token_count()-1);
  if (offset > chat_traj.priming_token_count_) {
    offset = chat_traj.rfind_message_prefix_begin_at(offset-1);
  }
  chat_traj.erase_all_at(offset);
  return true;
}

  bool
rendezllama::maybe_do_delete_inline_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary,
    const ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "D", opt)) {
    return false;
  }
  unsigned n = 0;
  parse_unsigned_FildeshX(in, &n);
  auto offset = chat_traj.token_count();
  while (offset > chat_traj.priming_token_count_) {
    offset -= 1;
    if (chat_traj.token_at(offset) == vocabulary.newline_token_id()) {
      if (n == 0) {
        offset += 1;
        break;
      }
      n -= 1;
    }
  }
  chat_traj.erase_all_at(offset);
  return true;
}

  bool
rendezllama::maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const rendezllama::ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "head", opt)) {
    return false;
  }
  unsigned n = 10;
  parse_unsigned_FildeshX(in, &n);
  for (size_t i = chat_traj.priming_token_count_; i < chat_traj.token_count(); ++i) {
    vocabulary.detokenize_to(out, chat_traj.token_at(i));
    if (vocabulary.last_char_of(chat_traj.token_at(i)) == '\n') {
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
rendezllama::maybe_do_regen_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "r", opt)) {
    return false;
  }
  size_t offset = chat_traj.rfind_last_message_prefix_end_at(chat_traj.token_count()-1);
  chat_traj.erase_all_at(offset);
  return true;
}

  bool
rendezllama::maybe_do_regen_inline_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "R", opt)) {
    return false;
  }
  auto offset = chat_traj.rfind_last_message_prefix_end_at(chat_traj.token_count());
  chat_traj.assign_range_message_prefix_id(
      chat_traj.message_prefix_id_,
      offset,
      chat_traj.token_count());
  return true;
}

  bool
rendezllama::maybe_do_tail_command(
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const rendezllama::ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "tail", opt)) {
    return false;
  }
  unsigned n = 10;
  parse_unsigned_FildeshX(in, &n);
  print_tail_lines(out, vocabulary, chat_traj, n);
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
    s += fildesh::make_string_view(*in);
  }
  else {
    s += opt.confidant + ':';
  }
  return true;
}
