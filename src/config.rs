use structopt::StructOpt;

const COWSAY_AND_TOILET: &str = "fortune -a | fmt -80 -s | $(shuf -n 1 -e cowsay cowthink) -$(shuf -n 1 -e b d g p s t w y) -f $(shuf -n 1 -e $(cowsay -l | tail -n +2)) -n | toilet -F gay -f term";

#[derive(Debug, StructOpt)]
#[structopt(name = "XCowsay", about = "Low-level screensaver that displays your favourite colorful terminal output and ASCII art", setting = clap::AppSettings::AllowLeadingHyphen)]
pub struct Opt {
    #[structopt(
        short,
        long,
        default_value = COWSAY_AND_TOILET
    )]
    pub cmd: String,
    #[structopt(short, long, default_value = "5")]
    pub delay: u64, // TODO type Duration
    #[structopt(short, long, default_value = "-*-fixed-*-r-*-*-14-*-*-*-*-*-*-*")]//TODO fix passing font string as argument
    pub font: String,
    #[structopt(short, long)]
    pub randomize: bool,

    #[structopt(short = "D", long)]
    pub debug: bool,
}

pub fn from_args() -> Opt {
    Opt::from_args()
}


#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn should_parse_font_format() {
        let expected_font = "-*-test-*-r-*-*-14-*-*-*-*-*-*-*";
        let opts = Opt::from_iter(&["test", "-f", expected_font]);

        assert_eq!(expected_font, opts.font);
    }

    #[test]
    fn should_parse_command_with_space() {
        let expected_command = "echo hello";
        let opts = Opt::from_iter(&["test", "-c", expected_command]);

        assert_eq!(expected_command, opts.cmd);
    }

    #[test]
    fn should_parse_command_with_hyphen_param() {
        let expected_command = "sl -l";
        let opts = Opt::from_iter(&["test", "-c", expected_command]);

        assert_eq!(expected_command, opts.cmd);
    }

    #[test]
    fn should_parse_complex_command_and_accept_other_params() {
        let expected_command = "fortune -a | cowsay | toilet -F gay -f term";
        let opts = Opt::from_iter(&["test", "-c", expected_command, "-r"]);

        assert_eq!(expected_command, opts.cmd);
        assert!(opts.randomize);
    }
}