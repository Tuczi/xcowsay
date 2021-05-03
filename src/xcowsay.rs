extern crate x11;

use std::{mem, thread};
use std::ffi::CString;
use std::os::raw::*;

use control_code::{CSI};
use x11::xlib;

use crate::config::Opt;
use crate::control_code_parse::XCowsayParser;
use crate::rgb_color::RgbColor;
use crate::xcontext::{XContext};
use crate::xdraw::XCowsayDrawer;
use std::str::from_utf8;
use self::x11::xlib::XEvent;
use std::time::Duration;
use crate::command::{Command, CommandOutputIterator};

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

    fn clear_line(&mut self, erase_mode: CSI::Erase);//TODO use type
    fn clear_display(&mut self, erase_mode: CSI::Erase);//TODO use type

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
        log::debug!("XContext init: {:?}", xcontext);
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

    //TODO refactor, maybe move parts of the logic to drawer
    //TODO check for safer code here https://github.com/erlepereira/x11-rs/blob/master/x11/examples/input.rs
    pub fn start_xevent_loop(&mut self) {
        log::debug!("Preparing xevent loop");
        // Hook close requests.
        let wm_protocols_str = CString::new("WM_PROTOCOLS").unwrap(); // safe unwrap as string does not have 0 char
        let wm_delete_window_str = CString::new("WM_DELETE_WINDOW").unwrap(); // safe unwrap as string does not have 0 char

        // This is safe as per `XContext` guaranties
        let wm_protocols = unsafe {
            xlib::XInternAtom(
                self.xcontext.display,
                wm_protocols_str.as_ptr(),
                xlib::False,
            )
        };
        // This is safe as per `XContext` guaranties
        let wm_delete_window = unsafe {
            xlib::XInternAtom(
                self.xcontext.display,
                wm_delete_window_str.as_ptr(),
                xlib::False,
            )
        };

        let mut protocols = [wm_delete_window];

        // This is safe as per `XContext` guaranties
        unsafe {
            xlib::XSetWMProtocols(
                self.xcontext.display,
                self.xcontext.window,
                protocols.as_mut_ptr(),
                protocols.len() as c_int,
            );
        }

        self.send_dummy_event();

        // This is safe as value is init inside the loop using XNextEvent call
        let mut event: xlib::XEvent = unsafe {
            mem::MaybeUninit::uninit().assume_init()
        };


        log::debug!("Starting xevent loop");
        loop {
            // This is safe as per `XContext` guaranties
            unsafe { xlib::XNextEvent(self.xcontext.display, &mut event); }
            log::debug!("Next Xevent");
            match event.get_type() {
                xlib::ClientMessage => {
                    let xclient = xlib::XClientMessageEvent::from(event);

                    if xclient.message_type == wm_protocols && xclient.format == 32 { //TODO should we compare message_type with array or specific event type?
                        let protocol = xclient.data.get_long(0) as xlib::Atom;
                        if protocol == wm_delete_window {
                            break;
                        }
                    } else {
                        self.drawer.prepare_new_frame();

                        if let Ok(mut process) = self.command.start_process_command() {
                            self.parse_process_output(&mut process);
                        } //TODO handle error

                        self.send_dummy_event();
                        thread::sleep( self.delay);
                    }
                }
                _ => (),
            }
        }
    }

    fn send_dummy_event(&self) {
        log::debug!("Sending dummy event");
        let dummy_event = xlib::XClientMessageEvent {
            type_: xlib::ClientMessage,
            serial: 0,
            send_event: 0,
            display : self.xcontext.display,
            window : self.xcontext.window,
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
            let text = from_utf8(read_bytes).unwrap();//TODO handle error

            let chars_parsed = self.parser.parse(&text, &mut self.drawer);
            self.drawer.flush();

            //TODO doesn't work for utf-8
            let len = read_bytes.len();
            // copy unparsed data to the beginning
            output.copy_leftovers(chars_parsed..len);
        }
    }
}
