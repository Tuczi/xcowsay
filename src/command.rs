use crate::config::Opt;
use std::io::Read;
use std::ops::Range;
use std::process::{Child, ChildStdout, Stdio};

pub struct Command {
    cmd: String,
}

// TODO Create trait to support hexagonal architecture
impl Command {
    pub fn new(config: &Opt) -> Command {
        Command {
            cmd: config.cmd.clone(),
        }
    }

    /// Starts new process in the background and returns iterator over it's stdout.
    /// Returns `Err` if process cannot be started.
    pub fn start_process_command(&self) -> std::io::Result<CommandOutputIterator> {
        let child = std::process::Command::new("sh")
            .arg("-c")
            .arg(self.cmd.as_str())
            .stdout(Stdio::piped())
            .spawn();

        match child {
            Ok(mut process) => {
                let stdout = process.stdout.take().unwrap();

                Ok(CommandOutputIterator {
                    buffer: [0; BUFFER_SIZE],
                    buffer_read_start: 0,
                    process,
                    stdout,
                })
            }
            Err(e) => Err(e),
        }
    }
}

const BUFFER_SIZE: usize = 1024;

/// Custom iterator over `process`'s `stdout` that uses fixed size byte `buffer`.
pub struct CommandOutputIterator {
    buffer: [u8; BUFFER_SIZE],
    buffer_read_start: usize,
    process: Child,
    stdout: ChildStdout,
}

impl CommandOutputIterator {
    /// Reads available `stdout` as `Some<&[u8]>`.
    /// Returns `None` if there is no more data to read
    pub fn read(&mut self) -> Option<&[u8]> {
        let process_status = self.process.try_wait();
        let process_finished = match process_status {
            Ok(process_status) => process_status.is_some(),
            Err(e) => {
                let kill_result = self.process.kill();
                log::error!(
                    "Error checking process status: {:?}. Process killed with status: {:?}",
                    e,
                    kill_result
                );
                return None;
            }
        };

        return match self.stdout.read(&mut self.buffer[self.buffer_read_start..]) {
            Ok(read_bytes) => {
                if process_finished && read_bytes == 0 {
                    return None; // end of process and it's output
                }

                let end_of_buffer = self.buffer_read_start + read_bytes;
                Some(&self.buffer[..end_of_buffer])
            }
            Err(e) => {
                if process_finished {
                    log::error!("Error reading process output: {:?}.", e);
                } else {
                    let kill_result = self.process.kill();
                    log::error!(
                        "Error reading process output: {:?}. Process killed with status: {:?}",
                        e,
                        kill_result
                    );
                }
                None
            }
        };
    }

    /// Copies unconsumed buffer `range` to the beginning of the buffer.
    /// Next `read` call will return that data in the beginning of the buffer.
    ///
    /// # Dev notes
    /// Just copy the data instead of using some fancy data structure like ringbuffer.
    /// There are usually less than 5-10 bytes so there is no performance differance but "copy" implementation is simpler.
    pub fn copy_leftovers(&mut self, range: Range<usize>) {
        for i in 0..range.len() {
            self.buffer[i] = self.buffer[range.start + i];
        }

        self.buffer_read_start = range.len();
    }
}
