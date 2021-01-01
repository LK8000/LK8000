#ifndef _DIALOGS_DLGOBJECTSELECT_H_
#define _DIALOGS_DLGOBJECTSELECT_H_

#include "Compiler.h"
#include "tchar.h"
#include <memory>
#include <vector>
#include "WindowControls.h"
#include "utils/stringext.h"

class LKSurface;
struct GeoPoint;


struct ObjectAdaptor_t {
  constexpr static size_t npos = std::numeric_limits<size_t>::max();

  virtual ~ObjectAdaptor_t() {}

  virtual void Select() const = 0;
  virtual bool Toggle() const = 0;
  
  virtual const TCHAR* Name() const = 0;

  virtual const TCHAR* Type() const = 0;
  virtual bool FilterType(unsigned type) const = 0;
  
  virtual void DrawPicto(LKSurface& Surface, const RECT &rc) const = 0;

  virtual bool FilterName(const TCHAR* filter) const {
    return (ci_search_substr(Name(), filter));
  }

  virtual size_t MatchUpdate(const TCHAR* filter, std::set<TCHAR>& keys) const {
    size_t filter_size = _tcslen(filter);
    const TCHAR * first_match = ci_search_substr(Name(), filter);
    const TCHAR * next_match = first_match;
    while (next_match) {
      keys.insert(to_lower(*(next_match + filter_size)));
      next_match = ci_search_substr(next_match+1, filter);
    }
    
    return (first_match) 
        ? std::distance(Name(), first_match)
        : npos;
  }
};

struct ObjectSelectInfo_t {
  constexpr static size_t npos = ObjectAdaptor_t::npos;

  double Distance;
  double Direction;
  std::unique_ptr<ObjectAdaptor_t> object;

  void Select() const { 
    object->Select(); 
  }

  // return true to Close Dialog
  bool Toggle() const { 
    return object->Toggle(); 
  }

  const TCHAR* Name() const { 
    return object->Name(); 
  }

  bool FilterName(const TCHAR* filter) const {
    return object->FilterName(filter);
  }

  const TCHAR* Type() const {
    return object->Type();
  }

  bool FilterType(unsigned type) const {
    return object->FilterType(type);
  }
  
  void DrawPicto(LKSurface& Surface, const RECT &rc) const {
    object->DrawPicto(Surface, rc);
  }

  size_t MatchUpdate(const TCHAR* filter, std::set<TCHAR>& keys) const {
    return object->MatchUpdate(filter, keys);
  }
};

class dlgSelectObject {
public:

  using array_info_t = std::vector<ObjectSelectInfo_t>;
  constexpr static size_t NAMEFILTERLEN = 20;

  dlgSelectObject() = default;
  virtual ~dlgSelectObject() {}

  int DoModal();

  array_info_t& GetArrayInfo() { return array_info; }

  virtual const unsigned GetTypeCount() const = 0;
  virtual const TCHAR* GetTypeLabel(unsigned type) const = 0;

  virtual int GetTypeWidth(LKSurface& Surface) = 0; // size of type column in list (0 if no column type)

  void UpdateList();

  size_t GetVisibleCount() {
    return VisibleCount;
  }

  void SetDistanceFilterIdx(int idx);
  void SetDirectionFilterIdx(int idx);
  void SetTypeFilterIdx(int idx);
  void SetNameFilter(const TCHAR (&filter)[NAMEFILTERLEN]);

  unsigned GetDistanceFilterIdx() const {
    return DistanceFilterIdx;
  }

  unsigned GetDirectionFilterIdx() const {
    return DirectionFilterIdx;
  }

  unsigned GetTypeFilterIdx() const {
    return TypeFilterIdx;
  }

  const TCHAR* GetNameFilter() const {
    return sNameFilter;
  }

  virtual const TCHAR* GetFilterLabel() const = 0;

protected:

  virtual const TCHAR* GetCaption() const = 0;

  virtual array_info_t PrepareData(const GeoPoint& position) = 0;

  void OnFilterDistance(DataField *Sender, DataField::DataAccessKind_t Mode);
  void OnFilterDirection(DataField *Sender, DataField::DataAccessKind_t Mode);
  void OnFilterType(DataField *Sender, DataField::DataAccessKind_t Mode);

  void OnSelectClicked(WndButton* pWnd);
  void OnListEnter(WindowControl* pWnd,  WndListFrame::ListInfo_t *ListInfo);
  void ResetFilter(WndForm* pForm);
  void OnFilterName(WndButton* pWnd);
  void SetDirectionData(DataField *Sender);

  // DrawListIndex = number of things to draw
  size_t DrawListIndex = 0;
  void OnPaintListItem(WindowControl * Sender, LKSurface& Surface);
  void OnWpListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo);

  bool OnTimerNotify(WndForm* pForm);
  bool FormKeyDown(WndForm* pForm, unsigned KeyCode);

  WndListFrame* pWndList = nullptr;
  array_info_t array_info;

  size_t VisibleCount = 0; // number of visible item after filters applied.

  unsigned DistanceFilterIdx = 0;
  unsigned DirectionFilterIdx = 0;
  unsigned TypeFilterIdx = 0;

  int lastHeading = 0;

  TCHAR sNameFilter[NAMEFILTERLEN] = _T("");
};

#endif // _DIALOGS_DLGOBJECTSELECT_H_
