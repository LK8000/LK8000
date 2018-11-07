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
#include "LKObjects.h"
#include <tchar.h>
#include <string.h>

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

    DataField(const TCHAR *EditFormat, const TCHAR *DisplayFormat, DataAccessCallback_t OnDataAccess=nullptr);
    virtual ~DataField(void){};

	virtual void Clear();

  virtual void Special(void);
  virtual void Inc(void);
  virtual void Dec(void);

  virtual void GetData(void);
  virtual void SetData(void);

  virtual bool GetAsBoolean(void) { assert(false); return(false); }
  virtual int GetAsInteger(void) { assert(false); return(0);}
  virtual double GetAsFloat(void) { assert(false); return(0);}
  virtual const TCHAR *GetAsString(void) { assert(false); return(NULL);}
  virtual const TCHAR *GetAsDisplayString(void) { return GetAsString(); }

  virtual bool SetAsBoolean(bool Value){ assert(false); return(false);}
  virtual int SetAsInteger(int Value){ assert(false); return(0);}
  virtual double SetAsFloat(double Value){ assert(false); return(0.0);}
  virtual const TCHAR *SetAsString(const TCHAR *Value){ assert(false); return(NULL);}

  virtual void Set(bool Value) { assert(false); }
  virtual void Set(int Value) { assert(false); }
  virtual void Set(double Value) { assert(false); }
  virtual void Set(const TCHAR *Value) { assert(false); }
  virtual void Set(unsigned Value) { assert(false); }

  virtual int SetMin(int Value) { assert(false); return(0);};
  virtual double SetMin(double Value) { assert(false); return(false);};
  virtual int SetStep(int Value){ assert(false); return(0);};
  virtual double SetStep(double Value){ assert(false); return(false);};
  virtual int SetMax(int Value){ assert(false); return(0);};
  virtual double SetMax(double Value){ assert(false); return(0);};
  void SetUnits(const TCHAR *text) { _tcscpy(mUnits, text); }
  const TCHAR* GetUnits() const { return mUnits; }

  virtual void addEnumText(const TCHAR *Text, const TCHAR *Label = nullptr ) { assert(false); }
  virtual void addEnumTextNoLF(const TCHAR *Text) { assert(false); }
  virtual void Sort(int startindex=0) { assert(false); }

  virtual int Find(const TCHAR *Text) { assert(false); return -1; }

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
    DataFieldBoolean(const TCHAR *EditFormat, const TCHAR *DisplayFormat, int Default, const TCHAR *TextTrue, const TCHAR *TextFalse, DataAccessCallback_t OnDataAccess):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
		  if (Default) {mValue=true;} else {mValue=false;}
      _tcscpy(mTextTrue, TextTrue);
      _tcscpy(mTextFalse, TextFalse);
      SupportCombo=true;

     (mOnDataAccess)(this, daGet);

    };

  void Inc(void) override;
  void Dec(void) override;
  int CreateComboList(void) override;

  bool GetAsBoolean(void) override;
  int GetAsInteger(void) override;
  double GetAsFloat(void) override;
  const TCHAR *GetAsString(void) override;

  void Set(int Value) override {
    if (Value>0)
      Set(true);
    else
      Set(false);
  };


  void Set(bool Value) override;

  bool SetAsBoolean(bool Value) override;
  int SetAsInteger(int Value) override;
  double SetAsFloat(double Value) override;
  const TCHAR *SetAsString(const TCHAR *Value) override;

};

typedef struct {
  unsigned int index;
  tstring mText;
  tstring mLabel;
} DataFieldEnumEntry ;

class DataFieldEnum: public DataField {

  private:
    unsigned int mValue;
    std::vector<DataFieldEnumEntry> mEntries;

  public:
    DataFieldEnum(const TCHAR *EditFormat,
		  const TCHAR *DisplayFormat,
		  int Default,
		  DataAccessCallback_t OnDataAccess = nullptr):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      SupportCombo=true;

      if (Default>=0)
	{ mValue = Default; }
      else
	{mValue = 0;}
      if (mOnDataAccess) {
	(mOnDataAccess)(this, daGet);
      }
    };
      ~DataFieldEnum();

  void Clear() override;
  void Inc(void) override;
  void Dec(void) override;
  int CreateComboList(void) override;


  void addEnumTextNoLF(const TCHAR *Text) override;
  void addEnumText(const TCHAR *Text, const TCHAR *Label) override ;

  int Find(const TCHAR *Text) override ;

  int GetAsInteger(void) override;
  const TCHAR *GetAsString(void) override;
  const TCHAR *GetAsDisplayString(void) override;
  bool GetAsBoolean() override;

  void Set(unsigned Value) override;
  void Set(int Value) override { Set((unsigned)Value); }
  void Set(bool Value) override { Set(Value?1:0); }

  int SetAsInteger(int Value) override;
  void Sort(int startindex=0) override;
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
  DataFieldFileReader(const TCHAR *EditFormat, const TCHAR *DisplayFormat, DataAccessCallback_t OnDataAccess=nullptr):
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

	void Clear() override;

  void Inc(void) override;
  void Dec(void) override;
  int CreateComboList(void) override;

  /**
   * 
   * @param fname : text to display in combo list
   * @param fpname : file Path relative ScanDirectoryTop @subdir
   */
  void addFile(const TCHAR *fname, const TCHAR *fpname);
  
  
  static bool checkFilter(const TCHAR *fname, const TCHAR* filter);
  int GetNumFiles(void);

  int GetAsInteger(void) override;
  const TCHAR *GetAsString(void) override;

  int GetLabelIndex(const TCHAR* label);

  /**
   * find and select existing item.
   * 
   * @param text : file Path relative ScanDirectoryTop @subdir
   * @return : true if item exists
   */
  bool Lookup(const TCHAR* text);
  
  /**
   * @return file Path relative ScanDirectoryTop @subdir
   */
  const TCHAR* GetPathFile(void) const;

  void Set(int Value) override;

  int SetAsInteger(int Value) override;

  void Sort(int startindex=0) override;

  gcc_nonnull_all
  void ScanDirectoryTop(const TCHAR *subdir, const TCHAR *filter);

  gcc_nonnull_all
  void ScanSystemDirectoryTop(const TCHAR *subdir, const TCHAR *filter);

#ifdef ANDROID
  gcc_nonnull_all
  void ScanZipDirectory(const TCHAR *subdir, const TCHAR *filter);
#endif
 protected:

  gcc_nonnull_all
  BOOL ScanDirectories(const TCHAR *sPath, const TCHAR* subdir, const TCHAR *filter);

};



#define OUTBUFFERSIZE 128

class DataFieldInteger:public DataField{

  private:
    int mValue;
    int mMin;
    int mMax;
    int mStep;
    PeriodClock mTmLastStep;
    int mSpeedup;
    TCHAR mOutBuf[OUTBUFFERSIZE+1];

  protected:
    int SpeedUp(bool keyup);
  public:
    DataFieldInteger(TCHAR *EditFormat, TCHAR *DisplayFormat, int Min, int Max, int Default, int Step, DataAccessCallback_t OnDataAccess=nullptr):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      mMin = Min;
      mMax = Max;
      mValue = Default;
      mStep = Step;

      SupportCombo=true;
     (mOnDataAccess)(this, daGet);

    };

  void Inc(void) override;
  void Dec(void) override;
  int CreateComboList(void) override;
  bool CreateKeyboard(void) override;

  bool GetAsBoolean(void) override;
  int GetAsInteger(void) override;
  double GetAsFloat(void) override;
  const TCHAR *GetAsString(void) override;
  const TCHAR *GetAsDisplayString(void) override;

  void Set(int Value) override;
  int SetMin(int Value) override {mMin=Value; return(mMin);}
  int SetMax(int Value) override {mMax=Value; return(mMax);}

  bool SetAsBoolean(bool Value) override;
  int SetAsInteger(int Value) override;
  double SetAsFloat(double Value) override;
  const TCHAR *SetAsString(const TCHAR *Value) override;

};

class DataFieldFloat:public DataField{

  private:
    double mValue;
    double mMin;
    double mMax;
    double mStep;
    PeriodClock mTmLastStep;
    int mSpeedup;
    int mFine;
    TCHAR mOutBuf[OUTBUFFERSIZE+1];


  protected:
    double SpeedUp(bool keyup);


  public:
    DataFieldFloat(TCHAR *EditFormat, TCHAR *DisplayFormat, double Min, double Max, double Default, double Step, int Fine, DataAccessCallback_t OnDataAccess=nullptr):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      mMin = Min;
      mMax = Max;
      mValue = Default;
      mStep = Step;
      mFine = Fine;

      SupportCombo=true;
     (mOnDataAccess)(this, daGet);

    };

  void Inc(void) override;
  void Dec(void) override;
  int CreateComboList(void) override;
  bool CreateKeyboard(void) override;
  int SetFromCombo(int iDataFieldIndex, TCHAR *sValue) override;

  bool GetAsBoolean(void) override;
  int GetAsInteger(void) override;
  double GetAsFloat(void) override;
  const TCHAR *GetAsString(void) override;
  const TCHAR *GetAsDisplayString(void) override;

  virtual void Set(int Value) override { Set((double)Value); };


  void Set(double Value) override;
  double SetMin(double Value) override;
  double SetMax(double Value) override;
  double SetStep(double Value) override;

  bool SetAsBoolean(bool Value) override;
  int SetAsInteger(int Value) override;
  double SetAsFloat(double Value) override;
  const TCHAR *SetAsString(const TCHAR *Value) override;

};

#define EDITSTRINGSIZE 32

class DataFieldString:public DataField{

  private:
    TCHAR mValue[EDITSTRINGSIZE];

  public:
    DataFieldString(const TCHAR *EditFormat, const TCHAR *DisplayFormat, const TCHAR *Default, DataAccessCallback_t OnDataAccess=nullptr):
      DataField(EditFormat, DisplayFormat, OnDataAccess){
      _tcscpy(mValue, Default);
      SupportCombo=false;
    };

  const TCHAR *SetAsString(const TCHAR *Value) override;
  void Set(const TCHAR *Value) override;

  const TCHAR *GetAsString(void) override;

  bool CreateKeyboard() override;

};

typedef enum{
  bkNone,
  bkTop,
  bkRight,
  bkBottom,
  bkLeft
}BorderKind_t;

class WndForm;

class WindowControl : public WndCtrlBase {
    friend class WndForm;
    friend class WndProperty;
 public:
    typedef void (*OnHelpCallback_t)(WindowControl * Sender);

  private:

    WindowControl *mOwner;
    WndForm *mParentWndForm;

    int  mBorderKind;

    LKColor mColorBack;
    LKColor mColorFore;
    LKBrush mBrushBk;
    LKBrush mBrushBorder;

    TCHAR *mHelpText;

    OnHelpCallback_t mOnHelpCallback;

    int mTag;
    bool mReadOnly;

    static int InstCount;

  protected:

    bool mCanFocus;
    bool mDontPaintSelector;

    std::list<WindowControl*> mClients;

    virtual void PaintBorder(LKSurface& Surface);
    virtual void PaintSelector(LKSurface& Surface);

    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) override;

  public:
    const TCHAR* GetCaption(void) const { return GetWndText(); }

    virtual void CalcChildRect(int& x, int& y, int& cx, int& cy) const;

	// only Call by final contructor or overwrite
    virtual void AddClient(WindowControl *Client);

    virtual void Paint(LKSurface& Surface);

    virtual int OnHelp();

    void SetOnHelpCallback(OnHelpCallback_t Function){
      mOnHelpCallback = Function;
    }

    WindowControl *GetCanFocus(void);
    bool SetCanFocus(bool Value);

    bool GetReadOnly(void){return(mReadOnly);};
    virtual bool SetReadOnly(bool Value);

    int  GetBorderKind(void);
    virtual int  SetBorderKind(int Value);

    virtual LKColor SetForeColor(const LKColor& Value);
    const LKColor& GetForeColor(void) const {return(mColorFore);};

    virtual LKColor SetBackColor(const LKColor& Value);
    const LKColor& GetBackColor(void) const {return(mColorBack);};

    BrushReference GetBackBrush() const {
        if( mBrushBk) {
            return mBrushBk;
        } else if (GetParent()) {
            return GetParent()-> GetBackBrush();
        } else {
            return LKBrush_FormBackGround;
        }
    }

    void SetBorderColor(const LKColor& Color) { mBrushBorder.Create(Color);}

    virtual void SetCaption(const TCHAR *Value);
    void SetHelpText(const TCHAR *Value);
	bool HasHelpText() const { return (mHelpText||mOnHelpCallback); }

    virtual WindowControl* GetClientArea(void) { return (this); }

    virtual WindowControl *GetParent(void) const {return(mOwner);};
    virtual WndForm *GetParentWndForm(void) {return(mParentWndForm);}

    int GetTag(void){return(mTag);};
    int SetTag(int Value){mTag = Value; return(mTag);};

    void SetTop(int Value);
    void SetLeft(int Value);
    void SetWidth(unsigned int Value);
    void SetHeight(unsigned int Value);

    WindowControl *FocusNext(WindowControl *Sender);
    WindowControl *FocusPrev(WindowControl *Sender);

    WindowControl(WindowControl *Owner, const TCHAR *Name, int X, int Y, int Width, int Height, bool Visible=true);
    ~WindowControl(void);

    void Destroy() override;

    void PaintSelector(bool Value){mDontPaintSelector = Value;};

    WindowControl *FindByName(const TCHAR *Name);

    void FilterAdvanced(bool advanced);

protected:

    virtual void OnSetFocus() override;
    virtual void OnKillFocus() override;

    bool OnClose() override{
        SetVisible(false);
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
      mLButtonDown = false;
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

    virtual void Paint(LKSurface& Surface);

  protected:

    virtual bool OnKeyDown(unsigned KeyCode);

    bool mIsListItem;

    int mLastDrawTextHeight;
    UINT mCaptionStyle;
    bool mLButtonDown;


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
    bool RecalculateIndices(bool bigscroll);
    void Redraw(void);
    int GetItemIndex(void) { 
      return (mListInfo.ScrollIndex + mListInfo.ItemIndex);
    }
    void SetItemIndexPos(int iValue);
    void SetItemIndex(int iValue);
    void SelectItemFromScreen(int xPos, int yPos, RECT *rect, bool select);

    void CalcChildRect(int& x, int& y, int& cx, int& cy) const;

protected:
    int GetScrollBarWidth();
    int GetScrollBarHeight (void);

    int GetScrollBarTop() { return GetScrollBarWidth(); }

    int GetScrollIndexFromScrollBarTop(int iScrollBarTop);
    int GetScrollBarTopFromScrollIndex();


    void DrawScrollBar(LKSurface& Surface);

    virtual void Paint(LKSurface& Surface);

    virtual bool OnLButtonDownNotify(Window* pWnd, const POINT& Pos) { return OnLButtonDown(Pos); }
    virtual bool OnLButtonDown(const POINT& Pos);
    virtual bool OnLButtonUp(const POINT& Pos);

private:
    constexpr static int ScrollbarWidthInitial = 32;
    int ScrollbarWidth;


    OnListCallback_t mOnListCallback;
    OnListCallback_t mOnListEnterCallback;
    ListInfo_t mListInfo;

    RECT rcScrollBarButton;
    RECT rcScrollBar;
    POINT mScrollStart;
    int mMouseScrollBarYOffset; // where in the scrollbar button was mouse down at
    bool mMouseDown;
    bool mCaptureScrollButton; // scrolling using scrollbar in progress
    bool mCaptureScroll; // "Smartphone like" scrolling in progress

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

    typedef bool (*OnTimerNotify_t)(WndForm* pWnd);
    typedef bool (*OnKeyDownNotify_t)(WndForm* pWnd, unsigned KeyCode);
    typedef bool (*OnKeyUpNotify_t)(WndForm* pWnd, unsigned KeyCode);
    typedef bool (*OnUser_t)(WndForm* pWndForm, unsigned id);

  protected:

    int mModalResult;
    LKColor mColorTitle;
    BrushReference mhBrushTitle;
    FontReference mhTitleFont;
    WindowControl *mClientWindow;
    RECT mClientRect;
    RECT mTitleRect;

    bool mModal;

    OnTimerNotify_t mOnTimerNotify;
    OnKeyDownNotify_t mOnKeyDownNotify;
    OnKeyUpNotify_t mOnKeyUpNotify;
    OnUser_t mOnUser;

    virtual void Paint(LKSurface& Surface) override;

  public:

    WndForm(const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, bool Modal = true);
    ~WndForm(void);
    void Destroy(void) override;

    virtual WindowControl* GetClientArea() override { return (mClientWindow ?mClientWindow:WindowControl::GetClientArea()); }
	  virtual WndForm* GetParentWndForm(void) override { return (this);}

    void AddClient(WindowControl *Client) override;

    bool OnClose(void) override {
      mModalResult = mrCancel;
      WindowControl::OnClose();
      return true;
    }

    PeriodClock enterTime;

    int GetModalResult(void){return(mModalResult);};
    int SetModalResult(int Value){mModalResult = Value;return(Value);};

    FontReference SetTitleFont(FontReference Value);

    int ShowModal(void);
    void Show() override;

    void SetCaption(const TCHAR *Value) override;

    int  SetBorderKind(int Value) override;

    LKColor SetForeColor(const LKColor& Value) override;
    LKColor SetBackColor(const LKColor& Value) override;
    void SetFont(FontReference Value) override;

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

    void SetOnUser(OnUser_t OnUser) {
        mOnUser = OnUser;
    }

  void ReinitialiseLayout(const RECT& Rect) { }

protected:
    bool OnKeyDownNotify(Window* pWnd, unsigned KeyCode);

    bool OnKeyUpNotify(Window* pWnd, unsigned KeyCode) {
        return (mOnKeyUpNotify && (mOnKeyUpNotify)(this, KeyCode));
    }

    void OnTimer() override {
        if(mOnTimerNotify) {
            mOnTimerNotify(this);
        }
    }

    bool OnUser(unsigned id) override {
      return (mOnUser && mOnUser(this, id));
    }

    virtual void OnDestroy() override {
        mModalResult = mrCancel;
        WndCtrlBase::OnDestroy();
    }
};

#define LEDMODE_DISABLED    0
#define LEDMODE_REDGREEN    1
#define LEDMODE_OFFGREEN    2
#define LEDMODE_MANUAL      3

#define LEDCOLOR_BLACK    0
#define LEDCOLOR_RED      1
#define LEDCOLOR_GREEN    2
#define LEDCOLOR_BLUE     3
#define LEDCOLOR_YELLOW   4
#define LEDCOLOR_ORANGE   5
#define LEDCOLOR_LGREEN   6
#define LEDCOLOR_DGREEN   7
#define LEDCOLOR_CYAN     8
#define MAXLEDCOLORS      9
//
// blending from rgb1 to rgb2 colors, bottom->up
//
typedef struct _LEDCOLORRAMP
{
     unsigned char r1;
     unsigned char g1;
     unsigned char b1;
     unsigned char r2;
     unsigned char g2;
     unsigned char b2;
     unsigned short l;
} LEDCOLORRAMP;


class WndButton:public WindowControl{
  public:
    typedef void (*ClickNotifyCallback_t)(WndButton* pWnd);

  private:

    virtual void Paint(LKSurface& Surface) override;
    bool mDown;
    bool mDefault;
    unsigned short mLedMode;   // 0=no led  1=OnOff mode  2=manual (choose colors)
    bool mLedOnOff;            // if mLedUse=1  this is used to toggle false=orange/true=green
    unsigned short mLedSize;   // override default size
    unsigned short mLedColor;  // in manual mode, use LEDCOLOR_xxx, default is Black. 

    int mLastDrawTextHeight;
    ClickNotifyCallback_t mOnClickNotify;

  public:

    WndButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, ClickNotifyCallback_t Function = NULL);

    virtual bool OnLButtonDown(const POINT& Pos) override;
    virtual bool OnLButtonUp(const POINT& Pos) override;
    virtual bool OnLButtonDblClick(const POINT& Pos) override;

    virtual bool OnKeyDown(unsigned KeyCode) override;
    virtual bool OnKeyUp(unsigned KeyCode) override;

    virtual void LedSetMode(unsigned short leduse);
    virtual void LedSetSize(unsigned short ledsize);
    virtual void LedSetColor(unsigned short ledcolor);
    virtual void LedSetOnOff(bool ledonoff);

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

    FontReference mhValueFont;
    int  mBitmapSize;
    int  mCaptionWidth;
    RECT mHitRectUp;
    RECT mHitRectDown;
    bool mDownDown;
    bool mUpDown;
    bool mUseKeyboard;
    bool mMultiLine;

    virtual void Paint(LKSurface& Surface) override;

    DataChangeCallback_t mOnDataChangeNotify;

    int CallSpecial(void);
    int IncValue(void);
    int DecValue(void);

    DataField *mDataField;
    tstring mValue;

    void UpdateButtonData();
    bool mDialogStyle;

  public:

    WndProperty(WindowControl *Parent, TCHAR *Name, TCHAR *Caption, int X, int Y, int Width, int Height, int CaptionWidth, DataChangeCallback_t DataChangeNotify, int MultiLine=false);
    ~WndProperty(void);
    virtual void Destroy(void) override;

    bool SetReadOnly(bool Value) override;
    bool SetUseKeyboard(bool Value);

    void RefreshDisplay(void);

    void SetFont(FontReference Value) override;

    bool OnKeyDown(unsigned KeyCode) override;
    bool OnKeyUp(unsigned KeyCode) override;
    bool OnLButtonDown(const POINT& Pos) override;
    bool OnLButtonUp(const POINT& Pos) override;
    bool OnLButtonDblClick(const POINT& Pos) override;

    DataField *GetDataField(void){return(mDataField);};
    DataField *SetDataField(DataField *Value);
    void SetText(const TCHAR *Value);
    int SetButtonSize(int Value);

};

int dlgComboPicker(WndProperty* theProperty);

#endif
