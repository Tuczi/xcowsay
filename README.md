# xcowsay

`fortune | cowsay` as screensaver in XScreenSaver and xfce4-screensaver!

xcowsay allows you to run ANY Linux command and display the output as your screensaver.
Command will be refreshed (rerun) by xcowsay automatically.

It supports:
1. Color text output!
2. Refreshing and animations
3. Multiple fonts

## Colors
Xcowsay respects CSI (ANSI escape sequences) so your output can be colored!
Try setup below command in settings:

```fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term```

## Requirements
1. CMake
2. libx11-dev

And one of the screensavers:
- XScreenSaver
- xfce4-screensaver

# Installation & configuration

## Compile xcowsay
1. Compile
To compile xcowsay run:
```
cmake CMakeLists.txt
make
```

## Install xcowsay for XScreenSaver
1. Add module to XScreenSaver configuration file. Edit `~/.xscreensaver` file and add `programs: xcowsay` (or modify `programs` section if exists).
*For more info about installing own module see: [http://www.dis.uniroma1.it/~liberato/screensaver/install.html].*
2. Copy xcowsay program to XScreenSaver binaries
3. Copy xcowsay.xml file to XScreenSaver configuration directory

E.g.
```
cp bin/xcowsay /usr/lib/xscreensaver
cp xscreen-config/xcowsay.xml /usr/share/xscreensaver/config
```
*Make sure that files has proper permissions set for read and execution*

## Install xcowsay for xfce4-screensaver
1. Copy xcowsay program to xfce4-screensaver binaries
2. Copy xcowsay.desktop file to xfce4-screensaver configuration directory
E.g. for Xubuntu:
```
cp bin/xcowsay /usr/libexec/xfce4-screensaver/xcowsay
cp xscreen-config/xcowsay.desktop /usr/share/applications/screensavers/xcowsay.desktop
```
*Make sure that files has proper permissions set for read and execution*

## Configuration
Following params can be configured in screensaver settings GUI:
* `-d` delay - delay to next command execution in seconds
* `-c` comamnd - command to be executed
* `-f` font - (advenced) font to be used to display the output of the command. Use [X logical font description (XLFD)](https://en.wikipedia.org/wiki/X_logical_font_description) format to specify the font e.g. `-*-fixed-*-r-*-*-14-*-*-*-*-*-*-*`.
  *Use `xlsfonts` command to list avaliable fonts*

**xfce4-screensaver settings GUI is not supported yet.** As a workaround you can edit `xcowsay.desktop` file and add your parameters to `Exec` section.

## Install special cows
*This step is optional.*
You can install additional cows for cowsay eg. colorfull christmas tree.
To do that just copy them to cowsay cows directory:
```
cp cows/* /usr/share/cowsay/cows
```

# TODO list:
- add proper configuration layer for xfce4-screensaver - check out https://git.xfce.org/apps/xfce4-screensaver/tree/savers/floaters.c
