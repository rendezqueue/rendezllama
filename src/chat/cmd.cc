#include "cmd.hh"

#include <cassert>
#include <cstring>

#include "src/chat/opt.hh"
#include "src/tokenize/tokenize.hh"

static
  bool
skip_cmd_prefix(FildeshX* in, const char* pfx,
                const rendezllama::ChatOptions& opt)
{
  const unsigned n = strlen(pfx);
  if (peek_bytestring_FildeshX(in, NULL, n+1)) {
    if (memchr(opt.command_delim_chars, in->at[in->off+n],
               sizeof(opt.command_delim_chars)-1))
    {
      in->off += n+1;
      return true;
    }
    return false;
  }
  return skip_bytestring_FildeshX(in, (const unsigned char*)pfx, n);
}

static
  void
print_tail_lines(std::ostream& out, struct llama_context* ctx,
                 const std::vector<llama_token> chat_tokens,
                 unsigned n)
{
  size_t i = chat_tokens.size();
  while (n > 0) {
    i = rendezllama::prev_newline_start_index(
        ctx, chat_tokens, i);
    n -= 1;
  }
  for (; i < chat_tokens.size(); ++i) {
    out << llama_token_to_str(ctx, chat_tokens[i]);
  }
  out.flush();
}

  void
rendezllama::trim_recent_chat_history(
    std::vector<llama_token>& tokens,
    unsigned& context_token_count,
    unsigned trimmed_token_count)
{
  unsigned old_token_count = tokens.size();
  assert(trimmed_token_count <= old_token_count);
  assert(trimmed_token_count > 0);
  tokens.resize(trimmed_token_count);
  if (context_token_count >= trimmed_token_count) {
    // The -1 is added to force an eval.
    context_token_count = trimmed_token_count-1;
  }
}

  bool
rendezllama::maybe_do_back_command(
    std::vector<llama_token>& chat_tokens,
    unsigned& context_token_count,
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
    if (chat_tokens.size() <= opt.priming_token_count) {
      break;
    }
    const llama_token token_id = chat_tokens.back();
    chat_tokens.pop_back();
    context_token_count -= 1;
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
  print_tail_lines(out, ctx, chat_tokens, 1);
  return true;
}

  bool
rendezllama::maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const std::vector<llama_token>& chat_tokens,
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
  for (size_t i = opt.priming_token_count; i < chat_tokens.size(); ++i) {
    out << llama_token_to_str(ctx, chat_tokens[i]);
    if (rendezllama::token_endswith(ctx, chat_tokens[i], '\n')) {
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
    const std::vector<llama_token>& chat_tokens,
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
  print_tail_lines(out, ctx, chat_tokens, n);
  return true;
}

  bool
rendezllama::maybe_parse_yield_command(
    std::string& s,
    FildeshX* in,
    const rendezllama::ChatOptions& opt)
{
  if (!skip_cmd_prefix(in, "yield", opt)) {
    return false;
  }
  s = '\n';
  if (in->off < in->size) {
    s.insert(s.end(), &in->at[in->off], &in->at[in->size]);
    if (!until_char_FildeshX(in, ':').at) {
      s += ':';
    }
  }
  else {
    s += opt.confidant + ':';
  }
  return true;
}
