/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "externs.h"
#include "FlarmIdFile.h"
#include "utils/array_back_insert_iterator.h"
#include "utils/zzip_stream.h"
#include <iostream>

namespace {

std::string::const_iterator GetAsString(std::string::const_iterator it, size_t size, TCHAR *res) {

  auto out = array_back_inserter(res, size - 1); // size - 1 to let placeholder for '\0'
  for(unsigned i = 0; i < (size -1); ++i) {
    out = (HexDigit(*(it++)) << 4) | HexDigit(*(it++));
  }
  *out = _T('\0');

  // remove trailing whitespace
  TCHAR* end = res + size - 1;
  while ((--end) >= res && _istspace(*end)) {
    *end = _T('\0');
  }
  return it;
}

} // namespace

FlarmId::FlarmId(const std::string& string) {
  if(string.length() != 172) {
    throw std::runtime_error("invalid flarmnet record");
  }

  auto it = string.begin();
  it = GetAsString(it, FLARMID_SIZE_ID, id);
  it = GetAsString(it, FLARMID_SIZE_NAME, name);
  it = GetAsString(it, FLARMID_SIZE_AIRFIELD, airfield);
  it = GetAsString(it, FLARMID_SIZE_TYPE, type);
  it = GetAsString(it, FLARMID_SIZE_REG, reg);
  it = GetAsString(it, FLARMID_SIZE_CN, cn);
  it = GetAsString(it, FLARMID_SIZE_FREQ, freq);


  // Add a valid CN if missing. Ex: D-6543 = D43
  if (_tcslen(cn) == 0 ) {
    int reglen=_tcslen(reg);
    if (reglen >=3) {
      cn[0] = reg[0];
      cn[1] = reg[reglen-2];
      cn[2] = reg[reglen-1];
      cn[3] = _T('\0');
    }
  }
}







void FlarmIdFile::ExtractParameter(const TCHAR *Source, 
				  TCHAR *Destination, 
				  int DesiredFieldNumber)
{
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength = _tcslen(Source);
  const TCHAR *sptr = Source;
  const TCHAR *eptr = Source+StringLength;

  if (!Destination) return;

  while( (CurrentFieldNumber < DesiredFieldNumber) && (sptr<eptr) )
    {
      if (*sptr == ','  )
        {
          CurrentFieldNumber++;
        }
      ++sptr;
    }

  Destination[0] = '\0'; // set to blank in case it's not found..

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (sptr < eptr)    &&
             (*sptr != ',') &&
             (*sptr != '\0') )
        {
          Destination[dest_index] = *sptr;
          ++sptr; 
					if(Destination[dest_index] != '\'') // remove '
	          ++dest_index;
        }
      Destination[dest_index] = '\0';
    }
}


void FlarmIdFile::OGNIdFile(void) {

  TCHAR OGNIdFileName[MAX_PATH] = _T("");
  LocalPath(OGNIdFileName, _T(LKD_CONF), _T("data.ogn"));

  /*
   * we can't use std::ifstream due to lack of unicode file name in mingw32
   */
  zzip_stream file(OGNIdFileName, "rt");
  if (!file) {
    return;
  }


  std::string src_line;
  src_line.reserve(512);
	unsigned int Doublicates= 0;
	unsigned int InvalidIDs = 0;
  std::istream stream(&file);
  std::getline(stream, src_line); // skip first line
  while (std::getline(stream, src_line)) {
    try {

      TCHAR *t_line = new TCHAR[src_line.size() + 1];

      std::copy(src_line.begin(), src_line.end(), t_line);
      t_line[src_line.size()] = '\0';

      auto flarmId = std::make_unique<FlarmId>();

			ExtractParameter(t_line, flarmId->id, 1);
		  ExtractParameter(t_line, flarmId->reg, 3);

			uint32_t RadioId = flarmId->GetId();

			if(_tcslen(flarmId->reg)==0) // valid registration?
			{
				_stprintf(flarmId->reg,_T("%X"),RadioId);
				InvalidIDs++;
			}

			{
			auto search = flarmIds.find(RadioId); 
      if (search == flarmIds.end()) // already exists?
			{
				ExtractParameter(t_line, flarmId->type, 2);
	      _stprintf(flarmId->name,_T("OGN: %X"),RadioId);
				ExtractParameter(t_line, flarmId->cn, 4);
/*

		 	  StartupStore(_T("==== %s"),NEWLINE);
 		    StartupStore(_T("OGN %s%s"),t_line,NEWLINE);
 		    StartupStore(_T("OGN ID=%s%s"),flarmId->id,NEWLINE);
 		    StartupStore(_T("OGN Type=%s%s"),flarmId->type,NEWLINE);
 		    StartupStore(_T("OGN Name=%s%s"),flarmId->name,NEWLINE);
 		    StartupStore(_T("OGN CN=%s%s"),flarmId->cn,NEWLINE);

*/
        auto ib = flarmIds.emplace(RadioId, std::move(flarmId));
		    assert(ib.second); // duplicated id ! invalid file ?
      }
			else
				Doublicates++;
			}

    } catch (std::exception& e) {
      StartupStore(_T("%s"), to_tstring(e.what()).c_str());
    }
  }
	if (InvalidIDs > 0)
	  StartupStore(_T(". found %u invalid IDs in OGN database %s"),InvalidIDs,NEWLINE);	
	if (Doublicates > 0)
	  StartupStore(_T(". found %u IDs also in OGN database -> ignored %s"),Doublicates,NEWLINE);
}






FlarmIdFile::FlarmIdFile() {

  TCHAR flarmIdFileName[MAX_PATH] = _T("");
  LocalPath(flarmIdFileName, _T(LKD_CONF), _T(LKF_FLARMNET));

  /*
   * we can't use std::ifstream due to lack of unicode file name in mingw32
   */
  zzip_stream file(flarmIdFileName, "rt");
  if (!file) {
    LocalPath(flarmIdFileName, _T(LKD_CONF), _T("data.fln"));
    file.open(flarmIdFileName, "rt");
  }
  if (!file) {
    return;
  }


  std::string src_line;
  src_line.reserve(173);

  std::istream stream(&file);
  std::getline(stream, src_line); // skip first line
  while (std::getline(stream, src_line)) {
    try {
      auto flarmId = std::make_unique<FlarmId>(src_line);
      auto ib = flarmIds.emplace(flarmId->GetId(), std::move(flarmId));
      assert(ib.second); // duplicated id ! invalid file ?
    } catch (std::exception& e) {
      StartupStore(_T("%s"), to_tstring(e.what()).c_str());
    }
  }

	unsigned int FlamnetCnt = (unsigned)flarmIds.size();
	StartupStore(_T(". FLARMNET database, found %u IDs"), FlamnetCnt);
  OGNIdFile();
	StartupStore(_T(". OGN database, found additinal %u IDs"), flarmIds.size() - FlamnetCnt);

	StartupStore(_T(". total %u Flarm device IDs found!"), flarmIds.size());
}

FlarmIdFile::~FlarmIdFile() {
  flarmIds.clear();
}

const FlarmId* FlarmIdFile::GetFlarmIdItem(uint32_t id) const {
  auto it = flarmIds.find(id);
  if (it != flarmIds.end()) {
    return it->second.get();
  }
  return nullptr;
}

const FlarmId* FlarmIdFile::GetFlarmIdItem(const TCHAR *cn) const {
  auto it = std::find_if(std::begin(flarmIds), std::end(flarmIds), [&](auto& item) {
    return (_tcscmp(item.second->cn, cn) == 0);
  });

  if (it != flarmIds.end()) {
    return it->second.get();
  }
  return nullptr;
}

uint32_t FlarmId::GetId() const {
  return _tcstoul(id, nullptr, 16);
}
