/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   language-convert.cpp
 * Author: Bruno de Lacheisserie
 */

#include <iostream>
#include <fstream> 
#include <filesystem>
#include <string>
#include <regex>
#include <cctype>

#include "picojson.h"

namespace fs = std::filesystem;
namespace json = picojson;

/*
The language is defined by a two-letter ISO 639-1 language code,
optionally followed by a two letter ISO 3166-1-alpha-2 region code

http://www.loc.gov/standards/iso639-2/php/code_list.php
https://www.iso.org/obp/ui/#iso:pub:PUB500001:en
*/

const char *language[][5] = {
    {"en",      "",             "ENG_MSG.TXT", "ENG_HELP.TXT", u8"English"},            // English
    {"cs",      "Translations", "CZE_MSG.TXT", "CZE_HELP.TXT", u8"Česky"},              // Czech
    {"es",      "Translations", "ESP_MSG.TXT", "ESP_HELP.TXT", u8"Español"},            // Spanish
    {"fr",      "Translations", "FRA_MSG.TXT", "FRA_HELP.TXT", u8"Français"},           // French
    {"de",      "Translations", "GER_MSG.TXT", "GER_HELP.TXT", u8"Deutsch"},            // German
    {"el",      "Translations", "GRE_MSG.TXT", "GRE_HELP.TXT", u8"Ελληνικά"},           // Greek
    {"hr",      "Translations", "HRV_MSG.TXT", "HRV_HELP.TXT", u8"Hrvatski"},           // Croatian
    {"hu",      "Translations", "HUN_MSG.TXT", "HUN_HELP.TXT", u8"magyar"},             // Hungarian
    {"it",      "Translations", "ITA_MSG.TXT", "ITA_HELP.TXT", u8"Italiano"},           // Italian
    {"nl",      "Translations", "NED_MSG.TXT", "NED_HELP.TXT", u8"Nederlands"},         // Dutch
    {"pl",      "Translations", "POL_MSG.TXT", "POL_HELP.TXT", u8"Polski"},             // Polish
    {"pt",      "Translations", "POR_MSG.TXT", "POR_HELP.TXT", u8"Português"},          // Portuguese
    {"pt_BR",   "Translations", "PTB_MSG.TXT", "PTB_HELP.TXT", u8"Português (Brasil)"}, // Portuguese (BRAZIL)
    {"ru",      "Translations", "RUS_MSG.TXT", "RUS_HELP.TXT", u8"русский язык"},       // Russian
    {"sr_Cyrl", "Translations", "SCI_MSG.TXT", "SCI_HELP.TXT", u8"српски (ћирилица)"},  // Serbian (Cyrillic)
    {"sr_Latn", "Translations", "SRB_MSG.TXT", "SRB_HELP.TXT", u8"srpski (latinica)"},  // Serbian (Latin)
    {"sv",      "Translations", "SWE_MSG.TXT", "SWE_HELP.TXT", u8"Svenska"},            // Swedish
    {"uk",      "Translations", "UKR_MSG.TXT", "UKR_HELP.TXT", u8"українська мова"},    // Ukrainian
    {"vi",      "Translations", "VIE_MSG.TXT", "VIE_HELP.TXT", u8"Tiếng Việt"},         // Viêt Namese
    {"zh",      "Translations", "CHN_MSG.TXT", "CHN_HELP.TXT", u8"繁體中文"}             // Traditional Chinese
};

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { 
	  return !std::isspace(ch); 
  }).base(), s.end());
}

// trim left (in place)
static inline void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), std::isspace));
}

// trim right and left (in place)
static inline void trim(std::string& s) {
  rtrim(s);
  ltrim(s);
}

int main(int argc, char *argv[]) {
  std::cout << "tranlate old LK8000 language file to json" << std::endl;
  if (argc != 2) {
    std::cout << "usage : convert <source path>" << std::endl;
    return 1;
  }

  const fs::path src_folder = fs::path(argv[1]) / "Common" / "Data" / "Language";

  json::object lang_list_json;
  json::object en_lang_json;

  for (const auto &p : language) {
    const auto code = p[0];
    const auto language = p[4];

    const fs::path msg_path = src_folder / p[1] / p[2];
    const fs::path hlp_path = src_folder / p[1] / p[3];
    const fs::path dst_path = src_folder / "Translations" / (std::string(p[0]) + ".json");

    lang_list_json[code] = json::value(language);

    json::object lang_json;

    {
      //  MSG file
      std::cout << msg_path << std::endl;

      const std::regex msg_regex(R"(_@M([0-9]*)_\s\"(.*)\"\s?)"); // key , value pair

      std::ifstream msg_file(msg_path);
      std::string src_line;
      while (std::getline(msg_file, src_line)) {
        // ignore Comments
        if (src_line.empty() || src_line.front() == '#') {
          continue;
        }
        std::smatch match;
        if (std::regex_search(src_line, match, msg_regex)) {
          std::string key = match[1];
          // to have token ordered in json file, all need to have same size (left '0' padded)
          while (key.length() < 6) {
            key = "0" + key;
          }
          key = "_@M" + key + "_";

          std::string value = match[2];

          // fix line break;
          value = std::regex_replace(value, std::regex(R"(\\r\\n)"), "\n");
          value = std::regex_replace(value, std::regex(R"(\\n)"), "\n");
          value = std::regex_replace(value, std::regex(R"(\\r)"), "\n");

          bool add = true;  
          if (!en_lang_json.empty()) {
            auto it = en_lang_json.find(key);
            if (it != en_lang_json.end()) {
              add = (it->second != json::value(value));
            }
          }
          if (add) {
            // and store <key, Value> pair
            lang_json[key] = json::value(value);
          }
        }
      }
    }

    {
      // HELP file
      std::cout << hlp_path << std::endl;

      std::string key;
      std::string value;

      std::ifstream hlp_file(hlp_path);
      std::string src_line;
      while (std::getline(hlp_file, src_line)) {
        // skip Comment
        if (!src_line.empty() && src_line.front() == '#') {
          continue;
        }

        if (!src_line.empty() && src_line.front() == '@') {
          // ligne starting by '@' folowed by digit it's token index

          if (!key.empty()) {
            // normalize previous key
            trim(key);
            if (isdigit(key[0])) {
              while (key.length() < 6) {
                key = "0" + key;
              }
            }
            key = "_@H" + key + "_";

            rtrim(value);
            // fix line break;
            value = std::regex_replace(value, std::regex(R"(\\r\\n)"), "\n");
            value = std::regex_replace(value, std::regex(R"(\\n)"), "\n");
            value = std::regex_replace(value, std::regex(R"(\\r)"), "\n");

            bool add = true;
            if (!en_lang_json.empty()) {
              auto it = en_lang_json.find(key);
              if (it != en_lang_json.end()) {
                add = (it->second != json::value(value));
              }
            }
            if (add) {
              // and store <key, Value> pair
              lang_json[key] = json::value(value);
            }
          }

          // get the new key value;
          key = src_line.substr(1);
          if (key.front() == 'H') {
            key = key.substr(1);
          }
          // and reset value;
          value.clear();
        } else {
          // all line folowing
          rtrim(src_line);
          value += src_line + "\n";
        }
      }
    }

    std::ofstream out_file(dst_path);
    out_file << json::value(lang_json).serialize(true);

    if (en_lang_json.empty()) {
      en_lang_json = lang_json;
    }
  }

  std::ofstream out_file(src_folder / "language.json");
  out_file << json::value(lang_list_json).serialize(true);

  return 0;
}
