# Vicuna Assistant

This example should be used with [Vicuna](https://lmsys.org/blog/2023-03-30-vicuna/)-style models (specifically Vicuna v1.1) that are tuned to behave like an assistant chatbot.

AI assistants like to talk a lot, so this one often hits output limits.
Just press enter to let it continue.
If that's too annoying, increase these numbers in `setting.sxpb`:
```lisp
(sentence_limit 5)
(sentence_token_limit 50)
```

## Prompt Format
The Vicuna v1.1 format can be found [in the authors' FastChat repository](https://github.com/lm-sys/FastChat/blob/6ff8505ec80fc4b04d668f65d229f4f58bc449e0/fastchat/conversation.py#L393-L404).
We use a slightly modified version places an EOS token after the user's text:
```text
A chat between a curious user and an artificial intelligence assistant. The assistant gives helpful, detailed, and polite answers to the user's questions.

USER: Hello!</s> ASSISTANT: Hello there! How can I assist you today?</s>USER: ...
```
