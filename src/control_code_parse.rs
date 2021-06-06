use control_code::nom::{ErrorKind, IResult};
use control_code::C1::ControlSequence;
use control_code::CSI;
use control_code::SGR;
use control_code::{Control, C0};

use crate::rgb_color::RgbColor;
use crate::xcowsay::{DrawString, SetCursor, SetDisplay, EraseMode};

use std::collections::HashMap;
use std::str::from_utf8;
use control_code::CSI::Erase;

pub struct XCowsayParser {
    unimplemented_codes: HashMap<String, u32>,
}

macro_rules! log_unimplemented {
    ( $self:ident, $t:expr, $control_code:expr, $extra_log:expr ) => {{
        if log::log_enabled!(log::Level::Debug) {
            *$self
                .unimplemented_codes
                .entry(format!("{} {:?}", $t, $control_code))
                .or_insert(0) += 1;
        }
    }};
    ( $self:ident, $t:expr, $control_code:expr ) => {{
        if log::log_enabled!(log::Level::Debug) {
            *$self
                .unimplemented_codes
                .entry(format!("{} {:?}", $t, $control_code))
                .or_insert(0) += 1;
        }
    }};
}

impl From<CSI::Erase> for EraseMode {
    fn from(value: CSI::Erase) -> Self {
        match value {
            Erase::ToEnd => EraseMode::ToEnd,
            Erase::ToStart => EraseMode::ToStart,
            Erase::All => EraseMode::All,
        }
    }
}

impl XCowsayParser {
    pub fn new() -> XCowsayParser {
        XCowsayParser {
            unimplemented_codes: HashMap::new(),
        }
    }

    /// Returns number of bytes read from `text` slice
    pub fn parse<T: DrawString + SetDisplay + SetCursor>(
        &mut self,
        bytes: &[u8],
        callback: &mut T,
    ) -> usize {
        //TODO refactor this function to operate on bytes
        let text = from_utf8(bytes).unwrap(); //TODO handle error
        let mut i = 0;
        let chars_read ;
        loop {
            if i >= text.len() {
                chars_read = text.len();
                break;
            }

            let mut text_slice = &text[i..text.len()];
            let mut parse_result = control_code::parse(text_slice.as_bytes());

            // while plain text... TODO move plain_text reading to separate function
            let start = i;
            let mut end = start;
            while IResult::Error(ErrorKind::Custom(0)) == parse_result {
                end += 1;

                text_slice = &text[end..text.len()];
                parse_result = control_code::parse(text_slice.as_bytes());
            }

            callback.print_text(&text[start..end]);
            i = end;

            if i >= text.len() {
                chars_read = text.len();
                break;
            }

            match parse_result {
                IResult::Done(expr, Control::C0(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    let c0_result = self.on_c0(code, callback); //TODO use better return type to signalize end of stream
                    if c0_result.is_err() {
                        // Workaround to "hide" unsupported sequence
                        // if parser reports C0:Escape then read buffer till next letter - e.g. end of CSI sequence
                        // One of the known missing CSI cods is "10" aka "default font" used by asciiquarium as "\x1B[0;10m"
                        // This is safe as buffer is marked as Done (contains full sequence)
                        if &text_slice[..2] == "\x1B[" {
                            let end_of_csi =
                                text_slice.find(|c| char::is_ascii_alphabetic(&c)).unwrap() + 1;
                            log_unimplemented!(self, "ESC", &text_slice[..end_of_csi]);
                            chars_read = i + end_of_csi;
                        } else {
                            log_unimplemented!(self, "ESC", text_slice);
                            chars_read = i;
                        }

                        break;
                    }
                }
                IResult::Done(expr, Control::C1(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    self.on_c1(code, callback);
                }
                IResult::Done(expr, Control::DEC(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    log_unimplemented!(self, "DEC", code);
                }

                IResult::Incomplete(needed) => {
                    log_unimplemented!(self, "CONTROL INCOMPLETE", needed);
                    chars_read = i;
                    break;
                }

                IResult::Error(ErrorKind::Custom(0)) => {
                    log_unimplemented!(
                        self,
                        "CUSTOM_ERROR",
                        parse_result,
                        "first byte is not a start of control sequence"
                    );
                }

                IResult::Error(error) => {
                    log_unimplemented!(self, "ERROR", error);
                }
            }
            i += 1;
        }

        log::debug!("Unimplemented status codes: {:?}", self.unimplemented_codes);
        self.unimplemented_codes = HashMap::new();

        chars_read
    }

    fn on_c1<T: DrawString + SetDisplay + SetCursor>(
        &mut self,
        code: control_code::C1::T,
        callback: &mut T,
    ) {
        if let ControlSequence(csi) = code {
            match csi {
                CSI::SelectGraphicalRendition(sgrs) => {
                    for sgr in sgrs {
                        match sgr {
                            SGR::Reset => callback.reset_text_graphic(),
                            SGR::Foreground(sgr_color) => {
                                callback.set_foreground_color(RgbColor::from(
                                    sgr_color,
                                    RgbColor::white(),
                                ));
                            }
                            SGR::Background(SGR::Color::Default) => {
                                //ignore for now as we always have Default=Black background
                            }
                            SGR::Background(SGR::Color::Index(0)) => {
                                //ignore for now as we always have Index(0)=Black background
                            }
                            _ => log_unimplemented!(self, "C1 SGR", sgr),
                        }
                    }
                }
                CSI::EraseDisplay(erase_mode) => callback.clear_display(erase_mode.into()),
                CSI::EraseLine(erase_mode) => callback.clear_line(erase_mode.into()),
                CSI::DeleteCharacter(count) => callback.delete_character(count),
                CSI::CursorVerticalPosition(position) => callback.set_cursor_vertical(position),
                CSI::CursorHorizontalPosition(position) => callback.set_cursor_horizontal(position),
                CSI::CursorPosition { x, y } => {
                    callback.set_cursor_horizontal(x);
                    callback.set_cursor_vertical(y);
                }
                CSI::CursorUp(by) => callback.move_cursor_vertical(-(by as i32)),
                CSI::CursorDown(by) => callback.move_cursor_vertical(by as i32),
                CSI::CursorBack(by) => callback.move_cursor_horizontal(-(by as i32)),
                CSI::CursorForward(by) => callback.move_cursor_horizontal(by as i32),
                _ => log_unimplemented!(self, "C1 CSI", csi),
            }
        } else {
            log_unimplemented!(self, "C1", code);
        }
    }

    fn on_c0<T: DrawString + SetDisplay + SetCursor>(
        &mut self,
        code: control_code::C0::T,
        callback: &mut T,
    ) -> Result<(), ()> {
        match code {
            C0::LineFeed => {
                callback.set_cursor_horizontal(0);
                callback.move_cursor_vertical(1);
                Ok(())
            }
            C0::CarriageReturn => {
                callback.set_cursor_horizontal(0);
                Ok(())
            }
            C0::Escape => {
                // Can happen if parser does not support some CSI codes e.g. "\x1B[10m" - 10 means default font and its not supported
                Err(())
            }
            C0::Null => Err(()),
            _ => {
                log_unimplemented!(self, "C0", code);
                Ok(())
            }
        }
    }
}
