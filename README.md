# xcowsay
"<code>fortune | cowsay</code>" as screensaver in XScreenSaver

## Colors
It respects CSI (ANSI escape sequences) so your output can be colored!
Try setup below command in settings:

<code>fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term</code>
