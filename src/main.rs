#![warn(
    rust_2018_idioms,
    future_incompatible,
    nonstandard_style,
    unused,
    rust_2021_compatibility,
    clippy::all,
    // clippy::pedantic, // TODO enable?
    clippy::cargo,
    missing_docs
)]
#![deny(warnings)]
//! `fortune | cowsay` as screensaver in `XScreenSaver` and xfce4-screensaver!
//!
//! xcowsay allows you to run **ANY** Linux command and display the output as your screensaver.
//! Command will be refreshed (rerun) by xcowsay automatically.
//!
//! It supports:
//! 1. Colorful text output (CSI codes)
//! 2. Refreshing and animations
//! 3. Multiple fonts
//!
//! For details see [`README.md`](https://github.com/Tuczi/xcowsay/blob/master/README.md)
use crate::config::Opt;

mod command;
mod config;
mod control_code_parse;
mod rgb_color;
mod xcontext;
mod xcowsay;
mod xdraw;

//TODO implement Bold SGR (and braightning that color a bit) e.g. like this https://github.com/kovidgoyal/kitty/pull/3234/files
//TODO fix some small bugs in:
// - `sl` steam locomotive
// - `asciiquarium`

fn main() {
    let opts = config::from_args();
    setup_logger(&opts);
    let result = std::panic::catch_unwind(|| {
        let mut xcowsay = xcowsay::XCowsay::new(&opts);
        log::info!("Xcowsay init finished");
        xcowsay.start_loop();
        log::info!("xevent loop finished");
    });

    if result.is_err() {
        log::error!("xcowsay panic: {:?}", result.err().unwrap());
    } else {
        log::info!("Exiting xcowsay");
    }
}

fn setup_logger(opts: &Opt) {
    if opts.debug {
        stderrlog::new()
            .module(module_path!())
            .verbosity(3) //log::LevelFilter::Debug
            .color(stderrlog::ColorChoice::AlwaysAnsi)
            .init()
            .unwrap();
    } else {
        simple_logging::log_to_file("/tmp/xcowsay-test.log", log::LevelFilter::Warn).unwrap();
    };
}
