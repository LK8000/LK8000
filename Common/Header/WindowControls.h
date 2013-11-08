/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindowControls.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__WINDOWSCONTROL_H)
#define __WINDOWSCONTROL_H

#define IsEmptyString(x)        ((x==NULL) || (x[0]=='\0'))

#define MAXSETCAPTION 254	// max chars in SetCaption, autolimited

#define BORDERTOP    (1<<bkTop)
#define BORDERRIGHT  (1<<bkRight)
#define BORDERBOTTOM (1<<bkBottom)
#define BORDERLEFT   (1<<bkLeft)

#define clBlack   RGB_BLACK;
#define clMaroon  RGB(0x00,0x00,0x80)
#define clGreen   RGB(0x00,0x80,0x00)
#define clOlive   RGB(0x00,0x80,0x80)
#define clNavy    RGB(0x80,0x00,0x00)
#define clPurple  RGB(0x80,0x00,0x80)
#define clTeal    RGB(0x80,0x80,0x00)
#define clGray    RGB(0x80,0x80,0x80)
#define clSilver  RGB(0xC0,0xC0,0xC0)
#define clRed     RGB(0xFF,0x00,0xFF)
#define clLime    RGB(0x00,0xFF,0x00)
#define clYellow  RGB(0x00,0xFF,0xFF)
#define clBlue    RGB(0xFF,0x00,0x00)
#define clFuchsia RGB(0xFF,0x00,0xFF)
#define clAqua    RGB(0xFF,0xFF,0x00)
#define clLtGray  RGB(0xC0,0xC0,0xC0)
#define clDkGray  RGB(0x80,0x80,0x80)
#define clWhite   RGB(0xFF,0xFF,0xFF)
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
  virtual int CreateKeyboard(void) {return FALSE;};

  ComboList* GetCombo(void) { return &mComboList;}
  virtual int SetFromCombo(int iDataFieldIndex, TCHAR *sValue) {return SetAsInteger(iDataFieldIndex);};
  void CopyString(TCHAR * szStringOut, bool bFormatted);
  bool SupportCombo;  // all Types dataField support combolist except DataFieldString.
  protected:
    void (*mOnDataAccess)(DataField *Sender, DataAccessKind_t Mode);
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
  BOOL ScanFiles(const TCHAR *pattern, const TCHAR *filter);
  BOOL ScanDirectories(const TCHAR *pattern, const TCHAR *filter);

};



#define OUTBUFFERSIZE 128

class DataFieldInteger:public DataField{

  private:
    int mValue;
    int mMin;
    int mMax;
    int mStep;
    DWORD mTmLastStep;
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
    DWORD mTmLastStep;
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
  int CreateKeyboard(void);
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

class WindowControl {
 public:
    typedef void (*OnHelpCallback_t)(WindowControl * Sender);

  private:

    int mX;
    int mY;
    int mWidth;
    int mHeight;

    HWND mParent;
    WindowControl *mOwner;
    WindowControl *mTopOwner;
    HDC  mHdc;
    HBITMAP mBmpMem;
    int  mBorderKind;
    COLORREF mColorBack;
    COLORREF mColorFore;
    HBRUSH mhBrushBk;
    HPEN mhPenBorder;
    HPEN mhPenSelector;
    RECT mBoundRect;
    HFONT mhFont;
    TCHAR mName[64];
    TCHAR *mHelpText;

    OnHelpCallback_t mOnHelpCallback;

    int mTag;
    bool mReadOnly;
    bool mHasFocus;

    int  mBorderSize;
    bool mVisible;

    WindowControl *mActiveClient;

    LONG mSavWndProcedure;

    static int InstCount;
    static HBRUSH hBrushDefaultBk;
    static HPEN hPenDefaultBorder;
    static HPEN hPenDefaultSelector;

  protected:

    HWND mHWnd;
    bool mCanFocus;
    TCHAR mCaption[MAXSETCAPTION+1]; // +1 just for safety!
    bool mDontPaintSelector;

    WindowControl *mClients[50];
    int mClientCount;

    virtual void PaintSelector(HDC hDC);
    virtual WindowControl *SetOwner(WindowControl *Value);
    void UpdatePosSize(void);
    bool HasFocus(void) { return mHasFocus; };

  public:
    TCHAR* GetCaption(void) { return mCaption; };
    int WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void AddClient(WindowControl *Client);

    virtual void Paint(HDC hDC);

    virtual int OnHelp();

    virtual int OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnLButtonDown(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnLButtonUp(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnKeyDown(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnKeyUp(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnCommand(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnMouseMove(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };
    virtual int OnUnhandledMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
		(void)hwnd; (void)uMsg; (void)wParam; (void)lParam;
      return(1);
    };
    virtual void Close(void){
      SetVisible(false);
    };
    virtual void Show(void){
      SetVisible(true);
    };

    void SetOnHelpCallback(void(*Function)(WindowControl * Sender)){
      mOnHelpCallback = Function;
    }

    RECT *GetBoundRect(void){return(&mBoundRect);};

    int GetWidth(void){return(mWidth);};
    int GetHeight(void){return(mHeight);};

    virtual bool SetFocused(bool Value, HWND FromTo);
    bool GetFocused(void);
    WindowControl *GetCanFocus(void);
    bool SetCanFocus(bool Value);

    bool GetReadOnly(void){return(mReadOnly);};
    bool SetReadOnly(bool Value);

    bool SetVisible(bool Value);
    bool GetVisible(void);

    int  GetBorderKind(void);
    int  SetBorderKind(int Value);

    HFONT GetFont(void){return(mhFont);};
    virtual HFONT SetFont(HFONT Value);

    virtual COLORREF SetForeColor(COLORREF Value);
    COLORREF GetForeColor(void){return(mColorFore);};

    virtual COLORREF SetBackColor(COLORREF Value);
    COLORREF GetBackColor(void){return(mColorBack);};

    HBRUSH   GetBackBrush(void){return(mhBrushBk);};
    HPEN     GetBorderPen(void){return(mhPenBorder);};
    HPEN     GetSelectorPen(void){return(mhPenSelector);};

    virtual void SetCaption(const TCHAR *Value);
    void SetHelpText(const TCHAR *Value);
	const TCHAR* GetHelpText() const { return mHelpText; }

    HWND GetHandle(void){return(mHWnd);};
    virtual HWND GetClientAreaHandle(void){return(mHWnd);};
    HWND GetParent(void){return(mParent);};
    HDC  GetDeviceContext(void){return(mHdc);};

    WindowControl *GetOwner(void){return(mOwner);};

    void SetParentHandle(HWND hwnd);

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

    WindowControl(WindowControl *Owner, HWND Parent, const TCHAR *Name, int X, int Y, int Width, int Height, bool Visible=true);
    virtual ~WindowControl(void);

    virtual void Destroy(void);

    virtual void Redraw(void);

    void PaintSelector(bool Value){mDontPaintSelector = Value;};

    WindowControl *FindByName(const TCHAR *Name);

    void FilterAdvanced(bool advanced);

};

class WndFrame:public WindowControl{

  public:

    WndFrame(WindowControl *Owner, const TCHAR *Name, 
             int X, int Y, int Width, int Height):
      WindowControl(Owner, NULL, Name, X, Y, Width, Height)
    {

      mLastDrawTextHeight = 0;
      mIsListItem = false;

      SetForeColor(GetOwner()->GetForeColor());
      SetBackColor(GetOwner()->GetBackColor());
      mCaptionStyle = DT_EXPANDTABS
      | DT_LEFT
      | DT_NOCLIP
      | DT_WORDBREAK;
    };

    virtual void Destroy(void);

    virtual int OnMouseMove(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(1);
    };

    void SetCaption(const TCHAR *Value);
    TCHAR *GetCaption(void){return(mCaption);};

    UINT GetCaptionStyle(void){return(mCaptionStyle);};
    UINT SetCaptionStyle(UINT Value);

    int GetLastDrawTextHeight(void){return(mLastDrawTextHeight);};

    void SetIsListItem(bool Value){mIsListItem = Value;};


    int OnLButtonDown(WPARAM wParam, LPARAM lParam);
    int OnLButtonUp(WPARAM wParam, LPARAM lParam);

  protected:

    int OnKeyDown(WPARAM wParam, LPARAM lParam);

    bool mIsListItem;

    int mLastDrawTextHeight;
    UINT mCaptionStyle;

    virtual void Paint(HDC hDC);

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

    virtual void Destroy(void);
    
    int OnMouseMove(WPARAM wParam, LPARAM lParam);
    int OnItemKeyDown(WindowControl *Sender, WPARAM wParam, LPARAM lParam);
    int PrepareItemDraw(void);
    void ResetList(void);
    void SetEnterCallback(void (*OnListCallback)(WindowControl * Sender, ListInfo_t *ListInfo));
    void RedrawScrolled(bool all);
    void DrawScrollBar(HDC hDC);
    int RecalculateIndices(bool bigscroll);
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

  protected:
#else
  protected:
#define SCROLLBARWIDTH_INITIAL 32
    int ScrollbarTop;
    int ScrollbarWidth;
#endif

    int OnLButtonDown(WPARAM wParam, LPARAM lParam);
    int OnLButtonUp(WPARAM wParam, LPARAM lParam);

    OnListCallback_t mOnListCallback;
    OnListCallback_t mOnListEnterCallback;
    ListInfo_t mListInfo;
    virtual void Paint(HDC hDC);
	  RECT rcScrollBarButton;
	  RECT rcScrollBar;
    int mMouseScrollBarYOffset; // where in the scrollbar button was mouse down at
    bool mMouseDown;
#ifdef GTCFIX
    DWORD LastMouseMoveTime;
#else
    int LastMouseMoveTime;
#endif
};

class WndOwnerDrawFrame:public WndFrame{

  public:

    typedef void (*OnPaintCallback_t)(WindowControl * Sender, HDC hDC);

    WndOwnerDrawFrame(WindowControl *Owner, TCHAR *Name, int X, int Y, int Width, int Height, void(*OnPaintCallback)(WindowControl * Sender, HDC hDC)):
      WndFrame(Owner, Name, X, Y, Width, Height)
    {
      mCaption[0] = '\0';
      mOnPaintCallback = OnPaintCallback;
      SetForeColor(GetOwner()->GetForeColor());
      SetBackColor(GetOwner()->GetBackColor());

    };

    virtual void Destroy(void);

    void SetOnPaintNotify(void (*OnPaintCallback)(WindowControl * Sender, HDC hDC)){
      mOnPaintCallback = OnPaintCallback;
    }

  protected:

    OnPaintCallback_t mOnPaintCallback;
    virtual void Paint(HDC hDC);

};

extern WindowControl *ActiveControl;
extern WindowControl *LastFocusControl;

#define mrOK             2
#define mrCancle         3

class WndForm:public WindowControl{

  protected:

    static ACCEL  mAccel[];

    int mModalResult;
    HACCEL mhAccelTable;
    COLORREF mColorTitle;
    HBRUSH mhBrushTitle;
    HFONT mhTitleFont;
    WindowControl *mClientWindow;
    RECT mClientRect;
    RECT mTitleRect;

    int (*mOnTimerNotify)(WindowControl * Sender);
    int (*mOnKeyDownNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam);
    int (*mOnKeyUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam);
    int (*mOnLButtonUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam);
    int (*mOnUserMsgNotify)(WindowControl * Sender, MSG *msg);


    int OnUnhandledMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void Paint(HDC hDC);
    int cbTimerID;

  public:

    WndForm(HWND Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height);
    ~WndForm(void);
    virtual void Destroy(void);

    bool bLButtonDown; //RLD
    HWND GetClientAreaHandle(void);
    void AddClient(WindowControl *Client);

    virtual bool SetFocused(bool Value, HWND FromTo);


    int OnLButtonUp(WPARAM wParam, LPARAM lParam){
		(void)wParam; (void)lParam;
      return(0);
    };

    void Close(void){
      WindowControl::Close();
      mModalResult = mrCancle;
    }

    DWORD enterTime;

    void SetToForeground(void);

    int GetModalResult(void){return(mModalResult);};
    int SetModalResult(int Value){mModalResult = Value;return(Value);};

    HFONT SetTitleFont(HFONT Value);

    int ShowModal(bool bEnableMap);
    int ShowModal(void);
    void Show(void);

    void SetCaption(const TCHAR *Value);
    TCHAR *GetCaption(void){return(mCaption);};

    virtual int OnCommand(WPARAM wParam, LPARAM lParam);

    COLORREF SetForeColor(COLORREF Value);
    COLORREF SetBackColor(COLORREF Value);
    HFONT SetFont(HFONT Value);
    void SetKeyDownNotify(int (*KeyDownNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam));
    void SetKeyUpNotify(int (*KeyUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam));
    void SetLButtonUpNotify(int (*LButtonUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam));

    void SetTimerNotify(int (*OnTimerNotify)(WindowControl * Sender));

    void SetUserMsgNotify(int (*OnUserMsgNotify)(WindowControl * Sender, MSG *msg));
private:
    static DWORD timeAnyOpenClose; // when any dlg opens or child closes

};

class WndButton:public WindowControl{

  private:

    virtual void Paint(HDC hDC);
    bool mDown;
    bool mDefault;
    int mLastDrawTextHeight;
    void (*mOnClickNotify)(WindowControl * Sender);

  public:

    typedef void (*ClickNotifyCallback_t)(WindowControl * Sender);

    WndButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, void(*Function)(WindowControl * Sender) = NULL);
    virtual void Destroy(void);

    int OnLButtonUp(WPARAM wParam, LPARAM lParam);
    int OnLButtonDown(WPARAM wParam, LPARAM lParam);
    int OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam);

    int OnKeyDown(WPARAM wParam, LPARAM lParam);
    int OnKeyUp(WPARAM wParam, LPARAM lParam);

    void SetOnClickNotify(void(*Function)(WindowControl * Sender)){
      mOnClickNotify = Function;
    }


};



#define STRINGVALUESIZE         128

class WndProperty:public WindowControl{

  private:

    static int InstCount;

    HWND mhEdit;
    POINT mEditSize;
    POINT mEditPos;
    HFONT mhCaptionFont;
    HFONT mhValueFont;
    int  mBitmapSize;
    int  mCaptionWidth;
    RECT mHitRectUp;
    RECT mHitRectDown;
    bool mDownDown;
    bool mUpDown;
    bool mUseKeyboard;

    virtual void Paint(HDC hDC);
    void (*mOnClickUpNotify)(WindowControl * Sender);
    void (*mOnClickDownNotify)(WindowControl * Sender);

    int (*mOnDataChangeNotify)(WindowControl * Sender, int Mode, int Value);

    int CallSpecial(void);
    int IncValue(void);
    int DecValue(void);
    WNDPROC mEditWindowProcedure;

    DataField *mDataField;

    void UpdateButtonData(int Value);
    bool mDialogStyle;

  public:


    typedef int (*DataChangeCallback_t)(WindowControl * Sender, int Mode, int Value);

    WndProperty(WindowControl *Parent, TCHAR *Name, TCHAR *Caption, int X, int Y, int Width, int Height, int CaptionWidth, int (*DataChangeNotify)(WindowControl * Sender, int Mode, int Value), int MultiLine=false);
    ~WndProperty(void);
    virtual void Destroy(void);

    int WndProcEditControl(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    TCHAR *GetCaption(void){return(mCaption);};

    bool SetFocused(bool Value, HWND FromTo);

    bool SetReadOnly(bool Value);
    bool SetUseKeyboard(bool Value);

    void RefreshDisplay(void);

    HFONT SetFont(HFONT Value);

    int OnKeyDown(WPARAM wParam, LPARAM lParam);
    int OnEditKeyDown(WPARAM wParam, LPARAM lParam);
    int OnLButtonDown(WPARAM wParam, LPARAM lParam);
    int OnLButtonUp(WPARAM wParam, LPARAM lParam);
    int OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam);

//    int GetAsInteger(void){return(mValue);};
//    int SetAsInteger(int Value);

    DataField *GetDataField(void){return(mDataField);};
    DataField *SetDataField(DataField *Value);
    void SetText(const TCHAR *Value);
    int SetButtonSize(int Value);

};

int dlgComboPicker(WndProperty* theProperty);

typedef void (*webpt2Event)(const TCHAR *);

class WndEventButton:public WndButton {
 public:
  WndEventButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, 
		 int X, int Y, int Width, int Height, 
		 const TCHAR *ename,
		 const TCHAR *eparameters);
  ~WndEventButton();
 public:
  void CallEvent(void);
 private:
  webpt2Event inputEvent;
  TCHAR *parameters;
};



#endif

