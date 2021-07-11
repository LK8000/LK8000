/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   language-check.cpp
 * Author: Bruno de Lacheisserie
 */
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>
#include "json.hpp"
#include "fifo_map.hpp"

namespace fs = std::filesystem;

// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
template<class K, class V, class dummy_compare, class A>
using fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<fifo_map>;


int main(int argc, char *argv[]) {
  std::cout << "check LK8000 language file" << std::endl;
  if (argc != 2) {
    std::cout << "usage : check <path>" << std::endl;
    return 1;
  }

  const fs::path work_folder =
      fs::path(argv[1]) / "Common" / "Data" / "Language" / "Translations";


  json default_language;
  {
    std::ifstream fstream(work_folder / "en.json", std::ifstream::in);
    fstream >> default_language;
  }


  for(auto& entry : fs::directory_iterator(work_folder)) {

    if(entry.is_directory()) {
      continue;
    }
    const fs::path& file_path = entry.path();
    if(file_path.extension().string() != ".json") {
      continue;
    }
    if(file_path == work_folder / "en.json") {
      continue;
    }

    json language; 
    {
      std::ifstream fstream(file_path, std::ifstream::in);
      fstream >> language;
    }

    json updated = json::object();
    for (const auto &p : language.items()) {
      auto it = default_language.find(p.key());
      if (it != default_language.end()) {
        if (it->get<std::string>() == p.value().get<std::string>()) {
          continue;
        } else if (it->get<std::string>() == "[deprecated]") {
          continue;
        }
      } else {
        continue;
      }
      updated[p.key()] = p.value();
    }
    std::ofstream out_file(file_path);
    out_file << updated.dump(4, ' ');
    out_file << std::endl;

    std::cout << file_path << ":" << updated.size() << "/" << language.size() << std::endl;

  }
}
