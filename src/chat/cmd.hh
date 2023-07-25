#ifndef RENDEZLLAMA_CMD_HH_
#define RENDEZLLAMA_CMD_HH_

#include <iostream>
#include <string>
#include <vector>

#include <fildesh/fildesh.h>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;
class ChatTrajectory;

bool
maybe_do_back_command(
    ChatTrajectory& chat_traj,
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const ChatOptions& opt);
bool
maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt);
bool
maybe_do_tail_command(
    FildeshX* in,
    std::ostream& out,
    struct llama_context* ctx,
    const ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt);
bool
maybe_parse_yield_command(
    std::string& ret_buffer,
    FildeshX* in,
    const ChatOptions& opt);

}  // namespace rendezllama
#endif
