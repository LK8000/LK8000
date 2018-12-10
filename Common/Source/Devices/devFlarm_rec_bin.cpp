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
#include "Dialogs/dlgProgress.h"

bool deb_ = true;
int IGC_Index =0;
int iNoIGCFiles=5;
int IGC_DrawListIndex=0;
uint16_t Sequence=0;

#define MAX_IGCFILES 100
TCHAR szIGCStrings[MAX_IGCFILES][90 + 1]={_T("IGC File 1"),_T("IGC File 2"),_T("IGC File 3")};
TCHAR szIGCSubStrings[MAX_IGCFILES][90 + 1]={_T(""),_T(""),_T("")};
ListElement* dlgIGCSelectListShowModal(  DeviceDescriptor_t *d) ;
int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_Index);
static WndForm *wf = NULL;
static WndListFrame *wIGCSelectListList = NULL;
static WndOwnerDrawFrame *wIGCSelectListListEntry = NULL;
ListElement* pIGCResult = NULL;

DeviceDescriptor_t *global_d= NULL;


void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize);
bool RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize);

PDeviceDescriptor_t CDevFlarm::m_pDevice=NULL;
BOOL CDevFlarm::m_bCompassCalOn=FALSE;
Mutex* CDevFlarm::m_pCritSec_DeviceData=NULL;
double CDevFlarm::m_abs_press=0.0;
double CDevFlarm::m_delta_press=0.0;




bool CDevFlarm::Register(){
	return devRegister(GetName(), cap_baro_alt|cap_vario, &Install);
}

BOOL CDevFlarm::Install( PDeviceDescriptor_t d ) {
  StartupStore(_T("Flarm Drvier Install %s"), NEWLINE);
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = ParseNMEA;
	d->PutMacCready = NULL;
	d->PutBugs = NULL;
	d->PutBallast = NULL;
	d->Open = Open;
	d->Close = Close;
	d->Init = NULL;
	d->LinkTimeout = NULL;
	d->Declare = NULL;
	d->IsGPSSource = GetFalse;
	d->IsBaroSource = GetTrue;
	d->Config = Config;
	d->ParseStream    = FlarmParseString;

	return(TRUE);
}






BOOL CDevFlarm::Open( PDeviceDescriptor_t d) {
	m_pDevice = d;

	m_pCritSec_DeviceData = new Mutex();

	return TRUE;
}

BOOL CDevFlarm::Close (PDeviceDescriptor_t d) {
	m_pDevice = NULL;

	delete m_pCritSec_DeviceData;
	m_pCritSec_DeviceData = NULL;

	return TRUE;
}






void CDevFlarm::LockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->Lock();
	}
}

void CDevFlarm::UnlockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->Unlock();
	}
}

BOOL CDevFlarm::ParseNMEA( DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO ) {
	return FALSE;
}








BOOL CDevFlarm::SetDeviceName( PDeviceDescriptor_t d, const tstring& strName ){
/*	if (d && d->Com && strName.size() <= 15) {
		d->Com->WriteString(TEXT("$PCPILOT,C,SET,"));
		d->Com->WriteString(strName.c_str());
		d->Com->WriteString(TEXT("\r\n"));
		return GetDeviceName(d);
	}*/
	return FALSE;
}



CallBackTableEntry_t CDevFlarm::CallBackTable[]={
  EndCallBackEntry()
};

BOOL CDevFlarm::Config(PDeviceDescriptor_t d){
	if(m_pDevice != d) {
		StartupStore(_T("Flarm Config : Invalide device descriptor%s"), NEWLINE);
		return FALSE;
	}

	WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_DEVFLARM);
	if(wf) {

    	WndButton *wBt = NULL;

    	wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnCloseClicked);
    	}

    	wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
    	if(wBt){
        	wBt->SetOnClickNotify(OnIGCDownloadClicked);
    	}



//		wf->SetTimerNotify(1000, OnTimer);
		wf->ShowModal();

		delete wf;
		wf=NULL;
	}
	return TRUE;
}

bool CDevFlarm::OnTimer(WndForm* pWnd){
  Update(pWnd);
  return true;
}

void CDevFlarm::OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



void CDevFlarm::OnIGCDownloadClicked(WndButton* pWnd) {
	(void)pWnd;
	if(m_pDevice) {
	dlgIGCSelectListShowModal(m_pDevice);
	}
	/*
	if(m_pDevice) {
		SetZeroDeltaPressure(m_pDevice);
	}*/
}

void CDevFlarm::Update(WndForm* pWnd) {

}




static void OnEnterClicked(WndButton* pWnd) {

    (void)pWnd;

    if (IGC_Index >= iNoIGCFiles) {
        IGC_Index = iNoIGCFiles - 1;
    }
    ReadFlarmIGCFile( global_d, IGC_Index);

}


static void OnCloseClicked(WndButton* pWnd) {

  IGC_Index = -1;

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
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


int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_Index)
{

if(d)
{
   TCHAR Tmp[200];
   _stprintf(Tmp, _T("Dolanload IGC File: %s ?"),szIGCStrings[IGC_Index ]);
    if (MessageBoxX(
                     // LKTOKEN  _@M198_ = "Confirm Exit?"
                     Tmp,
                     TEXT("IGC Download"), mbYesNo) == IdYes)
    {

  _sntprintf(Tmp, 200, _T("%s: %s..."), MsgToken(1400), MsgToken(571));
  CreateProgressDialog(Tmp);



  bool err = false;
  uint8_t retry =0;
  do{
    StartupStore(TEXT("IGC Dowlnoad File : %s "),szIGCStrings[IGC_Index ]);

    FILE *f;

    TCHAR* remaining;
    TCHAR* Filename  = _tcstok_r(szIGCStrings[IGC_Index ],TEXT(" "),&remaining);
    TCHAR filename[MAX_PATH];
    LocalPath(filename, _T(LKD_LOGS), Filename);
    f = _tfopen(filename, TEXT("w"));



    uint8_t   Command = SELECTRECORD;

    uint8_t   pBlock[2048];
    pBlock[0] = IGC_Index;
    uint16_t  blocksize=1;

    uint16_t  Seq;
    SendBinBlock(d,  Sequence++,  SELECTRECORD, &IGC_Index,  1);
    err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize);




    blocksize = 0;
    do
    {
  //      Poco::Thread::sleep(5);
      SendBinBlock(d,  Sequence++,  GETIGCDATA, &pBlock[0], 0);
      err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize);
 //     Poco::Thread::sleep(10);
      if(Sequence % 10 == 0)
        {
          StartupStore(TEXT("IGC Dowlnoad: %04u %u%%"),Sequence ,pBlock[2]);
          _sntprintf(Tmp, 200, _T("Downloading %s: %u%%..."), szIGCStrings[IGC_Index],pBlock[2]);
          ProgressDialogText(Tmp) ;
        }
      for(int i=0; i < blocksize-3; i++)
      {
        fputc(pBlock[3+i],f);
        if(pBlock[3+i] == EOF_)
          Command = EOF_;
      }
    }
    while((Command == ACK) && !err);

  retry++;
  fclose(f);
  } while (err && (retry < 3));

  CloseProgressDialog();
}
}

  return 0;

}

static void OnTaskSelectListListEnter(WindowControl * Sender,
                                       WndListFrame::ListInfo_t *ListInfo) {
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


static CallBackTableEntry_t IGCCallBackTable[] = {
    OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
    OnListCallbackEntry(OnMultiSelectListListInfo),
    ClickNotifyCallbackEntry(OnCloseClicked),
    ClickNotifyCallbackEntry(OnUpClicked),
    ClickNotifyCallbackEntry(OnEnterClicked),
    ClickNotifyCallbackEntry(OnDownClicked),
    EndCallBackEntry()
};



#define highbyte(a)  (((a)>>8) & 0xFF)
#define lowbyte(a)   ((a) & 0xFF)



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


BinBlock RecBlock;



void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize)
{

  uint16_t i;
  uint16_t CRC=0;
  uint8_t blk[8];
  RecBlock.RecReady = false;

  blk[0] = lowbyte (8 +blocksize);  // length
  blk[1] = highbyte(8 +blocksize);  // length
  blk[2] = 1;                       // version
  blk[3] = lowbyte(Sequence);       // sequence
  blk[4] = highbyte(Sequence);      // sequence
  blk[5] = Command;

  if(deb_)StartupStore(TEXT("Send blocksize:%u"), blocksize+8);
  if(deb_)StartupStore(TEXT("Send version:%u"), blk[2] );
  if(deb_)StartupStore(TEXT("Send sequence:%u"), Sequence );
  if(deb_)StartupStore(TEXT("Send Command:%x"), Command );
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
  if(deb_)StartupStore(TEXT("Send CRC:%X"), CRC );
  for(i=0; i < 8; i++)
  {
    SendEscChar(d,blk[i]);
  }

  for(i=0; i < blocksize; i++)
  {
    SendEscChar(d,pBlock[i]);
  }

}


#define REC_BUFFERSIZE 5000
uint8_t RingBuff[REC_BUFFERSIZE+1];
volatile  uint16_t InCnt=0;
volatile  uint16_t OutCnt=0;
static bool recEnable = false;











BOOL CDevFlarm::FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;
static uint16_t CRC_calc=0;
int cnt=0;
static BOOL ESCmode = false;
static bool FrameStart = false;

  while (cnt < len)
  {
    uint8_t InByte = (uint8_t) String[cnt++] ;
    {
      if(InByte == ESCAPE)
        ESCmode = true;
      else
      {
        if(ESCmode)
        {
          if(InByte == ESC_ESC)  InByte = ESCAPE;  else
            if(InByte == ESC_START)InByte = STARTFRAME;
          ESCmode = false;
        }

        if(!ESCmode)
        {
          if((InByte == STARTFRAME) && !RecBlock.RecReady)
          {
            if(deb_)StartupStore(TEXT("STARTFRAME received start new block!"   ));
            RecBlock.BlkIndex = 0;
            RecBlock.PL_Idx =0;
            RecBlock.RecReady = false;
            RecBlock.Error = NO_BLK_ERROR;
            CRC_calc = 0;
            ESCmode = false;
            FrameStart = true;
          }
          else
          {
            if(  FrameStart)
            {
              switch(RecBlock.BlkIndex++)
              {
                case 0: RecBlock.blocksize.byte[0] = InByte; CRC_calc = crc_update(CRC_calc,InByte);                                                                          break;
                case 1: RecBlock.blocksize.byte[1] = InByte; CRC_calc = crc_update(CRC_calc,InByte); if(deb_)StartupStore(TEXT("Block Size %u"  ), RecBlock.blocksize.val);   break;
                case 2: RecBlock.Version           = InByte; CRC_calc = crc_update(CRC_calc,InByte); if(deb_)StartupStore(TEXT("Block Ver %u"   ), RecBlock.Version      );   break;
                case 3: RecBlock.Sequence.byte[0]  = InByte; CRC_calc = crc_update(CRC_calc,InByte);                                                                          break;
                case 4: RecBlock.Sequence.byte[1]  = InByte; CRC_calc = crc_update(CRC_calc,InByte); if(deb_)StartupStore(TEXT("Block Seq %u"   ), RecBlock.Sequence.val );   break;
                case 5: RecBlock.Command           = InByte; CRC_calc = crc_update(CRC_calc,InByte); if(deb_)StartupStore(TEXT("Block Cmd %02X" ), RecBlock.Command      );   break;
                case 6: RecBlock.CRC_in.byte[0]    = InByte;                                                                                                                  break;
                case 7: RecBlock.CRC_in.byte[1]    = InByte;                                         if(deb_)StartupStore(TEXT("Block CRC %04X" ), RecBlock.CRC_in.val   );   break;
                default:
                  RecBlock.Payload[RecBlock.PL_Idx++] = InByte; CRC_calc = crc_update(CRC_calc,InByte);
                break;
              }
              if(RecBlock.BlkIndex == RecBlock.blocksize.val)
              {
                RecBlock.RecReady = true;
                FrameStart = false;
                if(RecBlock.CRC_in.val != CRC_calc)
                {
                 /* if(deb_)*/StartupStore(TEXT("CRC fail!rec: %X cal: %X "),RecBlock.CRC_in ,CRC_calc );
                  RecBlock.Error = CRC_BLKERROR;
                }
              /*  if(deb_)*/StartupStore(TEXT("\r\n===="));
              }
            }
          }
        } // if
      }  // else
    }  // if(!ESCmode)
  } //  (cnt < len)

return  true;
}




bool RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize)
{
  uint16_t timecnt=0;
  while(!RecBlock.RecReady)
  {
    if(timecnt++ > 5000) return true;
    Sleep(1);
  }
  *Sequence = RecBlock.Sequence.val;
  *Command  = RecBlock.Command;
  *blocksize = RecBlock.blocksize.val -8 ;
  for (uint16_t i=0; i < *blocksize; i++)
    pBlock[i] = RecBlock.Payload[i];

  return RecBlock.Error;
}



ListElement* dlgIGCSelectListShowModal( DeviceDescriptor_t *d) {

/*
  d->Com->StopRxThread();
  d->Com->SetRxTimeout(10000);                     // set RX timeout to 50[ms]
*/
  LockComm();

    IGC_Index = -1;
    iNoIGCFiles = 8;
    uint8_t blk[10];
    uint16_t RecSequence;
    uint8_t RecCommand;
    uint8_t pBlock[1024];
    uint16_t blocksize;
    Sequence = 0;
    if (d && d->Com)
    {
        global_d = d;
      d->Com->WriteString(TEXT("$PFLAX\r"));  // set to binary
      if(deb_)   StartupStore(TEXT("$PFLAX\r "));

      recEnable = true;


      if(deb_)StartupStore(TEXT("PING "));
      SendBinBlock(d, Sequence++, PING, NULL, 0);
      RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize);

      int IGCCnt =0;
      bool err; uint8_t retry = 0;
      CreateProgressDialog(TEXT("..."));
      do {
        blk[0] =IGCCnt;
        do{
           SendBinBlock(d, Sequence++, SELECTRECORD, &blk[0], 1);
           err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize);
        } while (err && (retry++ <4));

        if(RecCommand == ACK)
        {
          retry = 0;
          do{
            SendBinBlock(d, Sequence++, GETRECORDINFO, NULL, 0);
            err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize);
          } while (err && (retry++ <4));


          pBlock[blocksize++] = 0;
          if(RecCommand == ACK)
          {
            TCHAR  TempString[255];
            for(uint16_t i = 0; i < blocksize-2; i++)
               TempString[i]= (TCHAR) pBlock[i+2];
            if(deb_)   StartupStore(TEXT("%s "),TempString);
            TCHAR* remaining=NULL;
            TCHAR* Filename  = _tcstok_r(TempString,TEXT("|"),&remaining);
            TCHAR* Date      = _tcstok_r(NULL,TEXT("|"),&remaining);
            TCHAR* Takeoff   = _tcstok_r(NULL,TEXT("|"),&remaining);
            TCHAR* Duration  = _tcstok_r(NULL,TEXT("|"),&remaining);
            TCHAR* Pilot     = _tcstok_r(NULL,TEXT("|"),&remaining);
            TCHAR* CN        = _tcstok_r(NULL,TEXT("|"),&remaining);
            _stprintf( szIGCStrings[IGCCnt]   ,_T("%s (%s  [%5s])"),Filename, Date,Takeoff);
            _stprintf( szIGCSubStrings[IGCCnt],_T("%s"),Duration);
            if(Pilot) { _tcscat( szIGCSubStrings[IGCCnt] ,_T(" ") );_tcscat( szIGCSubStrings[IGCCnt] ,Pilot);}
            if(CN)    { _tcscat( szIGCSubStrings[IGCCnt] ,_T(" "));_tcscat( szIGCSubStrings[IGCCnt] ,CN);}

            _sntprintf(TempString, 255, _T("Getting Info %u..."), IGCCnt);
            ProgressDialogText(TempString) ;

          }
        }
        IGCCnt++;
     } while (RecCommand == ACK);
     CloseProgressDialog();
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
    wIGCSelectListList->SetEnterCallback(OnTaskSelectListListEnter);

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

    iNoIGCFiles = 0;
/*
    if(deb_)StartupStore(TEXT("EXiT "));
    SendBinBlock(d, Sequence++, EXIT, NULL, 0);
    RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize);
         Poco::Thread::sleep(205);*/
    StartupStore(TEXT("Leaving IGC dialog "));

    UnlockComm();
/*
    d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
    d->Com->StartRxThread();                       // restart RX thread
*/

    return pIGCResult;
}

