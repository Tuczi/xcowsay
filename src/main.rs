mod command;
mod config;
mod control_code_parse;
mod rgb_color;
mod xcontext;
mod xcowsay;
mod xdraw;

//TODO support
// - `sl` steam locomotive (right now cursor moving is problematic - not sure if curses.h reads screen size properly)
// - `asciiquarium`

fn main() {
    let opts = config::from_args();

    let mut xcowsay = xcowsay::XCowsay::new(opts);
    xcowsay.start_xevent_loop();
    xcowsay.close();
}
