#include "src/chat/opt.hh"

#include <cassert>
#include <cstring>
#include <iostream>

#include <fildesh/ostream.hh>

#include "src/chat/opt_schema.hh"

static
  void
chat_prefixes_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(((chat_prefixes)) \
      \"{{user}}:\" \
      \"{{char}} feels:\" \
      \"{{char}} wants:\" \
      \"{{char}} plans:\" \
      \"{{char}}:\" \
      )");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.given_chat_prefixes.size() == 5);
  assert(opt.chat_prefixes.size() == 5);
  opt.protagonist = "User";
  opt.protagonist_alias = "{{user}}";
  opt.confidant_alias = "{{char}}";

  in->off = 0;
  in->size = 0;
  *in = FildeshX_of_strlit("(confidant \"Char\")");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.chat_prefixes.size() == 5);
  assert(opt.chat_prefixes[0] == "User:");
  assert(opt.chat_prefixes[1] == "Char feels:");
  assert(opt.chat_prefixes[2] == "Char wants:");
  assert(opt.chat_prefixes[3] == "Char plans:");
  assert(opt.chat_prefixes[4] == "Char:");
}

static
  void
sentence_terminals_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  *in = FildeshX_of_strlit(
      "((sentence_terminals) \
      \"\\n\" \
      \"\\\"\" \
      \".\" \
      )");
  bool all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.sentence_terminals.size() == 3);
  assert(opt.sentence_terminals[0] == "\n");
  assert(opt.sentence_terminals[1] == "\"");
  assert(opt.sentence_terminals[2] == ".");
}

int main()
{
  chat_prefixes_parse_test();
  sentence_terminals_parse_test();
  return 0;
}
