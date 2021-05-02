use crate::config::Opt;

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
    setup_logger(&opts);

    let mut xcowsay = xcowsay::XCowsay::new(opts);
    xcowsay.start_xevent_loop();
    xcowsay.close();
}

fn setup_logger(opts: &Opt) {
    let log_level = if opts.debug {
        3 // log::LevelFilter::Debug
    } else {
        0 // log::LevelFilter::Error
    };

    stderrlog::new()
        .module(module_path!())
        .verbosity(log_level)
        .color(stderrlog::ColorChoice::AlwaysAnsi)
        .init()
        .unwrap();
}
