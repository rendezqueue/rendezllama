# Alpaca Assistant

This example must be used with [Alpaca](https://crfm.stanford.edu/2023/03/13/alpaca.html)-style models that are tuned to behave like an instruction-following assistant chatbot.
Most importantly, the model must be tuned to end the assistant's replies with an EOS token.

You might like to use this for code, so we constrain output by number of lines instead of number of sentences.
Just press enter to let it continue 10 more lines.
You can adjust this behavior in `setting.sxpb`:
```lisp
((sentence_terminals) "\n")
(sentence_limit 10)
(sentence_token_limit 1000)  ; Long enough for any reasonable line of text.
```

A also removed token penalization because penalties are very ad-hoc and this format works okay without them.
```lisp
(repeat_window 0)
```

## Prompt Format
Alpaca-style models put the user's text and the chatbot's response in markdown subsections, so it's a bit sparse.
```text
Below is an instruction that describes a task. Write a response that appropriately completes the request.

### Instruction:
Hello!

### Response:

```

The response sections all end with an EOS token, which is preserved in the context but is otherwise invisible.

