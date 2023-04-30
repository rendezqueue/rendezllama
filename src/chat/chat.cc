#include "chat.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"
#include "src/tokenize/tokenize.hh"

  const std::string&
rendezllama::antiprompt_suffix(
    const std::string& text,
    const std::vector<std::string>& antiprompts)
{
  static const std::string empty_string;
  for (const std::string& s : antiprompts) {
    if (text.size() >= s.size()) {
      const size_t offset = text.size() - s.size();
      if (0 == memcmp(&text[offset], &s[0], s.size())) {
        return s;
      }
    }
  }
  return empty_string;
}

static std::string protagonist_line_prefix(const rendezllama::ChatOptions& opt) {
  std::string s;
  if (opt.linespace_on) {s += ' ';}
  s += opt.protagonist + ": ";
  return s;
}
static std::string confidant_line_prefix(const rendezllama::ChatOptions& opt) {
  std::string s;
  if (opt.linespace_on) {s += ' ';}
  s += opt.confidant + ':';
  return s;
}

static bool maybe_trim_endspace(std::string& s)
{
  bool result = false;
  while (!s.empty() && s.back() == ' ') {
    s.pop_back();
    result = true;
  }
  return result;
}

  void
rendezllama::augment_chat_input(
    std::string& s,
    bool& prevent_subsequent_newline,
    const std::string& matched_antiprompt,
    const rendezllama::ChatOptions& opt)
{
  prevent_subsequent_newline = false;
  if (s.size() >= 2 && s[0] == '\\' && s[1] == 'n') {
    s.erase(0, 2);
    std::string maybe_newline, maybe_space;
    if (matched_antiprompt != "\n") {
      maybe_newline = '\n';
    }
    if (s.empty() || s[0] != ' ') {
      maybe_space = ' ';
    }
    s = maybe_newline + confidant_line_prefix(opt) + maybe_space + s;
    prevent_subsequent_newline = maybe_trim_endspace(s);
  }
  else if (s.front() == '\n') {
    s.erase(0, 1);
    std::string pfx;
    // This is from /yield.
    if (matched_antiprompt != "\n") {
      pfx += '\n';
    }
    if (opt.linespace_on) {pfx += ' ';}
    s = pfx + s;
  }
  else if (s.front() == ' ') {
    prevent_subsequent_newline = maybe_trim_endspace(s);
  }
  else if (s.front() == '\n') {
    // This is from /yield.
    s.erase(0, 1);
    std::string pfx;
    if (matched_antiprompt != "\n") {
      pfx += '\n';
    }
    if (opt.linespace_on) {pfx += ' ';}
    s = pfx + s;
  }
  else if (s.back() == '[' || s.back() == ':') {
    // Nothing.
  }
  else {
    std::string maybe_newline;
    if (matched_antiprompt != "\n") {
      maybe_newline = '\n';
    }
    s = (maybe_newline + protagonist_line_prefix(opt) +
         s + '\n' + confidant_line_prefix(opt));
  }
}

  llama_token
rendezllama::generate_next_token(
    struct llama_context* ctx,
    bool preventing_newline,
    const std::vector<llama_token>& extra_penalized_tokens,
    const std::vector<llama_token>& tokens,
    const rendezllama::ChatOptions& opt)
{
  // Zero probability for end-of-stream token.
  // (Technically called "end-of-sentence", but it's treated as end-of-stream.)
  llama_get_logits(ctx)[llama_token_eos()] = 0;

  if (preventing_newline) {
    llama_get_logits(ctx)[rendezllama::newline_token(ctx)] = 0;
  }

  const size_t trailing_token_count = std::min(
      tokens.size(), (size_t)opt.repeat_last_count);

  std::vector<llama_token> penalized_tokens;
  penalized_tokens.resize(trailing_token_count);
  for (unsigned i = 0; i < trailing_token_count; ++i) {
    penalized_tokens[i] = tokens[tokens.size() - trailing_token_count + i];
  }
  penalized_tokens.insert(
      penalized_tokens.end(),
      extra_penalized_tokens.begin(), extra_penalized_tokens.end());

  llama_token token_id = llama_sample_top_p_top_k(
      ctx,
      &penalized_tokens[0], penalized_tokens.size(),
      opt.top_k, opt.top_p, opt.temp, opt.repeat_penalty);

  // If the improbable happens, just use a newline token.
  if (token_id == llama_token_eos()) {
    int n = llama_tokenize(ctx, "\n", &token_id, 1, /*add_bos=*/false);
    assert(n == 1);
  }
  return token_id;
}

  unsigned
rendezllama::commit_to_context(
    struct llama_context* ctx,
    std::ostream& out,
    std::ostream& transcript_out,
    std::vector<llama_token>& chat_tokens,
    unsigned context_token_count,
    const ChatOptions& opt)
{
  assert(context_token_count <= chat_tokens.size());
  if (context_token_count == chat_tokens.size()) {
    return context_token_count;
  }

  unsigned display_token_count = chat_tokens.size() - context_token_count;
  if (context_token_count > 0) {
    // Includes a generated token, which was printed already.
    display_token_count -= 1;
  }

  if (chat_tokens.size() > opt.context_token_limit) {
    // Drop some of the rolling prompt while keeping the priming prompt
    // to avoid exceeding our context token limit.
    const unsigned rolling_token_count =
      (opt.context_token_limit - opt.priming_token_count) / 2;
    bool copying = false;
    unsigned dst_index = opt.priming_token_count;
    for (unsigned src_index = chat_tokens.size() - rolling_token_count;
         src_index < chat_tokens.size();
         ++src_index)
    {
      if (copying) {
        chat_tokens[dst_index] = chat_tokens[src_index];
        dst_index += 1;
      }
      else {
        copying = rendezllama::token_endswith(
            ctx, chat_tokens[src_index], '\n');
        if (copying) {
          rendezllama::print_tokens(
              transcript_out,
              chat_tokens.begin() + dst_index,
              chat_tokens.begin() + (src_index + 1),
              ctx);
        }
      }
    }
    chat_tokens.resize(dst_index);
    context_token_count = opt.priming_token_count;
  }

  while (context_token_count < chat_tokens.size()) {
    const unsigned n = std::min(
        (size_t)opt.batch_count,
        chat_tokens.size() - context_token_count);

    // Display.
    for (unsigned i = 0; i < n; ++i) {
      if (context_token_count + i < chat_tokens.size() - display_token_count) {
        continue;
      }
      const llama_token token_id = chat_tokens[context_token_count + i];
      const std::string s = llama_token_to_str(ctx, token_id);
      out << s;
    }
    out.flush();

    const int istat = llama_eval(
        ctx,
        &chat_tokens[context_token_count],
        n,
        context_token_count,
        opt.thread_count);
    if (istat != 0) {
      fildesh_log_error("Failed to eval.");
      return 0;
    }
    context_token_count += n;
  }
  return chat_tokens.size();
}

  unsigned
rendezllama::maybe_insert_answer_prompt(
    std::vector<llama_token>& chat_tokens,
    struct llama_context* ctx,
    unsigned answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens)
{
  if (answer_prompt_tokens.size() == 0) {return 0;}
  if (answer_prompt_offset != 0) {return answer_prompt_offset;}
  answer_prompt_offset = chat_tokens.size();
  while (answer_prompt_offset > 0) {
    if (rendezllama::token_endswith(ctx, chat_tokens[answer_prompt_offset-1], '\n')) {
      break;
    }
    answer_prompt_offset -= 1;
  }
  if (answer_prompt_offset > 0) {
    chat_tokens.insert(
        chat_tokens.begin()+answer_prompt_offset, 
        answer_prompt_tokens.begin(),
        answer_prompt_tokens.end());
  }
  return answer_prompt_offset;
}

  void
rendezllama::maybe_remove_answer_prompt(
    std::vector<llama_token>& chat_tokens,
    unsigned& context_token_count,
    unsigned& answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens,
    bool inputting)
{
  if (!inputting) {return;}
  if (answer_prompt_tokens.size() == 0) {return;}
  if (answer_prompt_offset == 0) {return;}
  chat_tokens.erase(
      chat_tokens.begin()+answer_prompt_offset,
      chat_tokens.begin()+answer_prompt_offset+answer_prompt_tokens.size());
  context_token_count = answer_prompt_offset;
  answer_prompt_offset = 0;
}
