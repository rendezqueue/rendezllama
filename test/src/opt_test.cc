#include <cassert>
#include <cstring>
#include <iostream>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"

static
  void
chat_prefixes_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX* in = open_FildeshXA();
  {
    const char content[] =
      "((chat_prefixes) \"{{user}}:\" \"{{char}} feels:\" \"{{char}} wants:\" \"{{char}} plans:\" \"{{char}}:\")\n";
    const size_t content_size = strlen(content);
    memcpy(grow_FildeshX(in, content_size), content, content_size);
  }
  bool all_good = rendezllama::parse_options_sxproto_content(opt, in, "");
  assert(all_good);
  assert(opt.given_chat_prefixes.size() == 5);
  assert(opt.chat_prefixes.size() == 0);
  opt.protagonist = "User";
  opt.template_protagonist = "{{user}}";
  opt.template_confidant = "{{char}}";

  in->off = 0;
  in->size = 0;
  {
    const char content[] = "confidant Char";
    const size_t content_size = strlen(content);
    memcpy(grow_FildeshX(in, content_size), content, content_size);
  }
  all_good = rendezllama::maybe_parse_option_command(opt, in, std::cerr);
  assert(all_good);
  assert(opt.chat_prefixes.size() == 5);
  assert(opt.chat_prefixes[0] == "User:");
  assert(opt.chat_prefixes[1] == "Char feels:");
  assert(opt.chat_prefixes[2] == "Char wants:");
  assert(opt.chat_prefixes[3] == "Char plans:");
  assert(opt.chat_prefixes[4] == "Char:");

  close_FildeshX(in);
}

int main()
{
  chat_prefixes_parse_test();
  return 0;
}
