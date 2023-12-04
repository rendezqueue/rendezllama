# rendezllama

Rendezllama is a text interface for running a local chatbot based on [ggerganov's llama.cpp](https://github.com/ggerganov/llama.cpp).

For now, there's just a command-line interface, but the plan is to make a progressive web app that connects with the chatbot running on a home server.

## Chat CLI

Assuming you have the quantized weights already and can compile C++, you can try the [assistant_vicuna example](example/prompt/assistant_vicuna/) with a few commands:
```shell
# If undefined, assume the 7B model exists in a sibling llama.cpp/ dir.
MODEL="${MODEL:-../llama.cpp/models/7B/ggml-model-q4_0.gguf}"
# Make just creates a bld/ directory and invokes CMake to build there.
make
# Run with specific settings from a file. They can be given as flags too.
./bld/src/chat/chat \
  --x_setting example/prompt/assistant_vicuna/setting.sxpb \
  --thread_count 8 \
  --model "${MODEL}"
```

See the [example/prompt/](example/prompt/) directory for more interesting/whimsical examples.

### Chat CLI Options

- Setting file.
  - `--x_setting setting.sxpb` loads settings from `setting.sxpb`.
    - All other options can be set within this file.
- Model files.
  - `--model ggml-model-q4_0.gguf` are the model weights. Usually quantized.
  - See [doc/setting/model.md](doc/setting/model.md) for LoRA files and memory options.
- Prompt files.
  - `--x_priming priming.txt` specifies the priming prompt text file. This is the prompt that never changes.
  - `--x_rolling rolling.txt` specifies rolling prompt. This is the initial chat dialogue. As the chat continues, older dialogue expires and "rolls" out of context.
    - The protagonist and confidant names are derived automatically from this.
  - See [doc/setting/prompt.md](doc/setting/prompt.md) for more prompt file & format options.

### Chat CLI Commands

In the chat, most things you type will be prefixed with the protagonist's name and suffixed by the confidant's dialogue line.
There are some special inputs and commands that help keep an infinite chat from going off the rails.
Remember, the recent chat content is just a rolling prompt concatenated to the end of the priming prompt, so its quality is just as important!
- Interactivity.
  - An empty input lets token generation keep happening.
  - See [doc/setting/stdio.md](doc/setting/stdio.md) for settings that I/O behavior and limits.
  - `/tail` or `/tail 10` shows the last 10 lines.
  - `/head` or `/head 10` shows the first 10 lines of the rolling prompt.
  - `/forget 10` removes the first 10 lines of the rolling prompt.
- Characters.
  - `/(protagonist "User")` changes the protagonist's name to "User".
  - `/(confidant "Char")` changes the confidant's name to "Char".
  - See [doc/setting/prompt.md#prefix](doc/setting/prompt.md#prefix) for more ways to control chat line prefixes.
- Editing.
  - A blank space forces token generation to continue on the same line.
  - ` some text` (note blank space in front) adds `some text` to the current line.
  - ` some text ` (note blank spaces in front and back) adds `some text` and forces another token on the same line. Useful when inserting a sentence.
  - `\nsome text` (note the escaped newline in front) adds a new line of dialogue for the confidant that starts with `some text`.
  - `/puts A line of text.` adds a new line of text. Does not echo anything.
  - `/yield` or `/y` adds a new line dialogue for the confidant.
  - `/yield Char:` or `/y Char:` adds a new line starting with `Char:`.
  - `/gets 64 Char:` is like `/yield` but generates slightly over a max of 64 bytes. Only prints the newly-generated text. Always includes a newline at the end.
  - `/r` regenerates the last line of dialogue.
  - `/R` generates text from the current position. Subsequent `/r` commands will only replace the generated text, nothing before it on the line.
  - `/d` deletes up to and including the last chat prefix.
  - `/D` or `/D 0` deletes all text on the current line without consuming a newline. Positive integers delete that many earlier lines in full.
  - `/b` or `/b 1` deletes the last token.
  - `/B` or `/B 1` deletes the last word.
- Repeat penalty.
  - `/less= some unwanted words` adds extra tokens to be penalized.
  - `/dropless` clears the extra penalized tokens list.
  - See [doc/setting/penalty.md](doc/setting/penalty.md) for algorithm parameters. Defaults are okay for a chatbot.
- Temperature-based sampling.
  - See [doc/setting/sample.md](doc/setting/sample.md) for algorithm parameters. Defaults are okay for a chatbot.

