# Orca Assistant

This example must be used with [Orca](https://aka.ms/orca-lm)-style models that are tuned to behave like an instruction-following assistant chatbot.
Most importantly, the model must be tuned to end the assistant's replies with an EOS token.

I have only been able to test with [orca_mini_13b](https://huggingface.co/psmathur/orca_mini_13b), which is based on [OpenLLaMA](https://github.com/openlm-research/open_llama).

Most settings are identical to the [assistant_alpaca](../assistant_alpaca/) example.
