/*******************************************************************************
 * data/aol_data_prep.hpp
 *
 * Part of dftil - Distributed Full Text Index Library
 *
 * Copyright (C) 2016 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 2)
    std::cerr << "Please just enter the path to AOL user logs" << std::endl;

  std::ifstream in(argv[1]);
  if (!in)
    std::cerr << "Please just enter the path to AOL user logs" << std::endl;

  std::ofstream out("aol_prepared.txt");
  std::string line;
  while (std::getline(in, line)) {
      std::istringstream iss(line);
      std::vector<std::string> tokens;
      std::string token;
      while (std::getline(iss, token, '\t'))
        tokens.emplace_back(token);

      for (size_t i = 1; i < tokens.size() - 3; ++i)
        out << tokens[i] << ' ';
      if (tokens.size() > 4)
        out << std::endl;
  }
  out.flush();
  out.close();
  in.close();

  return 0;
}

/******************************************************************************/