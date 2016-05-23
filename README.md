# xcowsay
"<code>fortune | cowsay</code>" as screensaver in XScreenSaver

## Install
run make (as root to coppy proper files)
add module to ~/.xscreensaver - modify modify ```programs: xcowsay``` 
(for more info about installing own module see: http://www.dis.uniroma1.it/~liberato/screensaver/install.html)

## Colors
Xcowsay respects CSI (ANSI escape sequences) so your output can be colored!
try:

```fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term```
