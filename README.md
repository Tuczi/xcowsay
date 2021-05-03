# xcowsay

`fortune | cowsay` as screensaver in XScreenSaver and xfce4-screensaver!

xcowsay allows you to run **ANY** Linux command and display the output as your screensaver.
Command will be refreshed (rerun) by xcowsay automatically.

It supports:
1. Colorful text output (CSI codes)
2. Refreshing and animations
3. Multiple fonts

## Colors
xcowsay respects CSI (ANSI escape sequences) so your output can be colored!
Try setup below command in settings:

```shell
fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term
```

## Requirements
1. Cargo (Rust)
2. libx11-dev

And one of the screensavers:
- XScreenSaver
- xfce4-screensaver

# Installation & configuration

## Compile xcowsay
To compile xcowsay run:
```shell
cargo build --release
```

## Install xcowsay for XScreenSaver
1. Add module to XScreenSaver configuration file. Edit `~/.xscreensaver` file and add `programs: xcowsay` (or modify `programs` section if exists).
*For more info about installing own module see: [http://www.dis.uniroma1.it/~liberato/screensaver/install.html].*
2. Copy xcowsay program to XScreenSaver binaries
3. Copy xcowsay.xml file to XScreenSaver configuration directory

E.g.
```
cp target/release/xcowsay /usr/lib/xscreensaver
cp xscreen-config/xcowsay.xml /usr/share/xscreensaver/config
```
*Make sure that files have proper permissions set for read and execution*

## Install xcowsay for xfce4-screensaver
1. Copy xcowsay program to xfce4-screensaver binaries
2. Copy xcowsay.desktop file to xfce4-screensaver configuration directory
E.g. for Xubuntu:
```
cp target/release/xcowsay /usr/libexec/xfce4-screensaver/xcowsay
cp xscreen-config/xcowsay.desktop /usr/share/applications/screensavers/xcowsay.desktop
```
*Make sure that files has proper permissions set for read and execution*

## Configuration
Following params can be configured in screensaver settings GUI:
* `-d` delay - delay to next command execution in seconds
* `-c` command - command to be executed
* `-r` randomize - randomize starting position of the output
* `-f` font - (advenced) font to be used to display the output of the command. Use [X logical font description (XLFD)](https://en.wikipedia.org/wiki/X_logical_font_description) format to specify the font e.g. `-*-fixed-*-r-*-*-14-*-*-*-*-*-*-*`.
  *Use `xlsfonts` command to list avaliable fonts*

**xfce4-screensaver settings GUI is not supported yet.** As a workaround you can edit `xcowsay.desktop` file and add your parameters to `Exec` section.

Other options:
* `-D` - debug mode. It runs xcowsay in separate window instead of root window (screensaver)

## Install special cows
*This step is optional.*

You can install additional cows for cowsay eg. colorful christmas tree.
To do that just copy them to cowsay cows directory:
```
cp cows/* /usr/share/cowsay/cows
```

## Debug mode
Debug mode runs xcowsay in separate window instead of root window (screensaver). To enable it use `-D` switch.
E.g.: `cargo run -- -D -d 5 -c 'echo "Hello \e[31;42mXCowsay\e[0;32m (debug mode)\e[0m"'`

## Development
Style:
1. Use `rustfmt` code style.
2. Add blank line in the end of files.

# TODO list:
- add proper configuration layer for xfce4-screensaver - check out https://git.xfce.org/apps/xfce4-screensaver/tree/savers/floaters.c
