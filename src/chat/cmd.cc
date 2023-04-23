#include "cmd.hh"

#include "src/chat/opt.hh"


  bool
rendezllama::maybe_parse_yield_command(
    std::string& s,
    FildeshX* in,
    const rendezllama::ChatOptions& opt)
{
  if (!skipstr_FildeshX(in, "yield")) {return false;}
  s = '\n';
  if (skipchrs_FildeshX(in, opt.command_delim_chars)) {
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
