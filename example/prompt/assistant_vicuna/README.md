# Vicuna Assistant

This example should be used with [Vicuna](https://lmsys.org/blog/2023-03-30-vicuna/)-style models that are tuned to behave like an assistant chatbot.

AI assistants like to talk a lot, so this one often hits output limits.
Just press enter to let it continue.
If that's too annoying, increase these numbers in `setting.sxpb`:
```lisp
(sentence_limit 5)
(sentence_token_limit 50)
```

## Prompt Format
Vicuna-style models put the user's text and the chatbot's response on single lines (unless the reply is multi-line), so we don't do any weird formatting tricks like in the [Alpaca confidant example](../confidant_alpaca/).
The prompt in this example was copied from the Vicuna authors' [FastChat](https://github.com/lm-sys/FastChat/blob/ecb5f2bbd51d848ec63001462be6d7a79938b6d4/fastchat/conversation.py#L109-L112) repository and uses a simpler opening message:
```text
A chat between a curious human and an artificial intelligence assistant. The assistant gives helpful, detailed, and polite answers to the human's questions.

### Human: Hello!
### Assistant:
```

Other Vicuna-style models use a similar format but usually differ slightly.
To use those, please change the priming prompt in `priming.txt` and the character names in `setting.sxpb`.

