# Prompt Files and Format

## File
Only the priming and rolling prompts are required.
```lisp
; Priming prompt text file. This is the prompt that never changes.
(x_priming priming.txt)
; Rolling prompt. This is the initial chat dialogue.
; As the chat continues, older dialogue expires and "rolls" out of context.
(x_rolling "rolling.txt")
; Where to save the chat transcript as it rolls out of context and can no longer be edited.
(o_rolling "transcript.txt")

; A multi-line prefix to place before every generated line of chat.
; Try this for models like Alpaca that are fine-tuned to follow instructions.
(x_answer "answer.txt")
```

## Prefix
By default, there are only 2 characters: the protagonist (your input) and the confidant (filled in by the LLM).
```lisp
; Protagonist's name. Can be changed later via the `/protagonist User` command.
(protagonist "User")
; Confidant's name.  Can be changed later via the `/confidant Bot` command.
(confidant "Bot")

(substitution
  ; Replace "{{user}}" in the input prompts with the protagonist name.
  (protagonist_alias "{{user}}")
  ; Replace "{{char}}" in the input prompts with the confidant name.
  (confidant_alias "{{char}}")
)
```

You can also add more chat prefixes to help frame how the token generation.
```lisp
(((chat_prefixes)) "{{user}}:" "{{char}} feels:" "{{char}} wants:" "{{char}} plans:" "{{char}}:")
```

## Format
```lisp
; Put a space at the start the priming prompt (default on).
(startspace_on 1)

; Put a space at the start of every line in the prompts (default off).
; This changes how the first word of a line (usually a character name) is tokenized.
(linespace_on 1)
```

