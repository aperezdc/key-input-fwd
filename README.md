
This program listens on key input events on the terminal and generates
uinput key press/release events matching those.

The use-case is the following:

1. ssh to the computer which doesn't have any keyboard attached
2. run some program requiring access to user input events in
background
3. run key-input-fwd
4. start typing
5. evens should be forwarded to the program running in background.
