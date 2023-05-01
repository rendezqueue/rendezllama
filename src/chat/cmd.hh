#ifndef RENDEZLLAMA_CMD_HH_
#define RENDEZLLAMA_CMD_HH_

#include <iostream>
#include <string>
#include <vector>

#include <fildesh/fildesh.h>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;

void
trim_recent_chat_history(
    std::vector<llama_token>& tokens,
    unsigned& context_token_count,
    unsigned trimmed_token_count);
bool
maybe_do_back_command(
    std::vector<llama_token>& chat_tokens,
    unsigned& context_token_count,
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const ChatOptions& opt);
bool
maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const std::vector<llama_token>& chat_tokens,
    const rendezllama::ChatOptions& opt);
bool
maybe_do_tail_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const std::vector<llama_token>& chat_tokens,
    const rendezllama::ChatOptions& opt);
bool
maybe_parse_yield_command(
    std::string& ret_buffer,
    FildeshX* in,
    const ChatOptions& opt);

}  // namespace rendezllama
#endif
