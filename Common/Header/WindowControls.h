/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindowControls.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__WINDOWSCONTROL_H)
#define __WINDOWSCONTROL_H

#include "Screen/LKWindowSurface.h"
#include "Screen/BrushReference.h"
#include "Screen/PenReference.h"
#include "Screen/FontReference.h"
#include "Window/WndCtrlBase.h"
#include "Poco/Timestamp.h"

#define IsEmptyString(x)        ((x==NULL) || (x[0]=='\0'))

#define MAXSETCAPTION 254	// max chars in SetCaption, autolimited

#define BORDERTOP    (1<<bkTop)
#define BORDERRIGHT  (1<<bkRight)
#define BORDERBOTTOM (1<<bkBottom)
#define BORDERLEFT   (1<<bkLeft)

#define clBlack   RGB_BLACK;
#define clMaroon  LKColor(0x00,0x00,0x80)
#define clGreen   LKColor(0x00,0x80,0x00)
#define clOlive   LKColor(0x00,0x80,0x80)
#define clNavy    LKColor(0x80,0x00,0x00)
#define clPurple  LKColor(0x80,0x00,0x80)
#define clTeal    LKColor(0x80,0x80,0x00)
#define clGray    LKColor(0x80,0x80,0x80)
#define clSilver  LKColor(0xC0,0xC0,0xC0)
#define clRed     LKColor(0xFF,0x00,0xFF)
#define clLime    LKColor(0x00,0xFF,0x00)
#define clYellow  LKColor(0x00,0xFF,0xFF)
#define clBlue    LKColor(0xFF,0x00,0x00)
#define clFuchsia LKColor(0xFF,0x00,0xFF)
#define clAqua    LKColor(0xFF,0xFF,0x00)
#define clLtGray  LKColor(0xC0,0xC0,0xC0)
#define clDkGray  LKColor(0x80,0x80,0x80)
#define clWhite   LKColor(0xFF,0xFF,0xFF)
#define clNone    0x1FFFFFFF
#define clDefault 0x20000000

#define FORMATSIZE 32
#define UNITSIZE 10

    typedef struct {
      int ItemIndex;
      int DataFieldIndex;
      TCHAR *StringValue;
      TCHAR *StringValueFormatted;
    } ComboListEntry_t;

class ComboList{

  public:

    ComboList(void) {
      ComboPopupDrawListIndex=0;
      ComboPopupItemIndex=-1;
      ComboPopupItemSavedIndex=-1;
    }

#define ComboPopupLISTMAX 2000   // CAREFUL!
#define ComboPopupITEMMAX 100
#define ComboPopupReopenMOREDataIndex -800001
#define ComboPopupReopenLESSDataIndex -800002
#define ComboPopupNULL -800003

    ComboListEntry_t * CreateItem(  int ItemIndex, int DataFieldIndex,const TCHAR *StringValue,const TCHAR *StringValueFormatted);
    void FreeComboPopupItemList(void);

    int ComboPopupDrawListIndex;
    int ComboPopupItemIndex;
    int ComboPopupItemSavedIndex;
    int ComboPopupItemCount;
    ComboListEntry_t * ComboPopupItemList[ComboPopupLISTMAX]; // RLD make this dynamic later

    int PropertyDataFieldIndexSaved;
    TCHAR PropertyValueSaved[ComboPopupITEMMAX];
    TCHAR PropertyValueSavedFormatted[ComboPopupITEMMAX];


  private:

};


class DataField{

  public:

    typedef enum{
     daGet,
     daPut,
     daChange,
     daInc,
     daDec,
     daSpecial,
    }DataAccessKind_t;

    typedef void (*DataAccessCallback_t)(DataField * Sender, DataAccessKind_t Mode);

    DataField(const TCHAR *EditFormat, const TCHAR *DisplayFormat, 
	      void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)=NULL);
    virtual ~DataField(void){};

	virtual void Clear();
        
  virtual void Special(void);
  virtual void Inc(void);
  virtual void Dec(void);

  virtual void GetData(void);
  virtual void SetData(void);

  virtual bool GetAsBoolean(void){return(false);};
  virtual int GetAsInteger(void){return(0);};
  virtual double GetAsFloat(void){return(0);};
  virtual TCHAR *GetAsString(void){return(NULL);};
  virtual TCHAR *GetAsDisplayString(void){return(NULL);};

  virtual bool SetAsBoolean(bool Value){ (void)Value;
	  return(false);};
	  virtual int SetAsInteger(int Value){ (void)Value;
	  return(0);};
	  virtual double SetAsFloat(double Value){ (void) Value;
	  return(0.0);};
  virtual TCHAR *SetAsString(const TCHAR *Value){(void)Value; return(NULL);};

  virtual void Set(bool Value){ (void)Value; };
  virtual void Set(int Value){ (void)Value;};
  virtual void Set(double Value){ (void)Value; };
  virtual void Set(const TCHAR *Value){ (void)Value; };

  virtual int SetMin(int Value){(void)Value; return(0);};
  virtual double SetMin(double Value){(void)Value; return(false);};
  virtual int SetStep(int Value){(void)Value; return(0);};
  virtual double SetStep(double Value){(void)Value; return(false);};
  virtual int SetMax(int Value){(void)Value; return(0);};
  virtual double SetMax(double Value){(void)Value; return(0);};
  void SetUnits(const TCHAR *text) { _tcscpy(mUnits, text); }
  const TCHAR* GetUnits() const { return mUnits; }

  void Use(void){
    mUsageCounter++;
  }

  int Unuse(void){
    mUsageCounter--;
    return(mUsageCounter);
  }

  void SetDisplayFormat(const TCHAR *Value);
  void SetEditFormat(const TCHAR *Value);
  void SetDisableSpeedUp(bool bDisable) {mDisableSpeedup=bDisable;}  // allows combolist to iterate all values
  bool GetDisableSpeedUp(void) {return mDisableSpeedup;}
  void SetDetachGUI(bool bDetachGUI) {mDetachGUI=bDetachGUI;}  // allows combolist to iterate all values w/out triggering external events
  bool GetDetachGUI(void) {return mDetachGUI;}
  virtual int CreateComboList(void) {return 0;};
  virtual bool CreateKeyboard(void) {return false;}

  ComboList* GetCombo(void) { return &mComboList;}
  virtual int SetFromCombo(int iDataFieldIndex, TCHAR *sValue) {return SetAsInteger(iDataFieldIndex);};
  void CopyString(TCHAR * szStringOut, bool bFormatted);
  bool SupportCombo;  // all Types dataField support combolist except DataFieldString.
  protected:
    DataAccessCallback_t mOnDataAccess;
    TCHAR mEditFormat[FORMATSIZE+1];
    TCHAR mDisplayFormat[FORMATSIZE+1];
    TCHAR mUnits[UNITSIZE+1];
    ComboList mComboList;
    int CreateComboListStepping(void);

  private:

    int mUsageCounter;
    bool mDisableSpeedup;
    bool mDetachGUI;

};

class DataFieldBoolean:public DataField{

  private:
    bool mValue;
    TCHAR mTextTrue[FORMATSIZE+1];
    TCHAR mTextFalse[FORMATSIZE+1];

  public:
    DataFieldBoolean(const TCHAR *EditFormat, const TCHAR *DisplayFormat, int Default, const TCHAR *TextTrue, const TCHAR *TextFalse, void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
		  if (Default) {mValue=true;} else {mValue=false;}
      _tcscpy(mTextTrue, TextTrue);
      _tcscpy(mTextFalse, TextFalse);
      SupportCombo=true;

     (mOnDataAccess)(this, daGet);

    };

  void Inc(void);
  void Dec(void);
  int CreateComboList(void);

  bool GetAsBoolean(void);
  int GetAsInteger(void);
  double GetAsFloat(void);
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void){
    return(GetAsString());
  };

  virtual void Set(int Value){ 
    if (Value>0) 
      Set(true); 
    else 
      Set(false);
  };

  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(bool Value);
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  bool SetAsBoolean(bool Value);
  int SetAsInteger(int Value);
  double SetAsFloat(double Value);
  TCHAR *SetAsString(const TCHAR *Value);

};

// Max number of enumerated items. ATTENTION! Polars, infoboxe types etc.. are around 100 already!
// Anything more than this will NOT raise a bug message .
#define DFE_MAX_ENUMS 200

typedef struct {
  TCHAR *mText;
  unsigned int index;
} DataFieldEnumEntry;

class DataFieldEnum: public DataField {

  private:
    unsigned int nEnums;
    unsigned int mValue;
    DataFieldEnumEntry mEntries[DFE_MAX_ENUMS];

  public:
    DataFieldEnum(const TCHAR *EditFormat, 
		  const TCHAR *DisplayFormat, 
		  int Default, 
		  void(*OnDataAccess)(DataField *Sender, 
				      DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      SupportCombo=true;

      if (Default>=0) 
	{ mValue = Default; } 
      else 
	{mValue = 0;}
      nEnums = 0;
      if (mOnDataAccess) {
	(mOnDataAccess)(this, daGet);
      }
    };
      ~DataFieldEnum();

  void Clear();
  void Inc(void);
  void Dec(void);
  int CreateComboList(void);

  void addEnumText(const TCHAR *Text);
  void addEnumTextNoLF(const TCHAR *Text);

  int GetAsInteger(void);
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void){
    return(GetAsString());
  };

  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(int Value);
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  int SetAsInteger(int Value);
  void Sort(int startindex=0);
};

#define DFE_MAX_FILES 300

typedef struct {
  TCHAR *mTextFile;
  TCHAR *mTextPathFile;
} DataFieldFileReaderEntry;

class DataFieldFileReader: public DataField {

 private:
  unsigned int nFiles;
  unsigned int mValue;
  DataFieldFileReaderEntry fields[DFE_MAX_FILES];

  public:
  DataFieldFileReader(const TCHAR *EditFormat, 
		      const TCHAR *DisplayFormat, 
		      void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      mValue = 0;
      fields[0].mTextFile= NULL;
      fields[0].mTextPathFile= NULL; // first entry always exists and is blank
      nFiles = 1;

      SupportCombo=true;
      (mOnDataAccess)(this, daGet);
      
    };
    ~DataFieldFileReader() {
		Clear();
	}
	
	void Clear();
    
  void Inc(void);
  void Dec(void);
  int CreateComboList(void);

  void addFile(const TCHAR *fname, const TCHAR *fpname);
  bool checkFilter(const TCHAR *fname, const TCHAR* filter);
  int GetNumFiles(void);

  int GetAsInteger(void);
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void);
  bool Lookup(const TCHAR* text);
  const TCHAR* GetPathFile(void) const;

  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(int Value);
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  int SetAsInteger(int Value);

  void Sort();
  void ScanDirectoryTop(const TCHAR *subdir, const TCHAR *filter);

 protected:
  BOOL ScanDirectories(const TCHAR *pattern, const TCHAR *filter);

};



#define OUTBUFFERSIZE 128

class DataFieldInteger:public DataField{

  private:
    int mValue;
    int mMin;
    int mMax;
    int mStep;
    Poco::Timestamp mTmLastStep;
    int mSpeedup;
    TCHAR mOutBuf[OUTBUFFERSIZE+1];

  protected:
    int SpeedUp(bool keyup);
  public:
    DataFieldInteger(TCHAR *EditFormat, TCHAR *DisplayFormat, int Min, int Max, int Default, int Step, void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      mMin = Min;
      mMax = Max;
      mValue = Default;
      mStep = Step;

      SupportCombo=true;
     (mOnDataAccess)(this, daGet);

    };

  void Inc(void);
  void Dec(void);
  int CreateComboList(void);

  bool GetAsBoolean(void);
  int GetAsInteger(void);
  double GetAsFloat(void);
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void);

  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(int Value);
  int SetMin(int Value){mMin=Value; return(mMin);};
  int SetMax(int Value){mMax=Value; return(mMax);};
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  bool SetAsBoolean(bool Value);
  int SetAsInteger(int Value);
  double SetAsFloat(double Value);
  TCHAR *SetAsString(const TCHAR *Value);

};

class DataFieldFloat:public DataField{

  private:
    double mValue;
    double mMin;
    double mMax;
    double mStep;
    Poco::Timestamp mTmLastStep;
    int mSpeedup;
    int mFine;
    TCHAR mOutBuf[OUTBUFFERSIZE+1];


  protected:
    double SpeedUp(bool keyup);


  public:
    DataFieldFloat(TCHAR *EditFormat, TCHAR *DisplayFormat, double Min, double Max, double Default, double Step, int Fine, void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      mMin = Min;
      mMax = Max;
      mValue = Default;
      mStep = Step;
      mFine = Fine;

      SupportCombo=true;
     (mOnDataAccess)(this, daGet);

    };

  void Inc(void);
  void Dec(void);
  int CreateComboList(void);
  bool CreateKeyboard(void);
  int SetFromCombo(int iDataFieldIndex, TCHAR *sValue);

  bool GetAsBoolean(void);
  int GetAsInteger(void);
  double GetAsFloat(void);
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void);

  virtual void Set(int Value){ Set((double)Value); };

  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(double Value);
  double SetMin(double Value);
  double SetMax(double Value);
  double SetStep(double Value);
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  bool SetAsBoolean(bool Value);
  int SetAsInteger(int Value);
  double SetAsFloat(double Value);
  TCHAR *SetAsString(const TCHAR *Value);

};

#define EDITSTRINGSIZE 32

class DataFieldString:public DataField{

  private:
    TCHAR mValue[EDITSTRINGSIZE];

  public:
    DataFieldString(const TCHAR *EditFormat, const TCHAR *DisplayFormat, const TCHAR *Default, void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      _tcscpy(mValue, Default);
      SupportCombo=false;
    };

  TCHAR *SetAsString(const TCHAR *Value);
  #if defined(__BORLANDC__)
  #pragma warn -hid
  #endif
  void Set(const TCHAR *Value);
  #if defined(__BORLANDC__)
  #pragma warn +hid
  #endif
  TCHAR *GetAsString(void);
  TCHAR *GetAsDisplayString(void);

};

typedef enum{
  bkNone,
  bkTop,
  bkRight,
  bkBottom,
  bkLeft
}BorderKind_t;

class WindowControl : public WndCtrlBase {
    friend class WndForm;
    friend class WndProperty;
 public:
    typedef void (*OnHelpCallback_t)(WindowControl * Sender);

  private:

    int mX;
    int mY;
    int mWidth;
    int mHeight;

    WindowControl *mOwner;
    WindowControl *mTopOwner;
    LKBitmap mBmpMem;
    int  mBorderKind;
    LKColor mColorBack;
    LKColor mColorFore;
    BrushReference mhBrushBk;
    LKBrush mBrushBk;
    PenReference mhPenBorder;
    PenReference mhPenSelector;
    FontReference mhFont;
    TCHAR *mHelpText;

    OnHelpCallback_t mOnHelpCallback;

    int mTag;
    bool mReadOnly;

    int  mBorderSize;

    static int InstCount;
    static BrushReference hBrushDefaultBk;
    static PenReference hPenDefaultBorder;
    static PenReference hPenDefaultSelector;

  protected:

    bool mCanFocus;
    bool mDontPaintSelector;

    std::list<WindowControl*> mClients;

    virtual void PaintBorder(LKSurface& Surface);
    virtual void PaintSelector(LKSurface& Surface);

    void UpdatePosSize(void);

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect);

  public:
    const TCHAR* GetCaption(void) const { return GetWndText(); }

    virtual void CalcChildRect(int& x, int& y, int& cx, int& cy) const;
    
	// only Call by final contructor or overwrite
    virtual void AddClient(WindowControl *Client);

    virtual void Paint(LKSurface& Surface);

    virtual int OnHelp();

    virtual void Close(void){
      SetVisible(false);
    };
    virtual void Show(void){
      SetVisible(true);
    };

    void SetOnHelpCallback(OnHelpCallback_t Function){
      mOnHelpCallback = Function;
    }

    int GetWidth(void) const {return(mWidth);}
    int GetHeight(void) const {return(mHeight);}

    WindowControl *GetCanFocus(void);
    bool SetCanFocus(bool Value);

    bool GetReadOnly(void){return(mReadOnly);};
    bool SetReadOnly(bool Value);

    int  GetBorderKind(void);
    int  SetBorderKind(int Value);

    FontReference GetFont(void) const {return(mhFont);};
    virtual void SetFont(FontReference Value);

    virtual LKColor SetForeColor(const LKColor& Value);
    const LKColor& GetForeColor(void) const {return(mColorFore);};

    virtual LKColor SetBackColor(const LKColor& Value);
    const LKColor& GetBackColor(void) const {return(mColorBack);};

    BrushReference GetBackBrush() const {return(mhBrushBk);};

    PenReference GetBorderPen() const {return(mhPenBorder);};
    void SetBorderPen(const LKPen& Pen) { mhPenBorder = Pen;}

    PenReference GetSelectorPen(void) const {return(mhPenSelector);};

    virtual void SetCaption(const TCHAR *Value);
    void SetHelpText(const TCHAR *Value);
	const TCHAR* GetHelpText() const { return mHelpText; }

    virtual WindowControl* GetClientArea(void) { return (this); }

    WindowControl *GetParent(void) const {return(mOwner);};
    virtual WindowControl *GetTopOwner(void) const {return(mTopOwner);}

    int GetTag(void){return(mTag);};
    int SetTag(int Value){mTag = Value; return(mTag);};

    void SetTop(int Value);
    void SetLeft(int Value);
    void SetWidth(int Value);
    void SetHeight(int Value);

    int GetTop(void){return(mY);};
    int GetLeft(void){return(mX);};

    WindowControl *FocusNext(WindowControl *Sender);
    WindowControl *FocusPrev(WindowControl *Sender);

    WindowControl(WindowControl *Owner, const TCHAR *Name, int X, int Y, int Width, int Height, bool Visible=true);
    virtual ~WindowControl(void);

    virtual void Destroy(void);

    void PaintSelector(bool Value){mDontPaintSelector = Value;};

    WindowControl *FindByName(const TCHAR *Name);

    void FilterAdvanced(bool advanced);

protected:

    virtual void OnSetFocus();
    virtual void OnKillFocus();

    virtual void OnDestroy() {
        Close();
        WndCtrlBase::OnDestroy();
    }
    virtual bool OnClose() {
        Close();
        return true;
    }

};

class WndFrame:public WindowControl{

  public:

    WndFrame(WindowControl *Owner, const TCHAR *Name, 
             int X, int Y, int Width, int Height):
      WindowControl(Owner, Name, X, Y, Width, Height)
    {

      mLastDrawTextHeight = 0;
      mIsListItem = false;

      SetForeColor(GetParent()->GetForeColor());
      SetBackColor(GetParent()->GetBackColor());
      mCaptionStyle = DT_EXPANDTABS
      | DT_LEFT
      | DT_NOCLIP
      | DT_WORDBREAK;
    };

    UINT GetCaptionStyle(void){return(mCaptionStyle);};
    UINT SetCaptionStyle(UINT Value);

    int GetLastDrawTextHeight(void){return(mLastDrawTextHeight);};

    void SetIsListItem(bool Value){mIsListItem = Value;};


    virtual bool OnLButtonDown(const POINT& Pos);
    virtual bool OnLButtonUp(const POINT& Pos);

    virtual void Paint(LKSurface& Surface);

  protected:

    virtual bool OnKeyDown(unsigned KeyCode);

    bool mIsListItem;

    int mLastDrawTextHeight;
    UINT mCaptionStyle;


};

class WndListFrame:public WndFrame{

  public:

    typedef struct{
      int TopIndex;
      int BottomIndex;
      int ItemIndex;
      int DrawIndex;
//      int SelectedIndex;
      int ScrollIndex;
      int ItemCount;
      int ItemInViewCount;
      int ItemInPageCount;
    }ListInfo_t;

    typedef void (*OnListCallback_t)(WindowControl * Sender, ListInfo_t *ListInfo);

    WndListFrame(WindowControl *Owner, TCHAR *Name, int X, int Y, 
                 int Width, int Height, 
                 void (*OnListCallback)(WindowControl * Sender, 
                                        ListInfo_t *ListInfo));

    virtual bool OnMouseMove(const POINT& Pos);
    bool OnItemKeyDown(WindowControl *Sender, unsigned KeyCode);
    int PrepareItemDraw(void);
    void ResetList(void);

    void SetEnterCallback(OnListCallback_t OnListCallback) {
        mOnListEnterCallback = OnListCallback;
    }


    void RedrawScrolled(bool all);
    void DrawScrollBar(LKSurface& Surface);
    bool RecalculateIndices(bool bigscroll);
    void Redraw(void);
    int GetItemIndex(void){return(mListInfo.ItemIndex);}
    void SetItemIndexPos(int iValue);
    void SetItemIndex(int iValue);
    void SelectItemFromScreen(int xPos, int yPos, RECT *rect);
    int GetScrollBarHeight (void);
    int GetScrollIndexFromScrollBarTop(int iScrollBarTop);
    int GetScrollBarTopFromScrollIndex();
#if 100124
#define SCROLLBARWIDTH_INITIAL 32
    int ScrollbarTop;
    int ScrollbarWidth;

    virtual void Paint(LKSurface& Surface);

  protected:
#else
  protected:
#define SCROLLBARWIDTH_INITIAL 32
    int ScrollbarTop;
    int ScrollbarWidth;
#endif

    virtual bool OnLButtonDown(const POINT& Pos);
    virtual bool OnLButtonUp(const POINT& Pos);

    OnListCallback_t mOnListCallback;
    OnListCallback_t mOnListEnterCallback;
    ListInfo_t mListInfo;

    RECT rcScrollBarButton;
    RECT rcScrollBar;
    int mMouseScrollBarYOffset; // where in the scrollbar button was mouse down at
    bool mMouseDown;

    Poco::Timestamp LastMouseMoveTime;
};

class WndOwnerDrawFrame:public WndFrame{

  public:

    typedef void (*OnPaintCallback_t)(WindowControl * Sender, LKSurface& Surface);

    WndOwnerDrawFrame(WindowControl *Owner, TCHAR *Name, int X, int Y, int Width, int Height, OnPaintCallback_t OnPaintCallback):
      WndFrame(Owner, Name, X, Y, Width, Height)
    {
      SetOnPaintNotify(OnPaintCallback);

      SetForeColor(GetParent()->GetForeColor());
      SetBackColor(GetParent()->GetBackColor());

    };

    void SetOnPaintNotify(OnPaintCallback_t OnPaintCallback){
      mOnPaintCallback = OnPaintCallback;
    }

  protected:

    OnPaintCallback_t mOnPaintCallback;
    virtual void Paint(LKSurface& Surface);

};

extern WindowControl *ActiveControl;
extern WindowControl *LastFocusControl;

#define mrOK             2
#define mrCancel         3

class WndForm:public WindowControl{

    typedef bool (*OnTimerNotify_t)();
    typedef bool (*OnKeyDownNotify_t)(Window* pWnd, unsigned KeyCode);
    typedef bool (*OnKeyUpNotify_t)(Window* pWnd, unsigned KeyCode);

  protected:

    int mModalResult;
    LKColor mColorTitle;
    BrushReference mhBrushTitle;
    FontReference mhTitleFont;
    WindowControl *mClientWindow;
    RECT mClientRect;
    RECT mTitleRect;

    OnTimerNotify_t mOnTimerNotify;
    OnKeyDownNotify_t mOnKeyDownNotify;
    OnKeyUpNotify_t mOnKeyUpNotify;

    virtual void Paint(LKSurface& Surface);

  public:

    WndForm(const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height);
    ~WndForm(void);
    virtual void Destroy(void);

    bool bLButtonDown; //RLD
    WindowControl* GetClientArea() { return (mClientWindow ?mClientWindow:WindowControl::GetClientArea()); }
	virtual WindowControl* GetTopOwner(void) {return(this);}

    void AddClient(WindowControl *Client);

    void Close(void){
      WindowControl::Close();
      mModalResult = mrCancel;
    }

    Poco::Timestamp enterTime;

    int GetModalResult(void){return(mModalResult);};
    int SetModalResult(int Value){mModalResult = Value;return(Value);};

    FontReference SetTitleFont(FontReference Value);

    int ShowModal(void);
    void Show();

    void SetCaption(const TCHAR *Value);

    LKColor SetForeColor(const LKColor& Value);
    LKColor SetBackColor(const LKColor& Value);
    void SetFont(FontReference Value);

    void SetKeyDownNotify(OnKeyDownNotify_t KeyDownNotify) {
        mOnKeyDownNotify = KeyDownNotify;
    }

    void SetKeyUpNotify(OnKeyUpNotify_t KeyUpNotify) {
        mOnKeyUpNotify = KeyUpNotify;
    }

    void SetTimerNotify(unsigned uTime, OnTimerNotify_t OnTimerNotify) {
        mOnTimerNotify = OnTimerNotify;
        if(OnTimerNotify && uTime > 0) {
            StartTimer(uTime);
        } else {
            StopTimer();
        }
    }

protected:
    virtual bool OnKeyDownNotify(Window* pWnd, unsigned KeyCode);

    virtual bool OnKeyUpNotify(Window* pWnd, unsigned KeyCode) {
        return (mOnKeyUpNotify && (mOnKeyUpNotify)(this, KeyCode));
    }

    virtual void OnTimer() {
        if(mOnTimerNotify) {
            mOnTimerNotify();
        }
    }
};

class WndButton:public WindowControl{
  public:
    typedef void (*ClickNotifyCallback_t)(WndButton* pWnd);

  private:

    virtual void Paint(LKSurface& Surface);
    bool mDown;
    bool mDefault;
    int mLastDrawTextHeight;
    ClickNotifyCallback_t mOnClickNotify;

  public:
  
    WndButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, ClickNotifyCallback_t Function = NULL);

    virtual bool OnLButtonDown(const POINT& Pos);
    virtual bool OnLButtonUp(const POINT& Pos);
    virtual bool OnLButtonDblClick(const POINT& Pos);

    virtual bool OnKeyDown(unsigned KeyCode);
    virtual bool OnKeyUp(unsigned KeyCode);

    void SetOnClickNotify(ClickNotifyCallback_t Function){
      mOnClickNotify = Function;
    }
};



#define STRINGVALUESIZE         128

class WndProperty:public WindowControl{
  public:
    typedef int (*DataChangeCallback_t)(WindowControl * Sender, int Mode, int Value);
    
  private:

    RECT mEditRect;

    FontReference mhCaptionFont;
    FontReference mhValueFont;
    int  mBitmapSize;
    int  mCaptionWidth;
    RECT mHitRectUp;
    RECT mHitRectDown;
    bool mDownDown;
    bool mUpDown;
    bool mUseKeyboard;

    virtual void Paint(LKSurface& Surface);

    DataChangeCallback_t mOnDataChangeNotify;

    int CallSpecial(void);
    int IncValue(void);
    int DecValue(void);

    DataField *mDataField;
    std::tstring mValue;

    void UpdateButtonData(int Value);
    bool mDialogStyle;

  public:

    WndProperty(WindowControl *Parent, TCHAR *Name, TCHAR *Caption, int X, int Y, int Width, int Height, int CaptionWidth, DataChangeCallback_t DataChangeNotify, int MultiLine=false);
    ~WndProperty(void);
    virtual void Destroy(void);

    bool SetReadOnly(bool Value);
    bool SetUseKeyboard(bool Value);

    void RefreshDisplay(void);

    void SetFont(FontReference Value);

    bool OnKeyDown(unsigned KeyCode);
    bool OnKeyUp(unsigned KeyCode);
    bool OnLButtonDown(const POINT& Pos);
    bool OnLButtonUp(const POINT& Pos);
    bool OnLButtonDblClick(const POINT& Pos);

    DataField *GetDataField(void){return(mDataField);};
    DataField *SetDataField(DataField *Value);
    void SetText(const TCHAR *Value);
    int SetButtonSize(int Value);

};

int dlgComboPicker(WndProperty* theProperty);

#endif

