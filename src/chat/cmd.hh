#ifndef RENDEZLLAMA_CMD_HH_
#define RENDEZLLAMA_CMD_HH_

#include <iostream>
#include <string>
#include <vector>

#include <fildesh/fildesh.h>

namespace rendezllama {

struct ChatOptions;
class ChatTrajectory;
class Vocabulary;

bool
maybe_do_back_command(
    ChatTrajectory& chat_traj,
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const ChatOptions& opt);
bool
maybe_do_delete_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt);
bool
maybe_do_delete_inline_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary,
    const ChatOptions& opt);
bool
maybe_do_head_command(
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt);
bool
maybe_do_regen_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt);
bool
maybe_do_regen_inline_command(
    FildeshX* in,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt);
bool
maybe_do_tail_command(
    FildeshX* in,
    std::ostream& out,
    const Vocabulary& vocabulary,
    const ChatTrajectory& chat_traj,
    const rendezllama::ChatOptions& opt);
bool
maybe_parse_yield_command(
    std::string& ret_buffer,
    FildeshX* in,
    const ChatOptions& opt);

}  // namespace rendezllama
#endif
