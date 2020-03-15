//
// Created by tuczi on 04.06.16.
//
#include "main.hpp"

int main(int argc, char *argv[]) {
  const auto options = xcowsay::OptionsFactory::fromArgs(argc, argv);
  auto xcowsay = xcowsay::XCowsayFactory::fromOptions(options);

  xcowsay.draw();
}
