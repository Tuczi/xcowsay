use std::thread;

use crate::command::{Command, CommandOutputIterator};
use crate::config::Opt;
use crate::control_code_parse::XCowsayParser;
use crate::rgb_color::RgbColor;
use crate::xcontext::XContext;
use crate::xdraw::XCowsayDrawer;
use std::time::Duration;

pub struct XCowsay {
    drawer: XCowsayDrawer,
    parser: XCowsayParser,
    command: Command,
    delay: Duration,
}

#[derive(Debug)]
pub enum EraseMode {
    ToEnd,
    ToStart,
    All,
}

/// Interface for display graphic manipulation.
pub trait SetDisplay {
    fn set_foreground_color(&mut self, color: RgbColor);
    fn reset_text_graphic(&mut self);

    fn clear_line(&mut self, erase_mode: EraseMode);
    fn clear_display(&mut self, erase_mode: EraseMode);

    fn delete_character(&mut self, count: u32);
}

/// Interface for cursor position manipulation.
pub trait SetCursor {
    fn set_cursor_horizontal(&mut self, position: u32);
    fn set_cursor_vertical(&mut self, position: u32);

    fn move_cursor_horizontal(&mut self, by: i32);
    fn move_cursor_vertical(&mut self, by: i32);
}

/// Interface for drawing plain string.
pub trait DrawString {
    fn print_text(&mut self, text: &str);
}

impl XCowsay {
    pub fn new(config: Opt) -> XCowsay {
        let xcontext = XContext::new(&config);
        log::info!("XContext init: {:?}", xcontext);
        let drawer = XCowsayDrawer::new(&config, xcontext);
        let parser = XCowsayParser::new();
        let command = Command::new(config.cmd.clone());
        let delay = Duration::from_secs(config.delay);

        XCowsay {
            drawer,
            parser,
            command,
            delay,
        }
    }

    /// Starts main draw loop:
    /// 1. Run command
    /// 1. Read and parse command's output
    /// 1. Draw
    /// 1. Sleep
    pub fn start_loop(&mut self) {
        //I had to remove handling xevents as it looks like xfce-screensaver doesn't pass events and program hangs
        log::info!("Starting main loop");
        self.setup_envs_for_curses();

        loop {
            self.drawer.prepare_new_frame();

            match self.command.start_process_command() {
                Ok(mut process) => self.parse_process_output(&mut process),
                Err(e) => log::error!("Cannot start command: {:?}", e),
            }

            log::info!(
                "Command is fully processed. Going sleep for {:?}.",
                self.delay
            );
            thread::sleep(self.delay);
        }
    }

    fn parse_process_output(&mut self, output: &mut CommandOutputIterator) {
        while let Ok(Some(read_bytes)) = output.read() {
            let chars_parsed = self.parser.parse(read_bytes, &mut self.drawer);
            self.drawer.flush();

            let len = read_bytes.len();
            // copy unparsed data to the beginning
            output.copy_leftovers(chars_parsed..len);
        }
    }

    /// Setup `COLUMNS` and `LINES` envs so curses libs can detect terminal width and height properly
    fn setup_envs_for_curses(&self) {
        std::env::set_var("LINES", self.drawer.max_lines().to_string());
        std::env::set_var("COLUMNS", self.drawer.max_columns().to_string());
    }
}
