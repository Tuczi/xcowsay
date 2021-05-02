use control_code::nom::{ErrorKind, IResult};
use control_code::{Control, C0};
use control_code::C1::ControlSequence;
use control_code::CSI;
use control_code::SGR;

use crate::command;
use crate::config::Opt;
use crate::rgb_color::RgbColor;
use crate::xcowsay::{DrawString, SetCursor, SetDisplay};

use std::collections::HashMap;

pub struct XCowsayParser {
    pub command: command::Command,//TODO move this ownership somewhere else?
    pub unimplemented_codes: HashMap<String, u32>,
}

macro_rules! log_unimplemented {
    ( $map:expr, $t:expr, $control_code:expr, $extra_log:expr ) => {
        {
           //println!("Unimplemented control code {} {:?}. {}", $t, $control_code, $extra_log);
           *$map.entry(format!("{} {:?}", $t, $control_code)).or_insert(0) += 1;

        }
    };
    ( $map:expr, $t:expr, $control_code:expr ) => {
        {
            //println!("Unimplemented control code {} {:?}", $t, $control_code);
            *$map.entry(format!("{} {:?}", $t, $control_code)).or_insert(0) += 1;
        }
    };
}

impl XCowsayParser {
    pub fn new(config: &Opt) -> XCowsayParser {
        let command = command::Command::new(config);

        XCowsayParser { command, unimplemented_codes: HashMap::new() }
    }

    /// Returns number of bytes read from `text` slice
    pub fn parse<T: DrawString + SetDisplay + SetCursor>(
        &mut self,
        text: &str,
        callback: &mut T,
    ) -> usize {
        let mut i = 0;
        let mut chars_read = 0;
        loop {
            if i >= text.len() {
                chars_read = text.len();
                break;
            }

            let mut text_slice = &text[i..text.len()];
            let mut parse_result = control_code::parse(text_slice.as_bytes());

            // while plain text...
            let start = i;
            let mut end = start;
            while IResult::Error(ErrorKind::Custom(0)) == parse_result {
                end += 1;

                text_slice = &text[end..text.len()];
                parse_result = control_code::parse(text_slice.as_bytes());
            }

            callback.print_text(&text[start..end]);
            i = end;

            match parse_result {
                IResult::Done(expr, Control::C0(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    let c0_result = self.on_c0(code, callback);//TODO use better return type to signalize end of stream
                    if c0_result.is_err() {
                        chars_read = i;
                        break;
                    }
                }
                IResult::Done(expr, Control::C1(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    self.on_c1(code, callback);
                }
                IResult::Done(expr, Control::DEC(code)) => {
                    i = text.len() - expr.len() - 1; //TODO doesn't work for utf-8
                    log_unimplemented!(&mut self.unimplemented_codes, "DEC", code);
                }

                IResult::Incomplete(needed) => {
                    log_unimplemented!(&mut self.unimplemented_codes, "CONTROL INCOMPLETE", needed);
                    chars_read = i;
                    break;
                }

                IResult::Error(ErrorKind::Custom(0)) => {
                    log_unimplemented!(&mut self.unimplemented_codes, "CUSTOM_ERROR", parse_result, "first byte is not a start of control sequence");
                }

                IResult::Error(error) => {
                    log_unimplemented!(&mut self.unimplemented_codes, "ERROR", error);
                }
            }
            i += 1;
        }

        println!("Unimplemented status codes: {:?}", self.unimplemented_codes);
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
                            SGR::Reset => {
                                callback.reset_text_graphic();
                            }

                            SGR::Foreground(sgr_color) => {
                                callback.set_foreground_color(RgbColor::from(sgr_color));
                            }

                            _ => {
                                log_unimplemented!(&mut self.unimplemented_codes, "C1 SGR", sgr);
                            }
                        }
                    }
                }
                CSI::EraseDisplay(erase_mode) => {
                    callback.clear_display(erase_mode);
                }
                CSI::EraseLine(erase_mode) => {
                    callback.clear_line(erase_mode);
                }

                CSI::DeleteCharacter(count) => {
                    callback.delete_character(count);
                }

                CSI::CursorVerticalPosition(position) => {
                    callback.set_cursor_vertical(position);
                }

                CSI::CursorHorizontalPosition(position) => {
                    callback.set_cursor_horizontal(position);
                }

                CSI::CursorPosition { x, y } => {
                    callback.set_cursor_vertical(x);
                    callback.set_cursor_horizontal(y);
                }

                CSI::CursorUp(by) => {
                    callback.move_cursor_horizontal(- (by as i32));
                }

                CSI::CursorDown(by) => {
                    callback.move_cursor_horizontal(by as i32);
                }

                _ => {
                    log_unimplemented!(&mut self.unimplemented_codes, "C1 CSI", csi);
                }
            }
        } else {
            log_unimplemented!(&mut self.unimplemented_codes, "C1", code);
        }
    }

    fn on_c0<T: DrawString + SetDisplay + SetCursor>(&mut self, code: control_code::C0::T, callback: &mut T) -> Result<(), ()>{
        match code {
            C0::LineFeed => {
                callback.set_cursor_vertical(0);
                callback.move_cursor_horizontal(1);
                Ok(())
            }
            C0::CarriageReturn => {
                callback.set_cursor_vertical(0);
                Ok(())
            }

            C0::Null => {
                Err(())
            }
            _ => {
                log_unimplemented!(&mut self.unimplemented_codes, "C0", code);
                Ok(())
            }
        }
    }
}
