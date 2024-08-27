#include "../../inc/linker/linker.hpp"

using namespace std;
#include <getopt.h>
#include <iostream>

string convertStoO(const string &filename) {
  string output = filename;
  output.replace(output.end() - 1, output.end(), "o");
  return output;
}

int main(int argc, char** argv) {

  Linker *linker = Linker::getInstance();

  static struct option long_options[] = 
  {
    {"output", required_argument, 0, 'o'},
    {"place", required_argument, 0, 'p'},
    {"hex", no_argument, 0, 'h'},
    {"relocatable", no_argument, 0, 'r'},
    {0, 0 ,0 ,0}
  };

  string output_file;
  vector<string> input_files;

  unordered_map<string, uint32_t> section_addresses;

  bool hex_output = false;
  bool relocatable = false;

  int option_index = 0;
  int c;

  while ((c = getopt_long(argc, argv, "o:p:hr", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'o':
        output_file = optarg;
        break;

      case 'p': {
        // Parse --place=<section_name>@<address>
        string place_arg = optarg;
        size_t at_pos = place_arg.find('@');
        if (at_pos != string::npos) {
            string section_name = place_arg.substr(0, at_pos);  // Extract section name correctly
            unsigned long address = stoul(place_arg.substr(at_pos + 1), nullptr, 16);  // Convert address to a number
            if(address < 0x40000000) {
              cerr << "Invalid address for section " << section_name << ". Address must be greater than or equal to 0x40000000." << endl;
              return 1;
            }
            section_addresses[section_name] = address;
        } else {
            cerr << "Invalid format for --place option. Expected format: --place=<section_name>@<address>" << endl;
            return 1;
        }
        break;
      }

      case 'h':
        hex_output = true;
        break;

      case 'r':
        relocatable = true;
        break;

      default:
        break;
    }
  }

  // Handle input files (remaining command-line arguments)
  for (int i = optind; i < argc; i++) {
    input_files.push_back(argv[i]);
  }

  if(input_files.size() == 0) {
    cerr << "No input files provided." << endl;
    return 1;
  }

  if(hex_output && relocatable) {
    cerr << "Cannot output both hex and relocatable file." << endl;
    return 1;
  }

  if(!hex_output && !relocatable) {
    cerr << "Must specify output format: --hex or --relocatable" << endl;
    return 1;
  }

  if(hex_output && output_file.empty()) {
    output_file = "default_linker_output.hex";
  } else if(relocatable && output_file.empty()) {
    output_file = "default_linker_output.o";
  }

  int check = linker->link(input_files, output_file, section_addresses, hex_output, relocatable);

  if(check != 0) {
    cerr << "Linker: Error during linking." << endl;
    return 1;
  }

  cout << "Linker: Linking successful." << endl;

  return 0;
}