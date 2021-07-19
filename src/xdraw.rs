use std::ffi::CString;

use x11::xlib;

use crate::config::Opt;
use crate::rgb_color::RgbColor;
use crate::xcontext::XContext;
use crate::xcowsay::{DrawString, EraseMode, SetCursor, SetDisplay};
use rand::random;
use std::cmp::{max, min};
use x11::xlib::XFontStruct;

/// Cursor position in pixels.
/// Position (0, 0) represent left top corner of the screen.
struct CursorPosition {
    x: i32, //i32 for compatibility with c interface of xlib TODO consider changing it back to u32? looks like casting is required anyway
    y: i32,
}

/// Stores drawing context and provides abstraction layer for xlib.
pub struct XCowsayDrawer {
    xcontext: XContext,
    cursor_position: CursorPosition,
    pub random_new_cursor_position: bool,
}

impl XCowsayDrawer {
    fn set_new_cursor_position(&mut self) {
        self.cursor_position = if self.random_new_cursor_position {
            let x = (random::<u32>() % self.xcontext.window_attributes.width as u32 / 2) as i32; // TODO memorize it as this is the new line 0 column. so every setting cursor position should adjust that
            let y = (random::<u32>() % self.xcontext.window_attributes.height as u32 / 2) as i32; // TODO the same with y for consistency

            CursorPosition { x, y }
        } else {
            CursorPosition { x: 0, y: 0 }
        };

        self.cursor_position.y += self.font_ascent(); //ensure line is always fully visible
    }

    pub fn new(config: &Opt, xcontext: XContext) -> XCowsayDrawer {
        XCowsayDrawer {
            xcontext,
            cursor_position: CursorPosition { x: 0, y: 0 },
            random_new_cursor_position: config.randomize,
        }
    }

    /// Clears window and reset graphic attributes for a new frame
    pub fn prepare_new_frame(&mut self) {
        // This is safe as per `XContext` guaranties
        unsafe {
            xlib::XSetWindowBackground(
                self.xcontext.display,
                self.xcontext.window,
                xlib::XBlackPixel(self.xcontext.display, self.xcontext.screen),
            );
            xlib::XSetForeground(
                self.xcontext.display,
                self.xcontext.gc,
                xlib::XWhitePixel(self.xcontext.display, self.xcontext.screen),
            );
            xlib::XClearWindow(self.xcontext.display, self.xcontext.window);
        }

        self.set_new_cursor_position();
    }

    /// Flushes xlib display
    pub fn flush(&self) {
        // This is safe as per `XContext` guaranties
        unsafe {
            xlib::XFlush(self.xcontext.display); //TODO check result
        }
    }

    /// Returns maximum number of lines that can be used on display
    pub fn max_lines(&self) -> i32 {
        let max_height = self.xcontext.window_attributes.height;
        max_height / self.line_height()
    }

    /// Returns maximum number of columns that can be used on display
    pub fn max_columns(&self) -> i32 {
        let max_width = self.xcontext.window_attributes.width;
        max_width / self.char_width()
    }

    fn line_height(&self) -> i32 {
        self.font_ascent() + self.font_descent()
    }

    fn char_width(&self) -> i32 {
        let single_char_str = CString::new(" ").unwrap(); // TODO handle error

        // This is safe as we pass exact size of the text = 1
        unsafe { xlib::XTextWidth(self.xcontext.font, single_char_str.as_ptr(), 1) }
    }

    fn font(&self) -> XFontStruct {
        // This is safe as per `XContext` guaranties
        unsafe { *self.xcontext.font }
    }

    fn font_ascent(&self) -> i32 {
        self.font().ascent
    }

    fn font_descent(&self) -> i32 {
        self.font().descent
    }
}

impl DrawString for XCowsayDrawer {
    fn print_text(&mut self, text: &str) {
        if text.is_empty() {
            return;
        }

        let str = CString::new(text).unwrap(); // TODO handle error
        let line_height = self.line_height();
        // This is safe as per `XContext` guaranties and str is not null and it's length is equal to `text.len()`
        let text_width =
            unsafe { xlib::XTextWidth(self.xcontext.font, str.as_ptr(), text.len() as i32) };
        // This is safe as per `XContext` guaranties
        unsafe {
            xlib::XClearArea(
                self.xcontext.display,
                self.xcontext.window,
                self.cursor_position.x,
                self.cursor_position.y - (*self.xcontext.font).ascent,
                text_width as u32,
                line_height as u32,
                0,
            );
            xlib::XDrawString(
                self.xcontext.display,
                self.xcontext.window,
                self.xcontext.gc,
                self.cursor_position.x,
                self.cursor_position.y,
                str.as_ptr(),
                text.len() as i32,
            );
        }
        self.cursor_position.x += text_width;
    }
}

impl SetDisplay for XCowsayDrawer {
    fn set_foreground_color(&mut self, color: RgbColor) {
        // This is safe as per `XContext` guaranties
        unsafe {
            xlib::XSetForeground(self.xcontext.display, self.xcontext.gc, color.into());
        }
    }

    fn reset_text_graphic(&mut self) {
        self.set_foreground_color(RgbColor::white());
    }

    fn clear_line(&mut self, erase_mode: EraseMode) {
        let line_height = self.line_height();
        // This is safe as per `XContext` guaranties
        unsafe {
            match erase_mode {
                EraseMode::ToEnd => {
                    xlib::XClearArea(
                        self.xcontext.display,
                        self.xcontext.window,
                        self.cursor_position.x, // from cursor position
                        self.cursor_position.y - self.font_ascent(),
                        u32::MAX, // to end of line
                        line_height as u32,
                        0,
                    );
                }
                EraseMode::ToStart => {
                    xlib::XClearArea(
                        self.xcontext.display,
                        self.xcontext.window,
                        0, // from begging of line
                        self.cursor_position.y - self.font_ascent(),
                        self.cursor_position.x as u32, // to cursor position
                        line_height as u32,
                        0,
                    );
                }
                EraseMode::All => {
                    xlib::XClearArea(
                        self.xcontext.display,
                        self.xcontext.window,
                        0, // from begging of line
                        self.cursor_position.y - self.font_ascent(),
                        u32::MAX, // to end of line
                        line_height as u32,
                        0,
                    );
                }
            }
        }
    }

    fn clear_display(&mut self, erase_mode: EraseMode) {
        let line_height = self.line_height();
        // This is safe as per `XContext` guaranties
        unsafe {
            match erase_mode {
                EraseMode::ToEnd => {
                    self.clear_line(EraseMode::ToEnd);

                    xlib::XClearArea(
                        self.xcontext.display,
                        self.xcontext.window,
                        0, // from begging of the line below
                        self.cursor_position.y - self.font_ascent() + line_height,
                        u32::MAX, // to end of display
                        u32::MAX,
                        0,
                    );
                }
                EraseMode::ToStart => {
                    self.clear_line(EraseMode::ToStart);

                    xlib::XClearArea(
                        self.xcontext.display,
                        self.xcontext.window,
                        0, // from begging of display
                        0,
                        u32::MAX, // to end of the line above
                        (self.cursor_position.y - self.font_ascent()) as u32,
                        0,
                    );
                }
                EraseMode::All => {
                    xlib::XClearWindow(self.xcontext.display, self.xcontext.window);
                }
            }
        }
    }

    fn delete_character(&mut self, count: u32) {
        //TODO test and check if this is sufficient especially for fonts that have non-fixed char display size
        let line_height = self.line_height();
        let char_width = self.char_width();

        // This is safe as per `XContext` guaranties
        unsafe {
            //copy line from current position + n characters into current position
            xlib::XCopyArea(
                self.xcontext.display,
                self.xcontext.window,
                self.xcontext.window,
                self.xcontext.gc,
                self.cursor_position.x + (count as i32) * char_width,
                self.cursor_position.y - self.font_ascent(),
                u32::MAX,
                line_height as u32,
                self.cursor_position.x,
                self.cursor_position.y - self.font_ascent(),
            );
        }
    }
}

impl SetCursor for XCowsayDrawer {
    fn set_cursor_horizontal(&mut self, position: u32) {
        self.cursor_position.x = (position as i32) * self.char_width();

        // correct position if cursor is on edge
        self.cursor_position.x = min(
            self.xcontext.window_attributes.width,
            self.cursor_position.x,
        );
    }

    fn set_cursor_vertical(&mut self, position: u32) {
        self.cursor_position.y = (position as i32) * self.line_height() + self.font_ascent();

        // correct position if cursor is on edge
        self.cursor_position.y = min(
            self.xcontext.window_attributes.height - self.font_ascent(),
            self.cursor_position.y,
        );
    }

    fn move_cursor_horizontal(&mut self, by: i32) {
        self.cursor_position.x += by * self.char_width();

        // correct position if cursor is on edge
        self.cursor_position.x = min(
            self.xcontext.window_attributes.width,
            self.cursor_position.x,
        );
        self.cursor_position.x = max(0, self.cursor_position.x);
    }

    fn move_cursor_vertical(&mut self, by: i32) {
        self.cursor_position.y += by * self.line_height();

        // correct position if cursor is on edge
        self.cursor_position.y = min(
            self.xcontext.window_attributes.height - self.font_ascent(),
            self.cursor_position.y,
        );
        self.cursor_position.y = max(self.font_ascent(), self.cursor_position.y);
    }
}
