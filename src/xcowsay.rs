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
use std::io::Read;
use std::str::from_utf8;
use self::x11::xlib::XEvent;
use std::time::Duration;


pub struct XCowsay {
    xcontext: XContext,
    drawer: XCowsayDrawer,
    parser: XCowsayParser,
    delay: Duration
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
        let drawer = XCowsayDrawer::new(&config, xcontext.clone());
        let parser = XCowsayParser::new(&config);

        let delay = Duration::from_secs(config.delay);

        XCowsay {
            xcontext,
            drawer,
            parser,
            delay,
        }
    }

    pub fn close(&self) {
        self.xcontext.close();
    }

    //TODO refactor, maybe move parts of the logic to drawer
    //TODO check for safer code here https://github.com/erlepereira/x11-rs/blob/master/x11/examples/input.rs
    pub fn start_xevent_loop(&mut self) {
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

        // Main loop
        loop {
            // This is safe as per `XContext` guaranties
            unsafe { xlib::XNextEvent(self.xcontext.display, &mut event); }
            println!("Next Xevent");
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

                        if let Ok(mut process) = self.parser.command.start_process_command() {
                            let mut stdout = process.stdout.take().unwrap();

                            //TODO steam locomotive
                            // looks like cursor is updated improperly - check where its updated (e.g. display raw text)

                            const BUFFER_SIZE : usize = 1*1024; // TODO tune buffer size
                            let mut buffer = [0;BUFFER_SIZE];
                            let mut buffer_read_start = 0;
                            while let Ok(read_bytes) = stdout.read(&mut buffer[buffer_read_start..]) {
                                let end_of_buffer = buffer_read_start + read_bytes;
                                let text = from_utf8(&buffer).unwrap();//TODO handle error
                                let chars_parsed = self.parser.parse(&text[..end_of_buffer], &mut self.drawer);//TODO handle error

                                //TODO doesn't work for utf-8
                                buffer_read_start = end_of_buffer - chars_parsed;
                                if buffer_read_start > 0 {
                                    println!("AAA {} {} {}", read_bytes, chars_parsed, end_of_buffer);
                                    println!("Copying {} {:?}", buffer_read_start, &buffer[chars_parsed..end_of_buffer]);
                                }
                                for i in 0..buffer_read_start { //copy unparsed data to the beggining
                                    buffer[i] = buffer[chars_parsed + i];
                                }
                                self.drawer.flush();

                                //TODO process might be completed but output not fully read
                                let process_status = process.try_wait();
                                if process_status.is_err() {
                                    println!("Error checking proces status");
                                    process.kill();//TODO check result
                                    break;
                                } else if process_status.unwrap().is_some() && read_bytes == 0{ // TODO read should be execute here again to avoid race conditions
                                    break;
                                }

                            }//TODO handle error

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
}
