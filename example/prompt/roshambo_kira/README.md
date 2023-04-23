# Roshambo Scenario

Here, you play as L in a grueling series of roshambo matches against Kira.
Your real goal is to discover Kira's true identity.

The easiest way is to exploit Kira's huberis.
He will readily agree to divulge his real name if you can beat him in roshambo.

## Quality
The prompt seems like it could be improved to make Kira talk more.
Sometimes he talks in depth, but not often.

It's difficult to get good results because repeated games also need to repeat tokens (obviously), so the repeated token penalty trick doesn't work very well.
That's why we use a short `repeat_window`, which tends to make Kira repeat himself.

