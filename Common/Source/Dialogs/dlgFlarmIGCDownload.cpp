/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "devFlarm.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"
#include "dlgFlarmIGCDownload.h"


#define PRPGRESS_DLG

#define OPEN_STATE   1
#define READ_STATE   2
#define CLOSE_STATE  3
#define EXIT_STATE   4
static volatile int ThreadState =  EXIT_STATE;

static bool OnTimer(WndForm* pWnd);


static WndListFrame *wIGCSelectListList = NULL;
static WndOwnerDrawFrame *wIGCSelectListListEntry = NULL;

bool deb_ = false;
int IGC_Index =0;
int iNoIGCFiles=0;
int IGC_DrawListIndex=0;
uint16_t Sequence=0;

#define MAX_IGCFILES 100
#define REC_TIMEOUT 800




int DownloadError =REC_NO_ERROR;

TCHAR szIGCStrings[MAX_IGCFILES][90 + 1];
TCHAR szIGCSubStrings[MAX_IGCFILES][90 + 1];

int ReadFlarmIGCFile( DeviceDescriptor_t *d, uint8_t IGC_Index);
static WndForm *wf = NULL;
ListElement* pIGCResult = NULL;



void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize);
uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize, uint16_t Timeout);


uint16_t crc_update (uint16_t crc, uint8_t data) {

  crc = crc ^ ((uint16_t)data << 8);
  for (int i=0; i<8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
return crc;
}

uint16_t crc_update16 (uint16_t crc, uint16_t data) {
  crc = crc_update ( crc, lowbyte (data));
  crc = crc_update ( crc, highbyte (data));
  return crc;
}


void SendEscChar(DeviceDescriptor_t *d, uint8_t byte)
{
  if (d && d->Com)
  switch(byte)
  {
      case ESCAPE:
        d->Com->PutChar( ESCAPE);
        d->Com->PutChar( ESC_ESC);
      break;
      case STARTFRAME:
        d->Com->PutChar( ESCAPE);
        d->Com->PutChar( ESC_START);
      break;
      default:
        d->Com->PutChar( byte);
      break;
  }
}


void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize)
{

  uint16_t i;
  uint16_t CRC=0;
  uint8_t blk[8];


  blk[0] = lowbyte (8 +blocksize);  // length
  blk[1] = highbyte(8 +blocksize);  // length
  blk[2] = 1;                       // version
  blk[3] = lowbyte(Sequence);       // sequence
  blk[4] = highbyte(Sequence);      // sequence
  blk[5] = Command;

  d->Com->PutChar( STARTFRAME);
  for( i=0; i < 6; i++)
  {
      CRC =  crc_update(CRC, blk[i]);
  }

  if(pBlock)
  {
    for(i = 0; i < blocksize; i++)
    {
      CRC =  crc_update(CRC, pBlock[i]);
    }
  }
  blk[6] = lowbyte(CRC);
  blk[7] = highbyte(CRC);

  for(i=0; i < 8; i++)
  {
    SendEscChar(d,blk[i]);
  }

  for(i=0; i < blocksize; i++)
  {
    SendEscChar(d,pBlock[i]);
  }

  if(deb_)StartupStore(TEXT("\r\n===="));
 Poco::Thread::sleep(10);
}





uint8_t RecChar8( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{
  uint8_t Tmp;
  uint8_t err;
  err = RecChar(d, &Tmp, Timeout);

  if(!err)
  {
    if(Tmp == ESCAPE)
    {
      err = RecChar(d, &Tmp, Timeout);
      if(Tmp == ESC_ESC)
      {
        Tmp = 0x78;
        if(deb_) { StartupStore(TEXT("ESC_ESC" ));};
      }
      if(Tmp == ESC_START)
      {
        Tmp = 0x73;
        if(deb_) { StartupStore(TEXT("ESC_START"));};
      }
    }
  }
  *inchar = Tmp;
  return err;

}




uint8_t RecChar16( DeviceDescriptor_t *d,uint16_t *inchar, uint16_t Timeout)
{
  ConvUnion tmp;
  bool error  = RecChar8(d, &(tmp.byte[0]), Timeout);
       if(error == REC_NO_ERROR)
         error = RecChar8(d, &(tmp.byte[1]), Timeout);
  *inchar =  tmp.val;
return error;
}


uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize, uint16_t Timeout)
{

uint8_t error  = REC_NO_ERROR ;
uint8_t inchar;
uint8_t Version;
uint16_t CRC_in, CRC_calc=0;

  do{
    error = RecChar(d, &inchar, Timeout);

    if(error)
    {
        { if(deb_)StartupStore(TEXT("STARTFRAME timeout!" )); }
      if(error) return error;
    }
  } while( (inchar != STARTFRAME) && !error);

  if(!error)
    { if(deb_ )StartupStore(TEXT("STARTFRAME OK!"));}
  else
    { if(deb_)StartupStore(TEXT("STARTFRAME fail!" )); }

  error |= RecChar16(d, blocksize , Timeout); CRC_calc = crc_update16 (CRC_calc,*blocksize);{ if(deb_)StartupStore(TEXT("Block Size %u"  ), *blocksize); }if(error != REC_NO_ERROR)  return error;
  error |= RecChar8(d, &Version   , Timeout); CRC_calc = crc_update   (CRC_calc,Version)   ;{ if(deb_)StartupStore(TEXT("Block Ver %u"   ), Version);    }if(error != REC_NO_ERROR)  return error;
  error |= RecChar16(d, Sequence  , Timeout); CRC_calc = crc_update16 (CRC_calc,*Sequence) ;{ if(deb_)StartupStore(TEXT("Block Seq %u"   ), *Sequence);  }if(error != REC_NO_ERROR)  return error;
  error |= RecChar8(d, Command    , Timeout); CRC_calc = crc_update   (CRC_calc,*Command)  ;{ if(deb_)StartupStore(TEXT("Block Cmd %02X" ), *Command);   }if(error != REC_NO_ERROR)  return error;
  error |= RecChar16(d,&CRC_in    , Timeout);                                               { if(deb_)StartupStore(TEXT("Block CRC %04X" ), CRC_in);     }if(error != REC_NO_ERROR)  return error;
  if(deb_) {StartupStore(TEXT("Header  received!" ));}

  if(*blocksize > 8)
    for (uint16_t i = 0 ; i < (*blocksize-8) ; i++)
    {
       error |= RecChar8(d,&inchar   , Timeout);
       if(error != REC_NO_ERROR)
       {
    	 StartupStore(TEXT("Rec Block Body error: %u!"), error);
         return error;
       }
       pBlock[i] = inchar;
       CRC_calc = crc_update (CRC_calc,pBlock[i]); if(error)  return error;
   //    if(deb_) { StartupStore(TEXT("Block[%u]  %02X" ),i, pBlock[i]);}
    }
*blocksize -= 8;
 if(CRC_calc != CRC_in)
 {
   {StartupStore(TEXT("Rec Block CRC error!" ));}
    error = REC_CRC_ERROR;
 }
 else
 {
  if(error)
    {StartupStore(TEXT("Rec Block error  received!" ));}
  else
    {if(deb_) StartupStore(TEXT("Rec Block received OK!" ));}
 }
  return error;
}






static void UpdateList(void) {
  wIGCSelectListList->ResetList();
  wIGCSelectListList->Redraw();
}

static void OnUpClicked(WndButton* Sender) {
    if (IGC_Index > 0) {
        IGC_Index--;
    } else {
        LKASSERT(iNoIGCFiles>0);
        IGC_Index = (iNoIGCFiles - 1);
    }
    wIGCSelectListList->SetItemIndexPos(IGC_Index);
    wIGCSelectListList->Redraw();
    wIGCSelectListListEntry->SetFocus();
}

static void OnDownClicked(WndButton* pWnd) {

    (void)pWnd;

    if (IGC_Index < (iNoIGCFiles - 1)) {
        IGC_Index++;
    } else {
        IGC_Index = 0;
    }
    wIGCSelectListList->SetItemIndexPos(IGC_Index);
    wIGCSelectListList->Redraw();
    wIGCSelectListListEntry->SetFocus();
}




static void OnMultiSelectListListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {

    (void) Sender;

    if (ListInfo->DrawIndex == -1) {
        ListInfo->ItemCount = iNoIGCFiles;

    } else {
        IGC_DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
        IGC_Index = ListInfo->ItemIndex + ListInfo->ScrollIndex;
    }

}




static void OnEnterClicked(WndButton* pWnd) {
TCHAR Tmp[200 ];
    (void)pWnd;

    if (IGC_Index >= iNoIGCFiles) {
        IGC_Index = iNoIGCFiles - 1;
    }

    _stprintf(Tmp, _T("%s %s ?"),MsgToken(2404),szIGCStrings[IGC_Index ]);

     if (MessageBoxX(Tmp, MsgToken(2404),  mbYesNo) == IdYes)  // _@2404 "Download"
     {
       StartupStore(TEXT ("OnEnterClicked: %s"), szIGCStrings[IGC_Index]);
       ThreadState =  OPEN_STATE;        // start thread IGC download
       wf->SetTimerNotify(100, OnTimer); // check for end of download every 100ms
     }

}



static void OnMultiSelectListPaintListItem(WindowControl * Sender, LKSurface& Surface) {

    #define PICTO_WIDTH 50
    Surface.SetTextColor(RGB_BLACK);

    if (IGC_DrawListIndex < iNoIGCFiles)  {

        TCHAR text1[180] = {TEXT("IGC File")};
        TCHAR text2[180] = {TEXT("date")};

       _stprintf(text1, _T("%s"), szIGCStrings [IGC_DrawListIndex] );
       _stprintf(text2, _T("%s"), szIGCSubStrings[IGC_DrawListIndex] );
        Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));

        PixelRect rc = {
            0,
            0,
           0, // DLGSCALE(PICTO_WIDTH),
            static_cast<PixelScalar>(Sender->GetHeight())
        };

        /********************
         * show text
         ********************/

        Surface.SetBackgroundTransparent();
        Surface.SetTextColor(RGB_BLACK);
        Surface.DrawText(rc.right + DLGSCALE(2), DLGSCALE(2), text1);
        int ytext2 = Surface.GetTextHeight(text1);
        Surface.SetTextColor(RGB_DARKBLUE);
        Surface.DrawText(rc.right + DLGSCALE(2), ytext2, text2);
    }
}



static void OnIGCListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {
    (void) Sender;
    IGC_Index = ListInfo->ItemIndex + ListInfo->ScrollIndex;


    if (IGC_Index >= iNoIGCFiles) {
        IGC_Index = iNoIGCFiles - 1;
    }


    if (IGC_Index >= 0) {
      if(Sender) {
        WndForm * pForm = Sender->GetParentWndForm();
        if(pForm) {

             OnEnterClicked(NULL) ;

        }
      }
    }
}


void   StopIGCRead(void)
{

  ThreadState =  EXIT_STATE;
}


static void OnCloseClicked(WndButton* pWnd) {

  IGC_Index = -1;

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      wf->SetTimerNotify(0, NULL);    // reset Timer
      StopIGCRead();
      pForm->SetModalResult(mrCancel);

    }
  }
}


static bool OnTimer(WndForm* pWnd) {
#define MAX_LEN 80
TCHAR Tmp [MAX_LEN];

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
       if( ThreadState ==  EXIT_STATE)
	   {
    	 wf->SetTimerNotify(0, NULL);    // reset Timer
		 switch (DownloadError)
		 {
		   case REC_NO_ERROR     : _sntprintf(Tmp, MAX_LEN, _T("%s\n%s"),  szIGCStrings[IGC_Index] ,MsgToken(2406)); break; // 	_@M2406_ "IGC File download complete"
		   case REC_TIMEOUT_ERROR: _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2407)); break;  // _@M2407_ "Error: receive timeout
		   case REC_CRC_ERROR    : _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2408)); break;  // _@M2408_ "Error: CRC checksum fail"
		   case FILENAME_ERROR   : _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2409)); break;  // _@M2409_ "Error: invalid filename"
		   case FILE_OPEN_ERROR  : _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2410)); break;  // _@M2410_ "Error: can't open file"
		   case IGC_RECEIVE_ERROR: _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2411)); break;  // _@M2411_ "Error: Block invalid"
		   default               : _sntprintf(Tmp, MAX_LEN, _T("%s"), MsgToken(2412)); break;  // _@M2412_ "Error: unknown"
		 }
		 if (MessageBoxX( Tmp, MsgToken(2398), mbOk) == IdYes)  // _@M2406_ "Error: communication timeout"Reboot"
		 {
		 }
		 DownloadError = REC_NO_ERROR;
	   }
    }
  }
  return true;
}


static CallBackTableEntry_t IGCCallBackTable[] = {
    OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
    OnListCallbackEntry(OnMultiSelectListListInfo),
    ClickNotifyCallbackEntry(OnCloseClicked),
    ClickNotifyCallbackEntry(OnUpClicked),
    ClickNotifyCallbackEntry(OnEnterClicked),
    ClickNotifyCallbackEntry(OnDownClicked),


	EndCallBackEntry()
};


ListElement* dlgIGCSelectListShowModal( DeviceDescriptor_t *d) {
  LockComm();

    IGC_Index = -1;
    uint8_t blk[10];
    uint16_t RecSequence;
    uint8_t RecCommand;
    uint8_t pBlock[100];
    uint16_t blocksize;
    TCHAR  TempString[255];
    uint8_t retry = 0;
    uint8_t err  = REC_NO_ERROR ;
    if (d && d->Com)
    {
#ifdef PRPGRESS_DLG
      CreateIGCProgressDialog(TEXT("..."));
#endif

        d->Com->WriteString(TEXT("$PFLAX\r\n"));  // set to binary
        if(deb_) StartupStore(TEXT("$PFLAX\r "));


   /*   if(deb_) */StartupStore(TEXT("PING "));
   retry=0;

     do{
       _sntprintf(TempString, 255, _T("PING Flarm %u/15..."), retry);
#ifdef PRPGRESS_DLG
         IGCProgressDialogText(TempString) ;
#endif
      SendBinBlock(d, Sequence++, PING, NULL, 0);
      err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
     }while ((err != REC_NO_ERROR) && retry++ < 15);


      int IGCCnt =0;

      if (err == REC_NO_ERROR)
      {


      do {
        retry=0;
        do{
           blk[0] =IGCCnt;
           SendBinBlock(d, Sequence++, SELECTRECORD, &blk[0], 1);
           err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
        } while ((err== REC_CRC_ERROR) && (retry++ <4));

        if(RecCommand == ACK)
        {
          retry = 0;
          do{
            SendBinBlock(d, Sequence++, GETRECORDINFO, NULL, 0);
            err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
          } while ((err== REC_CRC_ERROR) && (retry++ <4));


          pBlock[blocksize++] = 0;
          if(RecCommand == ACK)
          {

            for(uint16_t i = 0; i < blocksize-2; i++)
               TempString[i]= (TCHAR) pBlock[i+2];
        /*    if(deb_) */  StartupStore(TEXT("%s "),TempString);
            TCHAR empty[3] = _T("");
            TCHAR* remaining=NULL;
            TCHAR* Filename  = _tcstok_r(TempString,TEXT("|"),&remaining); if(Filename == NULL)  {Filename = empty;};
            TCHAR* Date      = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Date     == NULL)  {Date     = empty;};
            TCHAR* Takeoff   = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Takeoff  == NULL)  {Takeoff  = empty;};
            TCHAR* Duration  = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Duration == NULL)  {Duration = empty;};
            TCHAR* Pilot     = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Pilot    == NULL)  {Pilot    = empty;};
            TCHAR* CN        = _tcstok_r(NULL,TEXT("|"),&remaining);       if(CN       == NULL)  {CN       = empty;};
            _stprintf( szIGCStrings[IGCCnt]   ,_T("%s (%s  [%5s])"),Filename, Date,Takeoff);
            _stprintf( szIGCSubStrings[IGCCnt],_T("%s"),Duration);
            if(Pilot) { _tcscat( szIGCSubStrings[IGCCnt] ,_T(" ") );_tcscat( szIGCSubStrings[IGCCnt] ,Pilot);}
            if(CN)    { _tcscat( szIGCSubStrings[IGCCnt] ,_T(" "));_tcscat( szIGCSubStrings[IGCCnt] ,CN);}

            _sntprintf(TempString, 255, _T("Getting Info %u..."), IGCCnt);
#ifdef PRPGRESS_DLG
            IGCProgressDialogText(TempString) ;
#endif

          }
        }
        IGCCnt++;
     } while (RecCommand == ACK);

     }
      iNoIGCFiles = IGCCnt;
#ifdef PRPGRESS_DLG
     CloseIGCProgressDialog();
#endif
     if (err != REC_NO_ERROR)
     {
		  if (MessageBoxX(MsgToken(2401), MsgToken(2401), mbOk) == IdYes)  // _@M2401_ "No Device found"
		  {

		  }
     }
    }

    if (iNoIGCFiles == 0)
    {
      return NULL;
    }

    wf = dlgLoadFromXML(IGCCallBackTable, ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P);

    if (!wf) return NULL;

    wIGCSelectListList = (WndListFrame*) wf->FindByName(TEXT("frmMultiSelectListList"));
    LKASSERT(wIGCSelectListList != NULL);
    wIGCSelectListList->SetBorderKind(BORDERLEFT);
    wIGCSelectListList->SetEnterCallback(OnIGCListEnter);

    wIGCSelectListListEntry = (WndOwnerDrawFrame*) wf->FindByName(TEXT("frmMultiSelectListListEntry"));
    if(wIGCSelectListListEntry) {
        /*
         * control height must contains 2 text Line
         * Check and update Height if necessary
         */
        LKWindowSurface windowSurface(MainWindow);
        LKBitmapSurface tmpSurface(windowSurface, 1, 1);
        const auto oldFont = tmpSurface.SelectObject(wIGCSelectListListEntry->GetFont());
        const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
        tmpSurface.SelectObject(oldFont);
        const int wHeight = wIGCSelectListListEntry->GetHeight();
        if(minHeight > wHeight) {
            wIGCSelectListListEntry->SetHeight(minHeight);
        }
        wIGCSelectListListEntry->SetCanFocus(true);
    } else LKASSERT(0);

    UpdateList();

    wf->ShowModal();
    wIGCSelectListList->Redraw();
    delete wf;

    wf = NULL;

     _sntprintf(TempString, 255, _T("%s?"), MsgToken(2403)); // _@M2403_ "Reset FLARM"
    if (MessageBoxX(MsgToken(2413), TempString, mbYesNo) == IdYes) // _@M2413_ "FLARM need reboot for normal operation\n reboot now?"
    {
        if(deb_)StartupStore(TEXT("EXIT "));
        SendBinBlock(d, Sequence++, EXIT, NULL, 0);
        RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);

        d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
        StartupStore(TEXT("$PFLAR,0*55\r\n"));
    }

    UnlockComm();
    StartupStore(TEXT("Leaving IGC dialog "));
    ThreadState =  EXIT_STATE;

    return pIGCResult;
}




int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_Index)
{

static FILE *f;
TCHAR Tmp[200];
TCHAR Name[200];
static  uint8_t err = 0;
static uint8_t   Command = SELECTRECORD;
static  uint8_t   pBlock[2000];
pBlock[0] = IGC_Index;
uint16_t  blocksize=1;
uint16_t  Seq;

if (ThreadState == EXIT_STATE)
	return 0;

if(d != NULL)
{
  /******OPEN STATE *********************************************************************************/
  if (ThreadState == OPEN_STATE)
  {
    StartupStore(TEXT ("OPEN_STATE: %s"), szIGCStrings[IGC_Index]);
	if(f) {fclose(f); f = NULL;}
	err = REC_NO_ERROR;
    _sntprintf(Name, 200, _T("%s"), szIGCStrings[IGC_Index ]);
    TCHAR* remaining;
    TCHAR* Filename  = _tcstok_r(Name ,TEXT(" "),&remaining);
    if(Filename == NULL)  err = FILENAME_ERROR;

    TCHAR filename[MAX_PATH];
    LocalPath(filename, _T(LKD_LOGS), Filename);
     f = _tfopen(filename, TEXT("w"));
    if(f == NULL)   { err = FILE_OPEN_ERROR;  }   // #define FILE_OPEN_ERROR 5

    _sntprintf(Tmp, 200, TEXT("IGC Dowlnoad File : %s "),szIGCStrings[IGC_Index ]);
    StartupStore(Tmp);
    SendBinBlock(d,  Sequence++,  SELECTRECORD, &IGC_Index,  1);
    err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize, REC_TIMEOUT);

    if(err != REC_NO_ERROR) err =  IGC_RECEIVE_ERROR;

  #ifdef PRPGRESS_DLG
    CreateIGCProgressDialog(Tmp);
  #endif
    ThreadState =  READ_STATE;

  }
  /******READ STATE *********************************************************************************/
  if (ThreadState == READ_STATE)
  {
    blocksize = 0;
    SendBinBlock(d,  Sequence++,  GETIGCDATA, &pBlock[0], 0);
    err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize,  6* REC_TIMEOUT);
    if(err==REC_NO_ERROR)
      _sntprintf(Tmp, 200, _T("%s %s: %u%%..."),MsgToken(2400), Name,pBlock[2]); // _@M2400_ "Downloading"
    if((Sequence %10) == 0)
      StartupStore(TEXT("%s"),Tmp);

#ifdef PRPGRESS_DLG
    IGCProgressDialogText(Tmp) ;  // update progress dialog text
#endif

    for(int i=0; i < blocksize-3; i++)
    {
      fputc(pBlock[3+i],f);
      if(pBlock[3+i] == EOF_)
        Command = EOF_;
    }

    if ((Command == ACK) && !err)
      ThreadState =  READ_STATE;
    else
      ThreadState = CLOSE_STATE;
  }
  /******CLOSE ON ERROR *********************************************************************************/
  if(err != REC_NO_ERROR) ThreadState = CLOSE_STATE;

  /******CLOSE STATE *********************************************************************************/
  if (ThreadState == CLOSE_STATE)
  {
#ifdef PRPGRESS_DLG
  CloseIGCProgressDialog();
#endif
    if(f) {fclose(f); f = NULL;}

    ThreadState =  EXIT_STATE;
    StartupStore(TEXT ("EXIT_STATE"));
    if(err != REC_NO_ERROR)
    {
      StartupStore(TEXT ("Error Code:%u"), err);
      DownloadError = err;
      err = REC_NO_ERROR;
    }

  }
}  // if(d)
return 0;
}


class IGCReadThread : public Poco::Runnable
{
public:

    void Start() {
        if(!Thread.isRunning())
        {

          bStop = false;
          Thread.start(*this);
        }
    }

    void Stop() {
		if(Thread.isRunning())
		{
			bStop = true;
			Thread.join();
		}
    }

protected:
    bool bStop;
    Poco::Thread Thread;

    void run() {
	  PeriodClock Timer;
	  while(!bStop) {
		if( ThreadState !=  EXIT_STATE)
		{
		  ReadFlarmIGCFile( CDevFlarm::GetDevice(), IGC_Index);
		}
//		unsigned n = Clamp<unsigned>(200U - Timer.ElapsedUpdate(), 0U, 400U);
		Sleep(100);
		Timer.Update();
	  }
	}
};

IGCReadThread IGCReadThreadThreadInstance;


void StartIGCReadThread() {

  StartupStore(TEXT("Start IGC Thread !"));
  IGCReadThreadThreadInstance.Start();
}

void StopIGCReadThread() {

  StartupStore(TEXT("Stop IGC Thread !"));
  IGCReadThreadThreadInstance.Stop();
}
