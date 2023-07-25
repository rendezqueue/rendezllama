# Alpaca Confidant

In this example, you chat with a whimsical character named Alpaca that sometimes spits at you.
It should used with [Alpaca](https://crfm.stanford.edu/2023/03/13/alpaca.html)-style models that are tuned to behave like an assistant chatbot with a specific format.

## Prompt Format
Alpaca-style models put the user's text and the chatbot's response after their name headers, so we do a little trick to condense them into single-line messages.
This is why "Alpaca" is a character in this example rather than being direct responses from the instruction-tuned model.
The prompt was adapted from the [SuperCOT-LoRA model card](https://huggingface.co/kaiokendev/SuperCOT-LoRA#prompting) and looks like:
```text
Below is an instruction that describes a task, paired with an input that provides further context. Write a response that appropriately completes the request.

### Instruction:
Suggest how to continue the input transcript of a conversation between User and Alpaca.

{{... description of Alpaca character ...}}

### Input:
User: Hello!
{{... more lines of dialogue as time goes on ...}}

### Response:
Based on the description of Alpaca and input chat history, the following would be a creative and realistic next line of dialogue.
Alpaca:
```

## Quality
Acknowledging the instruction in the response seems to help guide the transformer's attention mechanism (e.g., Alapaca reliably spits if it gets annoyed), but the flow of conversation sometimes suffers.

