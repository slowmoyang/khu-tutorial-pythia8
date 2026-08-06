#include "Pythia8/Pythia.h"
#include "Pythia8/PythiaStdlib.h"
#include "Pythia8Plugins/HepMC3.h"

#include "argparse/argparse.hpp"

#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


// Pythia8 accepts Random:seed up to 900000000; a negative value selects its
// built-in default seed and 0 seeds from the clock.
constexpr int kMaxSeed = 900000000;


void run(
    const fs::path cmnd_file_path,
    const fs::path output_file_path,
    const long num_events,
    const std::optional<int> seed
) {
  if (not fs::exists(cmnd_file_path)) {
    throw std::runtime_error(std::string{"file not found: "} + cmnd_file_path.c_str());
  }

  if (fs::exists(output_file_path)) {
    throw std::runtime_error(std::string{"file already exists: "} + output_file_path.c_str());
  }

  if (seed.has_value() and (seed.value() > kMaxSeed)) {
    throw std::runtime_error("seed must not exceed " + std::to_string(kMaxSeed));
  }

  /////////////////////////////////////////////////////////////////////////////
  // Pythia
  /////////////////////////////////////////////////////////////////////////////
  Pythia8::Pythia pythia{};

  pythia.readFile(cmnd_file_path.string());

  if (seed.has_value()) {
    pythia.readString("Random:setSeed = on");
    pythia.settings.mode("Random:seed", seed.value());
  }

  if (not pythia.init()) {
    throw std::runtime_error("pythia.init() failed");
  }

  /////////////////////////////////////////////////////////////////////////////
  // Pythia8ToHepMC
  /////////////////////////////////////////////////////////////////////////////
  Pythia8::Pythia8ToHepMC hepmc_writer{output_file_path.c_str()};

  /////////////////////////////////////////////////////////////////////////////
  // Event loop
  /////////////////////////////////////////////////////////////////////////////
  const int max_errors = pythia.mode("Main:timesAllowErrors");
  int num_errors = 0;
  long event_index = 0;

  while ((event_index < num_events)) {
    if (not pythia.next()) {
      if (++num_errors < max_errors) {
        continue;
      }
      throw std::runtime_error("too many pythia.next() errors");
    }

    // Cross section (mb -> pb) stored per event, HepMC convention.
    hepmc_writer.fillNextEvent(pythia);
    hepmc_writer.writeEvent();

    ++event_index;

  }

  pythia.stat();

}


// main
int main(int argc, char* argv[]) {
  argparse::ArgumentParser parser{"run-pythia8"};

  parser.add_argument("-c", "--cmnd")
    .required()
    .help("pythia8 cmnd file");

  parser.add_argument("-o", "--output")
    .required()
    .help("output HepMC3 file");

  parser.add_argument("-n", "--num-events")
    .scan<'d', long>()
    .required()
    .help("if the value is negative, process all events");

  parser.add_argument("-s", "--seed")
    .scan<'d', int>()
    .help("random number seed (1-900000000); 0 seeds from the clock and a "
          "negative value selects the pythia8 default seed. if omitted, the "
          "Random:* settings of the .cmnd file are left untouched");

  try {
    parser.parse_args(argc, argv);

  } catch (const std::exception& err) {
    std::cerr << "😱😱😱: " << err.what() << std::endl;
    std::cerr << parser;
    return 1;
  }


  try {
    const fs::path cmnd_file = parser.get<std::string>("cmnd");
    const fs::path output_file = parser.get<std::string>("output");
    const long num_events = parser.get<long>("num-events");
    const std::optional<int> seed = parser.present<int>("seed");

    run(
      /*cmnd_file=*/cmnd_file,
      /*output_file=*/output_file,
      /*num_events=*/num_events,
      /*seed=*/seed
    );

  } catch (const std::exception& err) {
    std::cerr << "😱😱😱: " << err.what() << std::endl;
    return 1;
  }

  return 0;
}
