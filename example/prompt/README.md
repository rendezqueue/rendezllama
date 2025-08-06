# Prompt Examples

In order of interest:
- [assistant_plain](assistant_plain/): AI assistant with single-line replies.
  - Minimial prompt that should work for any model.
- [roshambo_kira](roshambo_kira/): Play against a Kira in roshambo.
  - Demonstrates why LLMs are hard to get right.
- [confidant_alpaca](confidant_alpaca/): A camelid that occasionally spits.
  - Demonstrates a method of prompting instruction-tuned models to fill in character dialogue.
- Instruction-following AI assistants.
  - For all of these examples, the assistant must end its messages with a special token like EOS.
  - [assistant_alpaca](assistant_alpaca/): Alpaca prompt format.
  - [assistant_chatml](assistant_chatml/): ChatML prompt format that typically requires special `<|im_start|>` and `<|im_end|>` tokens but is configured with fallbacks.
  - [assistant_gemma](assistant_gemma/): Gemma prompt format that requires special `<start_of_turn>` and `<end_of_turn>` tokens.
  - [assistant_harmony](assistant_harmony/): Harmony prompt format that requires special `<|start|>`, `<|end|>`, `<|channel|>`, and `<|return|>` tokens.
  - [assistant_llama](assistant_llama/): Llama 3 prompt format that requires special `<|start_header_id|>`, `<|end_header_id|>`, and `<|eot_id|>` tokens.
  - [assistant_mistral](assistant_mistral/): Mistral propmt format that requires special `[INST]` and `[/INST]` tokens.
  - [assistant_vicuna](assistant_vicuna/): Vicuna prompt format.
- [assistant_coprocess](assistant_coprocess/): A simple assistant that can be controlled as a coprocess.
  - Demonstrates the `/puts` and `/gets` commands.

