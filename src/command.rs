use crate::config::Opt;
use std::io::Error;
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

    pub fn start_process_command(&self) -> std::io::Result<Child> {
        std::process::Command::new("sh")
            .arg("-c")
            .arg(self.cmd.as_str())
            .stdout(Stdio::piped())
            .spawn()
    }


}
