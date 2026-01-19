#include "src/chat/display.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

#include <cassert>
#include <vector>
#include <string>
#include <fildesh/fildesh.h>
#include "llama.h"

using rendezllama::ChatDisplay;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

static
  void
test_maybe_insert_answer_prompt(llama_model* model)
{
  const Vocabulary vocabulary(model);

  // Case: Empty answer prompt tokens.
  {
    ChatTrajectory traj(vocabulary.bos_token_id());
    ChatDisplay display;
    display.out_ = open_FildeshOF("/dev/null");

    display.maybe_insert_answer_prompt(traj, vocabulary);
    assert(display.answer_prompt_offset_ == 0);
    // Only BOS token.
    assert(traj.token_count() == 1);
  }

  // Case: Already inserted (offset != 0).
  {
    ChatTrajectory traj(vocabulary.bos_token_id());
    ChatDisplay display;
    display.out_ = open_FildeshOF("/dev/null");
    display.answer_prompt_offset_ = 5;
    display.answer_prompt_tokens_ = {1};

    unsigned old_count = traj.token_count();
    display.maybe_insert_answer_prompt(traj, vocabulary);
    assert(display.answer_prompt_offset_ == 5);
    assert(traj.token_count() == old_count);
  }

  // Case: Basic insertion.
  {
    ChatTrajectory traj(vocabulary.bos_token_id());
    ChatDisplay display;
    display.out_ = open_FildeshOF("/dev/null");

    traj.tokenize_append("User: Hello\n", vocabulary);
    unsigned count_before = traj.token_count();

    std::string prompt_str = "Assistant:";
    std::vector<Vocabulary::Token_id> prompt_tokens;
    {
      ChatTrajectory temp_traj(vocabulary.bos_token_id());
      temp_traj.tokenize_append(prompt_str, vocabulary);
      // Skip BOS.
      for (size_t i = 1; i < temp_traj.token_count(); ++i) {
        prompt_tokens.push_back(temp_traj.token_at(i));
      }
    }
    display.answer_prompt_tokens_ = prompt_tokens;

    display.maybe_insert_answer_prompt(traj, vocabulary);

    assert(display.answer_prompt_offset_ == count_before);
    assert(traj.token_count() == count_before + prompt_tokens.size());

    // Verify tokens.
    for (size_t i = 0; i < prompt_tokens.size(); ++i) {
        assert(traj.token_at(count_before + i) == prompt_tokens[i]);
    }
  }

  // Case: Insertion with suffix.
  {
    ChatTrajectory traj(vocabulary.bos_token_id());
    ChatDisplay display;
    display.out_ = open_FildeshOF("/dev/null");

    traj.tokenize_append("User: Hello\n ", vocabulary);

    std::string prompt_str = "Assistant:";
    std::vector<Vocabulary::Token_id> prompt_tokens;
    {
        ChatTrajectory temp_traj(vocabulary.bos_token_id());
        temp_traj.tokenize_append(prompt_str, vocabulary);
        for (size_t i = 1; i < temp_traj.token_count(); ++i) {
            prompt_tokens.push_back(temp_traj.token_at(i));
        }
    }
    display.answer_prompt_tokens_ = prompt_tokens;

    // Find expected insertion point.
    unsigned insertion_point = traj.token_count();
    while (insertion_point > 0) {
      if (vocabulary.last_char_of(traj.token_at(insertion_point-1)) == '\n') {
        break;
      }
      insertion_point -= 1;
    }
    assert(insertion_point > 0);

    unsigned count_before = traj.token_count();

    display.maybe_insert_answer_prompt(traj, vocabulary);

    assert(display.answer_prompt_offset_ == insertion_point);
    assert(traj.token_count() == count_before + prompt_tokens.size());

    // Verify tokens at insertion point.
    for (size_t i = 0; i < prompt_tokens.size(); ++i) {
      assert(traj.token_at(insertion_point + i) == prompt_tokens[i]);
    }
  }

  // Case: No newline found.
  {
    ChatTrajectory traj(vocabulary.bos_token_id());
    ChatDisplay display;
    display.out_ = open_FildeshOF("/dev/null");

    traj.tokenize_append("User: Hello", vocabulary);

    std::string prompt_str = "Assistant:";
    std::vector<Vocabulary::Token_id> prompt_tokens;
    {
      ChatTrajectory temp_traj(vocabulary.bos_token_id());
      temp_traj.tokenize_append(prompt_str, vocabulary);
      for (size_t i = 1; i < temp_traj.token_count(); ++i) {
        prompt_tokens.push_back(temp_traj.token_at(i));
      }
    }
    display.answer_prompt_tokens_ = prompt_tokens;

    unsigned count_before = traj.token_count();
    display.maybe_insert_answer_prompt(traj, vocabulary);

    assert(display.answer_prompt_offset_ == 0);
    assert(traj.token_count() == count_before);
  }
}

int main(int argc, char** argv) {
  assert(argc == 2 && "need model filename");
  rendezllama::GlobalScope rendezllama_global_scope;
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_model_load_from_file(argv[1], model_params);
  assert(model);

  test_maybe_insert_answer_prompt(model);

  llama_model_free(model);
  return 0;
}
