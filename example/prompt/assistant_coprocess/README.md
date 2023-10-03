# Coprocess Assistant

This example should be used as a coprocess.

Spawn `./bld/src/chat/chat --x_setting example/prompt/assistant_coprocess/setting.sxpb` from another program and send it 2 kinds of messages on stdin:
- `/puts SomeName: A line of dialogue.`
  - Adds a line of dialogue to the context.
  - Don't expect any output from this.
- `/gets 500 Banterbot:`
  - Makes the chatbot say something.
  - Expect a line of dialogue from the chatbot on stdout. Limited to slightly over 500 bytes.

You'll want to change a few things in `setting.sxpb`:
```lisp
; The bot name.
(confidant "Banterbot")
; Your computer's number of threads.
(thread_count 2)
; Convenient way to set model if you don't want to use the --model flag.
(model "../relative/path/to/model-ggml-q4_0.gguf")
```

You'll probably also want to change the `priming.txt` prompt to do what you want.
Despite its name, the current "Banterbot" prompt yields very little banter.

