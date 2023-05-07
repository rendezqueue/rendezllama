# Roshambo Scenario

Here, you play as L in a grueling series of roshambo matches against Kira.
Your real goal is to discover Kira's true identity.

## Strategy
With default settings, it's not very easy to have Kira reveal his name.

Some things you can try:
- Try complimenting his intelligence and then bet on the outcome of a game of roshambo.
  - This works better with smaller models like 7B.
- Type `/r` to regenerate Kira's response when he doesn't make sense.
- Add `(linespace_on 1)` to `setting.sxproto`.
  - For some reason, the different tokenization makes Kira more susceptible to influence.
- Add `(confidant "Light")` to `setting.sxproto` to rename Kira as Light.
  - Then literally just ask him who Kira is. Most times he just admits to being Kira.
- Type `/yield Ryuk` to bring Ryuk into the conversation.
  - Definitely try this if you got Kira's real name. Keep hitting enter to see the anime ending play out.
- Type `/yield Kira: My real name is` to make Kira start his message like that.
  - This is totally cheating.

## Quality
The prompt seems like it could be improved to make Kira talk more.
Sometimes he talks in depth, but not often.

It's difficult to get good results because repeated games also need to repeat tokens (obviously), so the repeated token penalty trick doesn't work very well.
That's why we use a short `repeat_window`, which tends to make Kira repeat himself.

