use std::io::Read;
use std::ops::Range;
use std::process::{Child, ChildStdout, Stdio};
use thiserror::Error;

pub struct Command {
    cmd: String,
}

impl Command {
    pub fn new(cmd: String) -> Command {
        Command { cmd }
    }

    /// Starts new process in the background and returns iterator over it's stdout.
    /// Returns `Err` if process cannot be started.
    pub fn start_process_command(&self) -> std::io::Result<OutputIterator> {
        let child = std::process::Command::new("sh")
            .arg("-c")
            .arg(self.cmd.as_str())
            .stdout(Stdio::piped())
            .spawn();

        match child {
            Ok(mut process) => {
                let stdout = process.stdout.take().unwrap();

                Ok(OutputIterator {
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
pub struct OutputIterator {
    buffer: [u8; BUFFER_SIZE],
    buffer_read_start: usize,
    process: Child,
    stdout: ChildStdout,
}

#[derive(Debug, Error)]
pub enum OutputIteratorError {
    #[error("Cannot check process status: {0}")]
    CheckProcessStatusFailed(std::io::Error),
    #[error("Cannot read process output: {0}")]
    ReadFailed(std::io::Error),
}

impl OutputIterator {
    /// Reads and returns available `stdout` of command process as `Ok(Some<&[u8]>)` if process is still running or some data is available.
    /// Returns `Some(None)` if command process finished and there is 0 bytes available to read.
    /// Method automatically kills command process and returns `Err(CommandOutputIteratorError)` if error occurred while checking command process status or reading data.
    ///
    /// Note that `Ok(Some(&[]))` is possible if there is no new data available in the buffer but process is still running
    pub fn read(&mut self) -> Result<Option<&[u8]>, OutputIteratorError> {
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
                return Err(OutputIteratorError::CheckProcessStatusFailed(e));
            }
        };

        return match self.stdout.read(&mut self.buffer[self.buffer_read_start..]) {
            Ok(read_bytes) => {
                if process_finished && read_bytes == 0 {
                    return Ok(None); // end of process and it's output
                }

                let end_of_buffer = self.buffer_read_start + read_bytes;
                Ok(Some(&self.buffer[..end_of_buffer]))
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
                Err(OutputIteratorError::ReadFailed(e))
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

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn should_start_simple_process_and_read_output() {
        let mut command_output = start_command("echo hello");

        let first_read = command_output.read().unwrap().unwrap();

        assert_eq!(b"hello\n", first_read);

        while let Some(output) = command_output.read().unwrap() {
            assert!(output.is_empty()); //TODO how to match empty slice better?
        }

        let last_read = command_output.read().unwrap();
        assert_eq!(None, last_read);
    }

    fn start_command(cmd: &str) -> OutputIterator {
        let mut command_output = Command::new(cmd.to_owned())
            .start_process_command()
            .unwrap();
        command_output
    }

    #[test]
    fn should_await_till_process_finish() {
        let mut command_output = start_command("sleep 0.1");

        let first_read = command_output.read().unwrap();

        // TODO first should be Some(empty slice)

        todo!("implement waiting until None");
    }
}
