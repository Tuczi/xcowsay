# xcowsay
"```fortune | cowsay```" as screensaver in XScreenSaver

## Install
1. run ```./install.sh``` or use cmake
2. add module to ```~/.xscreensaver``` - modify ```programs: xcowsay```

For more info about installing own module see: <http://www.dis.uniroma1.it/~liberato/screensaver/install.html>.

## Colors
Xcowsay respects CSI (ANSI escape sequences) so your output can be colored!
Try setup below command in settings:

```fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term```
