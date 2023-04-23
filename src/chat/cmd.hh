#ifndef RENDEZLLAMA_CMD_HH_
#define RENDEZLLAMA_CMD_HH_

#include <string>

#include <fildesh/fildesh.h>

namespace rendezllama {

struct ChatOptions;

bool
maybe_parse_yield_command(
    std::string& ret_buffer,
    FildeshX* in,
    const ChatOptions& opt);

}  // namespace rendezllama
#endif
