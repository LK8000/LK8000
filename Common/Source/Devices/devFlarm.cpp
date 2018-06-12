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

#define PRPGRESS_DLG


bool deb_ = false;
int IGC_Index =0;
int iNoIGCFiles=0;
int IGC_DrawListIndex=0;
uint16_t Sequence=0;

#define MAX_IGCFILES 100
#define REC_TIMEOUT 500
#define REC_CRC_ERROR     2
#define REC_TIMEOUT_ERROR 1
#define REC_NO_ERROR      0

TCHAR szIGCStrings[MAX_IGCFILES][90 + 1];
TCHAR szIGCSubStrings[MAX_IGCFILES][90 + 1];
ListElement* dlgIGCSelectListShowModal(  DeviceDescriptor_t *d) ;
int ReadFlarmIGCFile( DeviceDescriptor_t *d, uint8_t IGC_Index);
static WndForm *wf = NULL;
static WndListFrame *wIGCSelectListList = NULL;
static WndOwnerDrawFrame *wIGCSelectListListEntry = NULL;
ListElement* pIGCResult = NULL;

void StartIGCReadThread() ;

void StopIGCReadThread() ;



void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize);
uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize, uint16_t Timeout);

PDeviceDescriptor_t CDevFlarm::m_pDevice=NULL;
BOOL CDevFlarm::m_bCompassCalOn=FALSE;
Mutex* CDevFlarm::m_pCritSec_DeviceData=NULL;
double CDevFlarm::m_abs_press=0.0;
double CDevFlarm::m_delta_press=0.0;

TCHAR CDevFlarm::m_szVersion[15]={0};




bool CDevFlarm::Register(){
	return devRegister(GetName(), cap_baro_alt|cap_vario, &Install);
}

BOOL CDevFlarm::Install( PDeviceDescriptor_t d ) {
  StartupStore(_T("Flarm Drvier Install %s"), NEWLINE);
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = NULL ; // ParseNMEA;
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
	d->ParseStream  = FlarmParseString;

	return(TRUE);
}


#define REC_BUFFERSIZE 512
uint8_t RingBuff[REC_BUFFERSIZE+1];
volatile  uint16_t InCnt=0;
volatile  uint16_t OutCnt=0;
static bool recEnable = true;

BOOL CDevFlarm::FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;

int cnt=0;

if(recEnable)
  while (cnt < len)
  {
    RingBuff[InCnt++] = (TCHAR) String[cnt++];
    InCnt %= REC_BUFFERSIZE;
  } //  (cnt < len)

return  true;
}


uint8_t RecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{

  uint16_t TimeCnt =0;
  uint8_t Tmp;
  while(OutCnt == InCnt)
  {
    Poco::Thread::sleep(1);
    Poco::Thread::Thread::yield();
    if(TimeCnt++ > Timeout)
    {
      {StartupStore(TEXT("REC_TIMEOUT_ERROR" ));}
      return REC_TIMEOUT_ERROR;
    }
  }
  Tmp = RingBuff[OutCnt++];
  OutCnt %= REC_BUFFERSIZE;
  if(inchar)
    *inchar = Tmp;

  return REC_NO_ERROR;
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


inline double int16toDouble(int v) {
	return (double)(int16_t)v;
};

inline double int24toDouble(int v) {
	if(v > (1<<23)){
		v = -(v - (1<<24)+1);
	}
	return v;
};

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
	tnmeastring wiss(String);
	TCHAR* strToken = wiss.GetNextString();

 	if(_tcscmp(strToken,TEXT("$PCPROBE"))==0) {

  		strToken = wiss.GetNextString();

  		// this sentence must handled first, also we can't detect end of Compass Calibration.
		if (_tcscmp(strToken,TEXT("COMPASSCALIBRATION"))==0) {
			// $PCPROBE,COMPASSCALIBRATION
			//  The calibration of the accelerometers and of the magnetometers is being performed

			// no other thread modify m_bCompassCal Flag : no lock needed for read in this thread
			if(!m_bCompassCalOn){
				LockDeviceData();
				m_bCompassCalOn=TRUE;
				UnlockDeviceData();
			}

			return TRUE;
		}

		// if we receive sentence other than compass calibration -> compass calibration is finish.
		// no other thread modify m_bCompassCal Flag : no lock needed for read in this thread
		if(m_bCompassCalOn) {
			LockDeviceData();
			m_bCompassCalOn=FALSE;
			UnlockDeviceData();
		}




 	}
	return FALSE;
}










BOOL CDevFlarm::GetDeviceName( PDeviceDescriptor_t d ){
  /*  if (d && d->Com) {
    	d->Com->WriteString(TEXT("$PCPILOT,C,GETNAME\r\n"));
    }*/
	return TRUE;
}



BOOL CDevFlarm::GetFirmwareVersion(PDeviceDescriptor_t d) {
/*    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,GETFW\r\n"));
    }*/
	return TRUE;
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

        GetFirmwareVersion(m_pDevice);

//              wf->SetTimerNotify(1000, OnTimer);
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
      StopIGCReadThread();
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


void CDevFlarm::OnRebootClicked(WndButton* pWnd) {
        (void)pWnd;
        if(m_pDevice) {
            FlarmReboot(m_pDevice);
        }
}

BOOL CDevFlarm::FlarmReboot(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
    }
    return TRUE;
}

void CDevFlarm::Update(WndForm* pWnd) {
	TCHAR Temp[50] = {0};

	LockFlightData();

	UnlockFlightData();

	LockDeviceData();
	_stprintf(Temp, TEXT("Flarm - Version: %s"), m_szVersion);
	UnlockDeviceData();

	pWnd->SetCaption(Temp);


}



static void OnEnterClicked(WndButton* pWnd) {
TCHAR Tmp[200 ];
    (void)pWnd;

    if (IGC_Index >= iNoIGCFiles) {
        IGC_Index = iNoIGCFiles - 1;
    }

    _stprintf(Tmp, _T("%s %s ?"),MsgToken(2355),szIGCStrings[IGC_Index ]);
     if (MessageBoxX(Tmp, MsgToken(2355),  mbYesNo) == IdYes)  // _@M2355_ "IGC Download"
     {
       StartIGCReadThread();
     }

}


static void OnCloseClicked(WndButton* pWnd) {

  IGC_Index = -1;
  StopIGCReadThread();
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

#define OPEN_STATE   1
#define READ_STATE   2
#define CLOSE_STATE  3
#define EXIT_STATE   4
static int ThreadState =  EXIT_STATE;

int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_Index)
{

static FILE *f;
static int retry =0;
TCHAR Tmp[200];
TCHAR Name[200];
static  bool err = false;
static uint8_t   Command = SELECTRECORD;
static  uint8_t   pBlock[2000];
pBlock[0] = IGC_Index;
uint16_t  blocksize=1;
uint16_t  Seq;


if(d != NULL)
{

  if (ThreadState == OPEN_STATE)
  {

    _sntprintf(Name, 200, _T("%s"), szIGCStrings[IGC_Index ]);
    TCHAR* remaining;
    TCHAR* Filename  = _tcstok_r(Name ,TEXT(" "),&remaining);
    TCHAR filename[MAX_PATH];
    LocalPath(filename, _T(LKD_LOGS), Filename);
    if(f) fclose(f);
    f = _tfopen(filename, TEXT("w"));
    _sntprintf(Tmp, 200, TEXT("IGC Dowlnoad File : %s "),szIGCStrings[IGC_Index ]);
    StartupStore(Tmp);
    SendBinBlock(d,  Sequence++,  SELECTRECORD, &IGC_Index,  1);
    err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize, REC_TIMEOUT);

  #ifdef PRPGRESS_DLG
    CreateIGCProgressDialog(Tmp);
  #endif
    ThreadState =  READ_STATE;
    return 0;

  }
  /****************************************************************************************/
  if (ThreadState == READ_STATE)
  {
    blocksize = 0;
    SendBinBlock(d,  Sequence++,  GETIGCDATA, &pBlock[0], 0);
    err = RecBinBlock(d,  &Seq, &Command, &pBlock[0], &blocksize, 10000);
    if(err==0)
      _sntprintf(Tmp, 200, _T("Downloading %s: %u%%..."), Name,pBlock[2]);
    if((Sequence %10) == 0)
      StartupStore(TEXT("%s"),Tmp);
#ifdef PRPGRESS_DLG
    IGCProgressDialogText(Tmp) ;
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
      ThreadState =  CLOSE_STATE;

    if(err)
    {
      if(retry++ < 4)
        ThreadState =  OPEN_STATE;
      else
        ThreadState =  CLOSE_STATE;
    }
    return 0;
  }

  if (ThreadState == CLOSE_STATE)
  {
    fclose(f);
  #ifdef PRPGRESS_DLG
    CloseIGCProgressDialog();
  #endif
    retry=0;
    StopIGCReadThread();
    ThreadState =  EXIT_STATE;
    return 0;
  }

}  // if(d)
return 0;
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


static CallBackTableEntry_t IGCCallBackTable[] = {
    OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
    OnListCallbackEntry(OnMultiSelectListListInfo),
    ClickNotifyCallbackEntry(OnCloseClicked),
    ClickNotifyCallbackEntry(OnUpClicked),
    ClickNotifyCallbackEntry(OnEnterClicked),
    ClickNotifyCallbackEntry(OnDownClicked),
    EndCallBackEntry()
};




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

bool error  = false;
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
  if(error)
    {StartupStore(TEXT("Rec Block error  received!" ));}
  else
    {if(deb_) StartupStore(TEXT("Rec Block received OK!" ));}

  return error;
}





ListElement* dlgIGCSelectListShowModal( DeviceDescriptor_t *d) {
  LockComm();



    IGC_Index = -1;
  //  iNoIGCFiles = 0;
    uint8_t blk[10];
    uint16_t RecSequence;
    uint8_t RecCommand;
    uint8_t pBlock[100];
    uint16_t blocksize;
    TCHAR  TempString[255];
    bool err; uint8_t retry = 0;
    if (d && d->Com)
    {
#ifdef PRPGRESS_DLG
      CreateIGCProgressDialog(TEXT("..."));
#endif

        d->Com->WriteString(TEXT("$PFLAX\r\n"));  // set to binary
      /*  if(deb_)   */ StartupStore(TEXT("$PFLAX\r "));


   /*   if(deb_)*/ StartupStore(TEXT("PING "));
   retry=0;

     do{

       _sntprintf(TempString, 255, _T("PING Flarm %u..."), retry);
#ifdef PRPGRESS_DLG
         IGCProgressDialogText(TempString) ;
#endif
      SendBinBlock(d, Sequence++, PING, NULL, 0);
      err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, 250);
     }while ((err != REC_NO_ERROR) && retry++ < 10);


      int IGCCnt =0;

      if (err == REC_NO_ERROR)
      do {
        retry=0;
        do{
           blk[0] =IGCCnt;
           SendBinBlock(d, Sequence++, SELECTRECORD, &blk[0], 1);
           err = RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
        } while ((err== REC_CRC_ERROR) && (retry++ <4));

        if(err != REC_NO_ERROR)
        {
            if (MessageBoxX(MsgToken(2358), MsgToken(2358), mbOk) == IdYes)
            {
                if(deb_)StartupStore(TEXT("EXIT "));
                SendBinBlock(d, Sequence++, EXIT, NULL, 0);
                RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
            }
        }
        else
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
#ifdef PRPGRESS_DLG
            IGCProgressDialogText(TempString) ;
#endif
          }
        }
        IGCCnt++;
     } while (RecCommand == ACK);
      iNoIGCFiles = IGCCnt;
#ifdef PRPGRESS_DLG
     CloseIGCProgressDialog();
#endif
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



     _sntprintf(TempString, 255, _T("%s?"), MsgToken(2360)); // _@M2360_ "Reset FLARM"
    if (MessageBoxX(MsgToken(2360), TempString, mbYesNo) == IdYes)  // _@M2360_ "Reset FLARM"
    {
        if(deb_)StartupStore(TEXT("EXIT "));
        SendBinBlock(d, Sequence++, EXIT, NULL, 0);
        RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize, REC_TIMEOUT);
    }

    UnlockComm();
    StartupStore(TEXT("Leaving IGC dialog "));


    return pIGCResult;
}






class IGCReadThread : public Poco::Runnable
{
public:

    void Start() {
        bStop = false;
        if(!Thread.isRunning())
        {
      //    Thread.setPriority(PRIO_LOW);
       //   Thread.setStackSize( 2000);
          Thread.start(*this);
        }
    }

    void Stop() {

                bStop = true;
          //      Thread.join();
                }
protected:
    void run() {
              //    PeriodClock Timer;
                  while(!bStop) {
                      ReadFlarmIGCFile( CDevFlarm::GetDevice(), IGC_Index);

                   //      unsigned n = Clamp<unsigned>(1000U - Timer.ElapsedUpdate(), 0U, 1000U);

                        //  Sleep(5);
                          Poco::Thread::sleep(20);
                 //         Poco::Thread::Thread::yield();
                      //    Timer.Update();
                  }
                }

                bool bStop;

    Poco::Thread Thread;
};

IGCReadThread IGCReadThreadThreadInstance;

void StartIGCReadThread() {
  ThreadState = OPEN_STATE;
  StartupStore(TEXT("Start Thread !"));
  IGCReadThreadThreadInstance.Start();
}

void StopIGCReadThread() {
  StartupStore(TEXT("Stop Thread !"));
  IGCReadThreadThreadInstance.Stop();
}
