# Prompt Examples

In order of interest:
- [assistant_vicuna](assistant_vicuna/): Standard AI assistant.
  - Minimial prompt that lets a Vicuna-style model do its thing. Works with any model.
- [roshambo_kira](roshambo_kira/): Play against a Kira in roshambo.
  - Demonstrates why LLMs are hard to get right.
- [confidant_alpaca](confidant_alpaca/): A camelid that occasionally spits.
  - Demonstrates a method of prompting instruction-tuned models to fill in character dialogue.
- [assistant_alpaca](assistant_alpaca/): Instruction-following AI assistant.
  - Minimial prompt that lets an Alpaca-style model do its thing.
  - Only works with models that end the assistant's message with an EOS token.
- [assistant_chatml](assistant_chatml/): Instruction-following AI assistant.
  - Minimial prompt that lets an ChatML-style model do its thing.
  - Only works with models that have special `<|im_start|>` and `<|im_end|>` tokens.
- [assistant_coprocess](assistant_coprocess/): A simple assistant that can be controlled as a coprocess.
  - Demonstrates the `/puts` and `/gets` commands.

