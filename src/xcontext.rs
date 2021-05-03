use std::os::raw::{c_int, c_ulong};

use crate::config::Opt;
use std::ffi::CString;
use std::mem;
use x11::xlib;
use x11::xlib::{Display, XFontStruct, XWindowAttributes};
use xlib::GC;

pub type XWindow = c_ulong;
pub type XScreen = c_int;

/// Keeps all necessary fields for drawing and provides raw pointers safety.
#[derive(Clone, Debug)]
pub struct XContext {
    pub display: *mut Display,
    pub screen: XScreen,
    pub window: XWindow,
    pub window_attributes: XWindowAttributes,
    pub gc: GC,
    pub font: *mut XFontStruct,
}

impl XContext {
    fn open_display() -> *mut Display {
        let display = CString::new(std::env::var("DISPLAY").unwrap()).unwrap(); // TODO handle errors
                                                                                // This is safe because display string is pre-validated
        let display = unsafe { xlib::XOpenDisplay(display.as_ptr()) };
        if display.is_null() {
            panic!("XOpenDisplay failed");
        }

        display
    }

    fn get_root_window(display: *mut xlib::Display, screen: XScreen, config: &Opt) -> XWindow {
        if config.debug {
            // create new window in debug mode
            // This is safe because display and screen are pre-validated
            unsafe {
                let window = xlib::XCreateSimpleWindow(
                    display,
                    xlib::XRootWindow(display, screen),
                    24,
                    48,
                    860, //1280
                    640, //720
                    1,
                    xlib::XWhitePixel(display, screen),
                    xlib::XBlackPixel(display, screen),
                );
                // Show window
                xlib::XMapWindow(display, window); //TODO check status
                return window;
            }
        }

        let window = std::env::var("XSCREENSAVER_WINDOW");
        if window.is_ok() {
            let window = window.unwrap(); // TODO handle error
            let tmp = CString::new(window).unwrap(); // TODO handle error
                                                     //needs c style format detecting
                                                     // This is safe as tmp is not null
            unsafe {
                let window = libc::strtoul(tmp.as_ptr(), std::ptr::null_mut(), 0);
                //TODO check for errors

                return window;
            }
        }

        //TODO log info about using default root as fallback
        // This is safe as display is pre-validated
        unsafe { xlib::XDefaultRootWindow(display) }
    }

    fn load_font(display: *mut Display, gc: xlib::GC, config: &Opt) -> *mut XFontStruct {
        let font = CString::new(config.font.as_str()).unwrap(); // TODO handle error
                                                                // This is safe as display is pre-validated and font is not null
        unsafe {
            let font = xlib::XLoadFont(display, font.as_ptr());
            //TODO error handling
            xlib::XSetFont(display, gc, font);

            // get font metrics
            let mut v: xlib::XGCValues = mem::MaybeUninit::uninit().assume_init(); // It is init in the next line using XGetGCValues
            xlib::XGetGCValues(display, gc, xlib::GCFont as c_ulong, &mut v);
            let font_struct = xlib::XQueryFont(display, v.font);
            //TODO test for this nullcheck as this is part of the contract
            if font_struct.is_null() {
                panic!("XQueryFont failed. Font \"{}\"", config.font);
            }

            font_struct
        }
    }

    fn read_window_attributes(display: *mut Display, window: XWindow) -> XWindowAttributes {
        // This is safe as display and window are pre-validated and not null
        unsafe {
            let mut window_attributes: xlib::XWindowAttributes =
                mem::MaybeUninit::uninit().assume_init(); // It is init in the next line using XGetWindowAttributes
            xlib::XGetWindowAttributes(display, window, &mut window_attributes);

            window_attributes
        }
    }

    /// Creates new XContext with all necessary fields for drawing.
    /// All fields (including raw painters) are validated and guaranteed to be valid for entire lifetime of XContext struct.
    pub fn new(config: &Opt) -> XContext {
        let display = XContext::open_display();
        // This is safe as display is pre-validated
        let screen = unsafe { xlib::XDefaultScreen(display) };
        let window = XContext::get_root_window(display, screen, config);

        // This is safe as display is pre-validated
        let gc = unsafe { xlib::XCreateGC(display, window, 0, std::ptr::null_mut()) };
        let font = XContext::load_font(display, gc, &config);

        let window_attributes = XContext::read_window_attributes(display, window);

        XContext {
            display,
            screen,
            window,
            window_attributes,
            gc,
            font,
        }
    }

    pub fn close(&self) {
        // This is safe as display is not null
        unsafe {
            xlib::XCloseDisplay(self.display);
        }
    }
}
