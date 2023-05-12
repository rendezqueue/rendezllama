# rendezllama

Rendezllama is a text interface for running a local chatbot based on [ggerganov's llama.cpp](https://github.com/ggerganov/llama.cpp).

For now, there's just a command-line interface, but the plan is to make a progressive web app that connects with the chatbot running on a home server.

## Chat CLI

Assuming you have the quantized weights already and can compile C++, you can try the [assistant_vicuna example](example/prompt/assistant_vicuna/) with a few commands:
```shell
# If undefined, assume the 7B model exists in a sibling llama.cpp/ dir.
MODEL="${MODEL:-../llama.cpp/models/7B/ggml-model-q4_0.bin}"
# Make just creates a bld/ directory and invokes CMake to build there.
make LLAMA_OPENBLAS=0
# Run with specific settings from a file. They can be given as flags too.
./bld/src/chat/chat \
  --x_setting example/prompt/assistant_vicuna/setting.sxproto \
  --thread_count 8 \
  --model "${MODEL}"
```

See the [example/prompt/](example/prompt/) directory for more interesting/whimsical examples.

### Chat CLI Commands

In the chat, most things you type will be prefixed with the protagonist's name and suffixed by the confidant's dialogue line.
There are some special inputs and commands that help keep an infinite chat from going off the rails.
Remember, the recent chat content is just a rolling prompt concatenated to the end of the priming prompt, so its quality is just as important!
- Interactivity.
  - An empty input lets token generation keep happening.
  - Antiprompts are `.!?…` and newline. There's no way to change them right now.
  - `/sentence_limit 3` sets the number of sentences to generate before reading more input.
  - `/sentence_token_limit 40` sets the maximum number of tokens in a sentence before giving control to the user.
  - `/tail` or `/tail 10` shows the last 10 lines.
  - `/head` or `/head 10` shows the first 10 lines of the rolling prompt.
  - `/forget 10` removes the first 10 lines of the rolling prompt.
- Editing.
  - A blank space forces token generation to continue on the same line.
  - ` some text` (note blank space in front) adds `some text` to the current line.
  - ` some text ` (note blank spaces in front and back) adds `some text` and forces another token on the same line. Useful when inserting a sentence.
  - `\nsome text` (note the escaped newline in front) adds a new line of dialogue for the confidant that starts with `some text`.
  - `/yield` or `/y` adds a new line dialogue for the confidant.
  - `/yield Char:` or `/y Char:` adds a new line starting with `Char:`.
  - `/r` regenerates the current line of dialogue.
  - `/d` deletes current line.
  - `/b` or `/b 1` deletes the last token.
  - `/B` or `/B 1` deletes the last word.
- Coprocess interaction.
  - `/puts A line of text.` adds a new line of text. Does not echo anything.
  - `/gets 64 Char:` is like `/yield` but generates slightly over a max of 64 bytes. Only prints the newly-generated text. Always includes a newline at the end.
- Repeat penalty.
  - `/repeat_penalty 1.2` sets the repeated token penalty.
  - `/repeat_window 20` penalizes the most recent 20 tokens from being generated.
  - `/frequency_penalty 0.1` sets the frequency penalty. (0.0 is default, off)
  - `/presence_penalty 0.1` sets presence penalty. (0.0 is default, off)
  - `/less= some unwanted words` adds extra tokens to be penalized.
  - `/dropless` clears the extra penalized tokens list.
- Temperature-based sampling.
  - `/temp 0.7` sets the temperature.
  - `/top_k 40` sets the `top_k` parameter.
  - `/top_p 0.9` sets the `top_p` parameter.
  - `/tfs_z 0.9` sets Tail Free Sampling cutoff. (1.0 is default, off)
  - `/typical_p 0.9` sets the Locally Typical Sampling cutoff. (1.0 is default, off)
- Mirostat sampling.
  - Temperature still has meaning, but the other temperature-based sampling parameters have no effect.
  - `/mirostat 2` says to use mirostat version 2. (0 is default, off)
  - `/mirostat_tau 5.0` sets the target entropy.
  - `/mirostat_eta 0.1` sets the learning rate.
- Execution parameters.
  - `/thread_count 8` sets the number of threads.
  - `/batch_count 8` sets the batch size.

### Chat CLI Options

- Setting file.
  - `--x_setting setting.sxproto` loads settings from `setting.sxproto`.
    - All other options can be set within this file.
    - All numbers that chat commands can change can also be set within this file.
- Model files.
  - `--model ggml-model-q4_0.bin` are the model weights. Usually quantized.
    - Required.
  - `--lora ggml-adapter-model.bin` gives a LoRA.
  - `--lora_base ggml-model-f16.bin` gives higher-precision model weights to apply the LoRA on top of.
    - Not required when using `--lora` but you'll otherwise get a warning if the `--model` weights are low-precision.
- Prompt files.
  - `--x_priming priming.txt` specifies the priming prompt text file. This is the prompt that never changes.
    - Required.
  - `--x_rolling rolling.txt` specifies rolling prompt. This is the initial chat dialogue. As the chat continues, older dialogue expires and "rolls" out of context.
    - Required.
  - `--o_rolling transcript.txt` specifies a place to save the chat transcript as it rolls out of context and can no longer be edited.
  - `--x_answer answer.txt` specifies a multi-line prefix to place before every generated line of chat. Try this for models like Alpaca that are fine-tuned to follow instructions.
- Characters.
  - `--protagonist User` sets the protagonist's name. Can be changed later via the `/protagonist User` command.
  - `--confidant Bot` sets the confidant's name.  Can be changed later via the `/confidant Bot` command.
  - `--template_protagonist "{{user}}"` replaces "{{user}}" with the protagonist name.
  - `--template_confidant "{{char}}"` replaces "{{char}}" with the confidant name.
  - `--linespace_on 1` puts a space at the start of every line in the prompts (default off). This changes how the first word of a line (usually a character name) is tokenized.
- Memory.
  - `--mlock_on 1` tries to lock the model in memory (default off).
  - `--mmap_on 0` turns off mmap (default on).
    - This can remove a bottleneck when low-priority stuff (like ZFS disk cache) is preventing the mmapped model from staying in RAM.
- Misc.
  - `--coprocess_mode_on 1` limits stdout and expects to be controlled as a coprocess via commands like `/puts`, `/gets`, and `/d` (see [assistant_coprocess example](example/prompt/assistant_coprocess/)).
  - `--seed 1234` sets the random seed.
