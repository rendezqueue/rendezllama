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
      "((chat_prefixes) \
      \"{{user}}:\" \
      \"{{char}} feels:\" \
      \"{{char}} wants:\" \
      \"{{char}} plans:\" \
      \"{{char}}:\" \
      )");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.message_opts.size() == 5);
  for (const auto& message_opt : opt.message_opts) {
    assert(!message_opt.given_prefix.empty());
    assert(!message_opt.prefix.empty());
  }
  opt.protagonist = "User";
  opt.substitution.protagonist_alias = "{{user}}";
  opt.substitution.confidant_alias = "{{char}}";

  in->off = 0;
  in->size = 0;
  *in = FildeshX_of_strlit("(confidant \"Char\")");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.message_opts.size() == 5);
  assert(opt.message_opts[0].prefix == "User:");
  assert(opt.message_opts[1].prefix == "Char feels:");
  assert(opt.message_opts[2].prefix == "Char wants:");
  assert(opt.message_opts[3].prefix == "Char plans:");
  assert(opt.message_opts[4].prefix == "Char:");
}

static
  void
parse_options_test()
{
  rendezllama::ChatOptions opt;
  const char* argv[] = {
    "program_name",
    "--model", "model.gguf",
    "--protagonist", "Alice",
    "--confidant", "Bob",
    "--thread_count", "4",
    "--coprocess_mode_on", "1",
    NULL
  };
  int argc = 0;
  while (argv[argc]) {
    argc += 1;
  }

  int status = rendezllama::parse_options(opt, argc, (char**)argv);
  assert(status == 0);
  assert(opt.model_filename == "model.gguf");
  assert(opt.protagonist == "Alice");
  assert(opt.confidant == "Bob");
  assert(opt.thread_count == 4);
  assert(opt.coprocess_mode_on == true);
}

static
  void
sentence_terminals_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  *in = FildeshX_of_strlit(
      "(sentence_terminals () \
      \"\\n\" \
      \"\\\"\" \
      \".\" \
      )");
  bool all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(opt.sentence_terminals.size() == 3);
  // Insert 3 and expect that they add nothing new.
  opt.sentence_terminals.insert("\n");
  opt.sentence_terminals.insert("\"");
  opt.sentence_terminals.insert(".");
  assert(opt.sentence_terminals.size() == 3);
}

int main()
{
  chat_prefixes_parse_test();
  parse_options_test();
  sentence_terminals_parse_test();
  return 0;
}
