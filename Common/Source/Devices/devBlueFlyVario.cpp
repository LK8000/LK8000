/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 * File:   devBlueFlyVario.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 décembre 2013, 21:00
 */

/*
 * based on Alistair Dikie orignal work :
 * https://github.com/alistairdickie/BlueFlyVario_Android/blob/master/src/com/bfv/hardware/HardwareParameters.java
 */


#include "externs.h"
#include "Baro.h"
#include "devLK8EX1.h"
#include "devFlyNet.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include <iterator>
#include <functional>
#include "resource.h"

namespace { // unamed namespaces

using std::placeholders::_1;

enum ValueType {
    TYPE_BOOLEAN,
    TYPE_DOUBLE,
    TYPE_INT,
    TYPE_INTOFFSET,
    TYPE_INTLIST
};

class CHardwareParameter {
public:

    CHardwareParameter()
            : _Code(), _MinHWVersion(), _Type(), _Factor(), _MinHWVal(), _MaxHWVal(), available() {
    }

    CHardwareParameter(const tstring& code, int minHWVersion, ValueType type, double factor, int minHWVal, int maxHWVal)
            : _Code(code), _MinHWVersion(minHWVersion), _Type(type), _Factor(factor), _MinHWVal(minHWVal), _MaxHWVal(maxHWVal), available() {
    }

    ~CHardwareParameter() {

    }

    inline void Value(const tstring& Val) { available = true; _Value = Val; }
    inline const tstring& Value() const { return _Value; }

    inline operator bool ()  const { return !_Code.empty(); }

    inline const tstring& Code() const { return _Code; }
    inline ValueType Type() const { return _Type; }
    inline int MinHwVersion() const { return _MinHWVersion; }

    double Min() const {
        if(Type() == TYPE_DOUBLE) {
            return _MinHWVal / _Factor;
        }
        if(Type() == TYPE_INT) {
            return _MinHWVal * _Factor;
        }
        if(Type() == TYPE_INTOFFSET) {
            return _MinHWVal + _Factor;
        }
        return _MinHWVal;
    }

    double Max() const {
        if(Type() == TYPE_DOUBLE) {
            return _MaxHWVal / _Factor;
        }
        if(Type() == TYPE_INT) {
            return _MaxHWVal * _Factor;
        }
        if(Type() == TYPE_INTOFFSET) {
            return _MaxHWVal + _Factor;
        }
        return _MaxHWVal;
    }

    double ValueDouble() const {
        return _tcstod(_Value.c_str(), NULL) / _Factor;
    }

    int ValueInt() const {
        int val = _tcstol(_Value.c_str(),NULL,10);
        if(Type() == TYPE_INTOFFSET) {
            val += _Factor;
        } else {
            val *= _Factor;
        }
        return val;
    }

    bool ValueBool() const {
        return ValueInt()!=0;
    }

    bool IsAvailable(int HWVersion) const { return _MinHWVersion <= HWVersion; }

    void operator=(double v) {
        TCHAR szTmp[30] = {0};
        _stprintf(szTmp, _T("%d"), (int)(v * _Factor));
        _Value = szTmp;
    }

    void operator=(int v) {
        TCHAR szTmp[30] = {0};
        if(Type() == TYPE_INTOFFSET) {
            _stprintf(szTmp, _T("%d"), (int)(v - _Factor));
        } else {
            _stprintf(szTmp, _T("%d"), (int)(v / _Factor));
        }
        _Value = szTmp;
    }

    void operator=(bool v) {
        _Value = (v?_T("1"):_T("0"));
    }

private:
    const tstring _Code;
    const int _MinHWVersion;
    const ValueType _Type;
    const double _Factor;
    const int _MinHWVal;
    const int _MaxHWVal;

    bool available;
    tstring _Value;
};

class CHardwareParameters {
    typedef std::map<tstring, CHardwareParameter> ParameterList_t;

public:
    typedef ParameterList_t::const_iterator const_iterator;
    typedef ParameterList_t::iterator iterator;
    typedef ParameterList_t::value_type value_type;

    CHardwareParameters() : _Keys(), _Values(), _HwVersion(7) {

    }

    inline void Add(const tstring& code, int minHWVersion, ValueType type, double factor, int minHWVal, int maxHWVal) {
        _ParameterList.insert(std::make_pair(code, CHardwareParameter(code, minHWVersion, type, factor, minHWVal, maxHWVal)));
    }

    inline void Clear() { _ParameterList.clear(); }
    inline const_iterator begin() const { return _ParameterList.begin(); }
    inline const_iterator end() const { return _ParameterList.end(); }
    inline iterator begin() { return _ParameterList.begin(); }
    inline iterator end() { return _ParameterList.end(); }

    CHardwareParameter& GetParameter(const tstring& Key) {
        static CHardwareParameter nullParameter;
        iterator It = _ParameterList.find(Key);
        if(It != _ParameterList.end()) {
            return It->second;
        }
        return nullParameter;
    }

    inline void updateHardwareSettingsKeys(TCHAR* line) {
        _Keys = line;
    }

    void updateHardwareSettingsValues(TCHAR* line) {
        _Values = line;
        if (!_Keys.empty() && !_Values.empty()) {

            tstring::size_type PrevPosKey = 0, PosKey = 0;
            tstring::size_type PrevPosVal = 0, PosVal = 0;
            if(((PosVal = _Values.find_first_of(_T(" \n"), PosVal)) != tstring::npos)) { //skip first Value
                PrevPosVal = ++PosVal;
                while ( ((PosKey = _Keys.find_first_of(_T(" \n"), PosKey)) != tstring::npos)
                        && ((PosVal = _Values.find_first_of(_T(" \n"), PosVal)) != tstring::npos) )
                {

                    if (PosKey > PrevPosKey) {
                        const tstring key = _Keys.substr(PrevPosKey, PosKey-PrevPosKey);
                        const tstring value = _Values.substr(PrevPosVal, PosVal-PrevPosVal);

                        ParameterList_t::iterator It = _ParameterList.find(key);
                        if (It != _ParameterList.end()) {
                            (*It).second.Value(value);
                        }
                    }
                    PrevPosKey = ++PosKey;
                    PrevPosVal = ++PosVal;
                }
            }
        }
    }

    inline void SetHardwareVersion(const TCHAR* line) {
        _HwVersion = _tcstol(line, NULL, 10);
    }

    inline int GetHardwareVersion() const { return _HwVersion; }

    void UpdateDevice(const CHardwareParameter& Param, ComPort* Com) const {
        if(Param.MinHwVersion() <= _HwVersion) {
            TCHAR szTmp[35] = {0};
            _stprintf(szTmp, _T("$%s %s*"), Param.Code().c_str(), Param.Value().c_str());
            Com->WriteString(szTmp);
        }
    }

    inline operator bool ()  const { return !_Keys.empty() && !_Values.empty(); }

private:
    ParameterList_t _ParameterList;

    tstring _Keys;
    tstring _Values;
    int _HwVersion;
};

typedef std::map<DeviceDescriptor_t*, CHardwareParameters> Device2Parameters_t;
Device2Parameters_t gHardwareParameters;

namespace dlgBlueFlyConfig {
    DeviceDescriptor_t* pDevice;
    bool Init = true;

    constexpr TCHAR const * lstPageName [] = { _T("1"), _T("2"), _T("3"), _T("4"), _T("5") };
    typedef std::vector<WindowControl*> lstPageWnd_t;
    lstPageWnd_t lstPageWnd;
    unsigned CurrentPage = 0;

    typedef std::map<DataField*, tstring> DataField2Parameter_t;
    DataField2Parameter_t AssocFieldParam;

    void NextPage(WndForm* wfDlg, int Step) {
        if( (CurrentPage+Step) < lstPageWnd.size() )  {
            lstPageWnd[CurrentPage]->SetVisible(false);
            CurrentPage+=Step;
            lstPageWnd[CurrentPage]->SetVisible(true);

            WindowControl * pWnd = wfDlg->FindByName(_T("cmdNext"));
            if(pWnd) {
                pWnd->SetVisible(CurrentPage<(lstPageWnd.size()-1));
            }
            pWnd = wfDlg->FindByName(_T("cmdPrev"));
            if(pWnd) {
                pWnd->SetVisible(CurrentPage>0);
            }

            TCHAR szTmp[50] = {0};
            _stprintf(szTmp, _T("BlueFlyVario %u/%u"), CurrentPage+1, (unsigned)lstPageWnd.size());
            wfDlg->SetCaption(szTmp);
        }
    }

    void OnClose(WndButton* pWnd) {
      if(pWnd) {
        WndForm * pForm = pWnd->GetParentWndForm();
        if(pForm) {
          pForm->SetModalResult(mrOK);
        }
      }
    }

    void OnNextClicked(WndButton* pWnd) {
        NextPage(pWnd->GetParentWndForm(), +1);
    }

    void OnPrevClicked(WndButton* pWnd) {
        NextPage(pWnd->GetParentWndForm(), -1);
    }

    void OnParamData(DataField *Sender, DataField::DataAccessKind_t Mode) {
        if(Init) {
            return;
        }
        DataField2Parameter_t::iterator It;
        switch(Mode){
            case DataField::daGet:
                break;
            case DataField::daPut:
            case DataField::daChange:
                It = AssocFieldParam.find(Sender);
                if(It != AssocFieldParam.end()) {
                    CHardwareParameters& Parameters = gHardwareParameters[pDevice];
                    CHardwareParameter& Param = Parameters.GetParameter(It->second);
                    switch(Param.Type()) {
                        case TYPE_BOOLEAN:
                            Param = Sender->GetAsBoolean();
                            break;
                        case TYPE_DOUBLE:
                            Param = Sender->GetAsFloat();
                            break;
                        case TYPE_INT:
                        case TYPE_INTOFFSET:
                        case TYPE_INTLIST:
                            Param = Sender->GetAsInteger();
                            break;
                    }

                    ScopeLock Lock(CritSec_Comm);
                    if(pDevice && pDevice->Com) {
                        Parameters.UpdateDevice(Param, pDevice->Com);
                    }
                }
                break;
            case DataField::daInc:
            case DataField::daDec:
            case DataField::daSpecial:
                break;
        }
    }

    void OnReset(WndButton* pWnd) {
        ScopeLock Lock(CritSec_Comm);
        if(dlgBlueFlyConfig::pDevice && dlgBlueFlyConfig::pDevice->Com) {
            dlgBlueFlyConfig::pDevice->Com->WriteString("$RSX*\n\r");
        }
    }

    CallBackTableEntry_t CallBackTable[] = {
        ClickNotifyCallbackEntry(OnClose),
        ClickNotifyCallbackEntry(OnNextClicked),
        ClickNotifyCallbackEntry(OnPrevClicked),
        ClickNotifyCallbackEntry(OnReset),
        DataAccessCallbackEntry(OnParamData),
        EndCallBackEntry()
    };

    void FillProperty(WndForm *wfDlg, int HWVersion, CHardwareParameters::value_type& Val) {
        if(!wfDlg) return;
        CHardwareParameter& Param = Val.second;

        WndProperty* pWnd = (WndProperty*)wfDlg->FindByName(Param.Code().c_str());
        if(pWnd) {
            DataField* pData = pWnd->GetDataField();
            if(pData) {
                AssocFieldParam[pData] = Param.Code();
                switch(Param.Type()) {
                    case TYPE_BOOLEAN:
                        pData->Set(Param.ValueBool());
                        break;
                    case TYPE_DOUBLE:
                        pData->SetMax(Param.Max());
                        pData->SetMin(Param.Min());
                        pData->Set(Param.ValueDouble());
                        break;
                    case TYPE_INT:
                    case TYPE_INTOFFSET:
                        pData->SetMax(Param.Max());
                        pData->SetMin(Param.Min());
                        pData->Set(Param.ValueInt());
                        break;
                    case TYPE_INTLIST:
                        pData->Set(Param.ValueInt());
                        break;
                }
            }
            pWnd->SetVisible(Param.IsAvailable(HWVersion));
            pWnd->RefreshDisplay();
            
        }
    }

    int Show(DeviceDescriptor_t* d) {
        int nRet = IdCancel;

        pDevice = d;
        Init = true;

        WndForm *wfDlg = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_BLUEFLYCONFIG_L : IDR_XML_BLUEFLYCONFIG_P);

        if (wfDlg) {
            // build list of page WindowConrol*
            lstPageWnd.clear();
            lstPageWnd.reserve(std::distance(std::begin(lstPageName), std::end(lstPageName)));
            std::transform(std::begin(lstPageName), std::end(lstPageName),
                    std::inserter(lstPageWnd, lstPageWnd.begin()),
                    std::bind(&WndForm::FindByName, wfDlg, _1));

            if(!lstPageWnd.empty()) {
                // Show First Page
                CurrentPage=0;
                NextPage(wfDlg, 0);

                // Hide All Next Page
                std::for_each(++lstPageWnd.begin(), lstPageWnd.end(), std::bind(&WindowControl::SetVisible, _1, false));
            }

            // Init Enum WndProperty
            WndProperty* pWnd = (WndProperty*)wfDlg->FindByName(_T("BOM"));
            if(pWnd) {
                DataField* pData = pWnd->GetDataField();
                if(pData) {
                    pData->addEnumText(_T("BlueFlyVario"));
                    pData->addEnumText(_T("LK8EX1"));
                    pData->addEnumText(_T("LX"));
                    pData->addEnumText(_T("FlyNet"));
                }
            }

            // Set Value to all WndProperty
            CHardwareParameters& HardwareParameters = gHardwareParameters[pDevice];
            AssocFieldParam.clear();
            std::for_each(HardwareParameters.begin(), HardwareParameters.end(), 
                            std::bind(&FillProperty, wfDlg, HardwareParameters.GetHardwareVersion(), _1));

            Init = false;
            if (wfDlg->ShowModal()) {
                nRet = IdOk;
            }
            AssocFieldParam.clear();
            lstPageWnd.clear();
            delete wfDlg;
        }

        pDevice = nullptr;

        return nRet;
    }
};

/**************************************************************************************************************************************************************/
unsigned RequestParamTimer = 10;

BOOL BlueFlyVarioOpen(DeviceDescriptor_t* d) {
    RequestParamTimer = 10;
    CHardwareParameters& HardwareParameters = gHardwareParameters[d];

    HardwareParameters.Clear();

    HardwareParameters.Add(_T("BQH"), 7, TYPE_INTOFFSET, 80000.0, 0, 65535);
    HardwareParameters.Add(_T("BFL"), 6, TYPE_DOUBLE, 100.0, 0, 1000);
    HardwareParameters.Add(_T("BOL"), 6, TYPE_DOUBLE, 100.0, 0, 1000);
    HardwareParameters.Add(_T("BFS"), 6, TYPE_DOUBLE, 100.0, 0, 1000);
    HardwareParameters.Add(_T("BOS"), 6, TYPE_DOUBLE, 100.0, 0, 1000);
    HardwareParameters.Add(_T("BFK"), 6, TYPE_DOUBLE, 1000.0, 10, 10000);

    HardwareParameters.Add(_T("BFQ"), 6, TYPE_INT, 1.0, 500, 2000);
    HardwareParameters.Add(_T("BFI"), 6, TYPE_INT, 1.0, 0, 1000);
    HardwareParameters.Add(_T("BSQ"), 6, TYPE_INT, 1.0, 250, 1000);
    HardwareParameters.Add(_T("BSI"), 6, TYPE_INT, 1.0, 0, 1000);
    HardwareParameters.Add(_T("BVL"), 6, TYPE_DOUBLE, 1000.0, 1, 1000);
    HardwareParameters.Add(_T("BRM"), 6, TYPE_DOUBLE, 100.0, 10, 100);

    HardwareParameters.Add(_T("BAC"), 6, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BAD"), 6, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BTH"), 6, TYPE_INT, 1.0, 0, 10000);
    HardwareParameters.Add(_T("BOM"), 7, TYPE_INTLIST, 1.0, 0, 3);
    HardwareParameters.Add(_T("BOF"), 7, TYPE_INT, 20.0, 1, 50);

    HardwareParameters.Add(_T("BHV"), 10, TYPE_INT, 1.0, 0, 10000);
    HardwareParameters.Add(_T("BHT"), 10, TYPE_INT, 1.0, 0, 10000);
    HardwareParameters.Add(_T("BLD"), 9, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BBZ"), 10, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BZT"), 10, TYPE_DOUBLE, 100.0, 0, 1000);

    HardwareParameters.Add(_T("BUP"), 11, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BRB"), 8, TYPE_INT, 1.0, 0, 655535);
    HardwareParameters.Add(_T("BR2"), 9, TYPE_INT, 1.0, 0, 655535);
    HardwareParameters.Add(_T("BPT"), 9, TYPE_BOOLEAN, 1.0, 0, 1);
    HardwareParameters.Add(_T("BUR"), 9, TYPE_BOOLEAN, 1.0, 0, 1);

    return TRUE;
}

bool RequestConfig(DeviceDescriptor_t* d) {
    // Request device Config
    const char szRequest[] = "$BST*";
    return d->Com->Write(szRequest, sizeof (szRequest));
}

BOOL BlueFlyVarioClose(DeviceDescriptor_t* d) {
    gHardwareParameters.erase(d);
    return TRUE;
}

BOOL BlueFlyConfig(DeviceDescriptor_t* d) {
    if(gHardwareParameters[d]) {
        return (dlgBlueFlyConfig::Show(d) == IdOk);
    }
    return FALSE;
}

BOOL PRS(DeviceDescriptor_t* d, TCHAR *String, NMEA_INFO *_INFO){
	UpdateBaroSource(_INFO, d, StaticPressureToQNHAltitude((HexStrToInt(String)*1.0)));
	return TRUE;
}

BOOL BlueFlyVarioParseNMEA(DeviceDescriptor_t* d, TCHAR *String, NMEA_INFO *pGPS) {
    if( RequestParamTimer == 1 ) {
        if(!RequestConfig(d)) {
            RequestParamTimer = 10;
        }
    }
    if(RequestParamTimer > 0){
        --RequestParamTimer;
    }

    if(_tcsncmp(TEXT("PRS "), String, 4)==0){
        return PRS(d, &String[4], pGPS);
    } 
    if(LK8EX1ParseNMEA(d, String, pGPS)) {
        return TRUE;
    }
    if(FlyNetParseNMEA(d, String, pGPS)) {
        return TRUE;
    }

    if (_tcsncmp(String, _T("BFV "), 4) == 0) {
        gHardwareParameters[d].SetHardwareVersion(&String[4]);
        return TRUE;
    } else if (_tcsncmp(String, _T("BST "), 4) == 0) {
        gHardwareParameters[d].updateHardwareSettingsKeys(&String[4]);
        return TRUE;
    } else if (_tcsncmp(String, _T("SET "), 4) == 0) {
        gHardwareParameters[d].updateHardwareSettingsValues(&String[4]);
        return TRUE;
    }

    return FALSE;
}

} // unamed namespaces

void BlueFlyInstall(DeviceDescriptor_t* d) {

    _tcscpy(d->Name, TEXT("BlueFlyVario"));
    d->ParseNMEA = BlueFlyVarioParseNMEA;
    d->Open = BlueFlyVarioOpen;
    d->Close = BlueFlyVarioClose;

    d->Config = BlueFlyConfig;
}

/**************************************************************************************************************************************************************/
