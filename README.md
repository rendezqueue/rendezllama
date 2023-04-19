# rendezllama

Rendezllama is a text interface for running a local chatbot based on [ggerganov's llama.cpp](https://github.com/ggerganov/llama.cpp).

For now, there's just a command-line interface, but the plan is to make a progressive web app that connects with the chatbot running on a home server.

## Chat CLI

Assuming you have quantized weights in `../llama.cpp/models/7B/ggml-model-q4_0.bin`, you can start the chat CLI with:
```shell
make
./bld/src/chat/chat \
    --x_priming test/prompt/roshambo_0/priming.txt \
    --x_rolling test/prompt/roshambo_0/rolling.txt \
    --thread_count 8 \
    --model ../llama.cpp/models/7B/ggml-model-q4_0.bin
```

The confidant (bot) and protagonist (you) names are determined from last two lines of the rolling prompt (in that order).

In the chat, most things you type will be prefixed with the protagonist's name and suffixed by the confidant's dialogue line.
There are some special inputs and commands that help keep an infinite chat from going off the rails.
Remember, the recent chat content is just a rolling prompt concatenated to the end of the priming prompt, so its quality is just as important!
- Interactivity.
  - An empty input lets token generation keep happening.
  - Antiprompts are `.!?…` and newline. There's no way to change them right now.
  - `/sequent_limit 32` sets the number of tokens to generate before reading more input.
  - `/tail` or `/tail 10` shows the last 10 context lines.
- Editing.
  - `/yield` or `\n` inserts a newline and has the confidant say something. Useful for prompts that encourage the confidant to use inner monologue to think about the world.
  - `/r` regenerates the current line of dialogue.
  - `/d` deletes current line.
- Repeat penalty.
  - `/repeat_penalty 1.2` sets the repeated token penalty.
  - `/repeat_window 20` penalizes the most recent 20 tokens from being generated.
  - `/less= some unwanted words` adds extra tokens to be penalized.
  - `/dropless` clears the extra penalized tokens list.
- Generation parameters.
  - `/temp 0.7` sets the temperature.
  - `/top_k 40` sets the `top_k` parameter.
  - `/top_p 0.9` sets the `top_p` parameter.
- Execution parameters.
  - `/thread_count 8` sets the number of threads.
  - `/batch_count 8` sets the batch size.

