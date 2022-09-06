#ifndef FLARMIDFILE_H
#define FLARMIDFILE_H

#include <memory>
#include <unordered_map>
#include "tchar.h"

constexpr size_t FLARMID_SIZE_ID = 7;
constexpr size_t FLARMID_SIZE_NAME = 22;
constexpr size_t FLARMID_SIZE_AIRFIELD = 22;
constexpr size_t FLARMID_SIZE_TYPE = 22;
constexpr size_t FLARMID_SIZE_REG	= 8;
constexpr size_t FLARMID_SIZE_CN = (MAXFLARMCN + 1);
constexpr size_t FLARMID_SIZE_FREQ = 8;


struct FlarmId {
  explicit FlarmId(const std::string& string);
  explicit FlarmId(void);

  TCHAR id[FLARMID_SIZE_ID] = _T("");
  TCHAR name[FLARMID_SIZE_NAME] = _T("");
  TCHAR airfield[FLARMID_SIZE_AIRFIELD] = _T("");
  TCHAR type[FLARMID_SIZE_TYPE] = _T("");
  TCHAR reg[FLARMID_SIZE_REG] = _T("");
  TCHAR cn[FLARMID_SIZE_CN] = _T("");
  TCHAR freq[FLARMID_SIZE_FREQ] = _T("");

  uint32_t GetId() const;
};

typedef std::unique_ptr<FlarmId> FlarmId_ptr;
typedef std::unordered_map<uint32_t, FlarmId_ptr> FlarmIdMap;


class FlarmIdFile
{
private:
  FlarmIdMap flarmIds;

public:
  FlarmIdFile();
  ~FlarmIdFile();

    void ExtractParameter(const TCHAR *Source, 
				  TCHAR *Destination, 
				  int DesiredFieldNumber);

  void OGNIdFile(void);


  size_t Count() const {
    return flarmIds.size();
  }

  const FlarmId* GetFlarmIdItem(uint32_t id) const;
  const FlarmId* GetFlarmIdItem(const TCHAR *cn) const;
};

#endif
