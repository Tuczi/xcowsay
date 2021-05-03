extern crate x11;

use std::thread;

use control_code::CSI;
use x11::xlib;

use self::x11::xlib::XEvent;
use crate::command::{Command, CommandOutputIterator};
use crate::config::Opt;
use crate::control_code_parse::XCowsayParser;
use crate::rgb_color::RgbColor;
use crate::xcontext::XContext;
use crate::xdraw::XCowsayDrawer;
use std::str::from_utf8;
use std::time::Duration;
use std::io::Error;

pub struct XCowsay {
    xcontext: XContext,
    drawer: XCowsayDrawer,
    parser: XCowsayParser,
    command: Command,
    delay: Duration,
}

pub trait SetDisplay {
    fn set_foreground_color(&mut self, color: RgbColor);
    fn reset_text_graphic(&mut self);

    fn clear_line(&mut self, erase_mode: CSI::Erase); //TODO use type
    fn clear_display(&mut self, erase_mode: CSI::Erase); //TODO use type

    fn delete_character(&mut self, count: u32);
}

pub trait SetCursor {
    fn set_cursor_vertical(&mut self, position: u32);
    fn set_cursor_horizontal(&mut self, position: u32);

    fn move_cursor_horizontal(&mut self, by: i32);
}

pub trait DrawString {
    fn print_text(&mut self, text: &str);
}

impl XCowsay {
    pub fn new(config: Opt) -> XCowsay {
        let xcontext = XContext::new(&config);
        log::info!("XContext init: {:?}", xcontext);
        let drawer = XCowsayDrawer::new(&config, xcontext.clone());
        let parser = XCowsayParser::new(&config);
        let command = Command::new(&config);
        let delay = Duration::from_secs(config.delay);

        XCowsay {
            xcontext,
            drawer,
            parser,
            command,
            delay,
        }
    }

    pub fn close(&self) {
        self.xcontext.close();
    }

    //TODO I had to remove handling events as it looks like xfce-screensaver doesn't pass events and program hangs. Debug why
    pub fn start_xevent_loop(&mut self) {
        log::info!("Starting xevent loop");
        loop {
            self.drawer.prepare_new_frame();

            match self.command.start_process_command() {
                Ok(mut process) => self.parse_process_output(&mut process),
                Err(e) => log::error!("Cannot start command: {:?}", e),
            }

            self.send_dummy_event();
            log::info!("Command is fully processed. Going sleep for {:?}.", self.delay);
            thread::sleep(self.delay);
        }
    }

    fn send_dummy_event(&self) {
        let dummy_event = xlib::XClientMessageEvent {
            type_: xlib::ClientMessage,
            serial: 0,
            send_event: 0,
            display: self.xcontext.display,
            window: self.xcontext.window,
            message_type: 0,
            format: 32,
            data: Default::default(),
        }; //TODO how to distinguished this message from others e.g. exit event? XAtom?

        let mut tmp: XEvent = dummy_event.into();

        // This is safe as per `XContext` guaranties and dummy_event is not null
        unsafe {
            xlib::XSendEvent(self.xcontext.display, self.xcontext.window, 0, 0, &mut tmp);
            xlib::XFlush(self.xcontext.display);
        }
    }

    fn parse_process_output(&mut self, output: &mut CommandOutputIterator) {
        //TODO steam locomotive
        // looks like cursor is updated improperly - check where its updated (e.g. display raw text)

        while let Some(read_bytes) = output.read() {
            let text = from_utf8(read_bytes).unwrap(); //TODO handle error

            let chars_parsed = self.parser.parse(&text, &mut self.drawer);
            self.drawer.flush();

            //TODO doesn't work for utf-8
            let len = read_bytes.len();
            // copy unparsed data to the beginning
            output.copy_leftovers(chars_parsed..len);
        }
    }
}
