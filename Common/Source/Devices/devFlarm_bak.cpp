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

bool deb_ = false;
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


#define BARO__CPROBE		7
extern bool UpdateBaroSource( NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);


void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize);
bool RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize);

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

BOOL CDevFlarm::SetDeviceName( PDeviceDescriptor_t d, const tstring& strName ){
/*	if (d && d->Com && strName.size() <= 15) {
		d->Com->WriteString(TEXT("$PCPILOT,C,SET,"));
		d->Com->WriteString(strName.c_str());
		d->Com->WriteString(TEXT("\r\n"));
		return GetDeviceName(d);
	}*/
	return FALSE;
}

BOOL CDevFlarm::GetFirmwareVersion(PDeviceDescriptor_t d) {
/*    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,GETFW\r\n"));
    }*/
	return TRUE;
}

BOOL CDevFlarm::SetZeroDeltaPressure(PDeviceDescriptor_t d) {
   /* if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,CALZERO\r\n"));
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
	TCHAR Temp[50] = {0};

	LockFlightData();
	NMEA_INFO _INFO = GPS_INFO;
	UnlockFlightData();

	LockDeviceData();
	_stprintf(Temp, TEXT("Flarm - Version: %s"), m_szVersion);
	UnlockDeviceData();

	pWnd->SetCaption(Temp);

	WndProperty* wp;
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpPitch"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.Pitch, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpHeading"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.MagneticHeading, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpRoll"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f%s"), _INFO.Roll, MsgToken(2179));
		wp->SetText(Temp);
	}

	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGx"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelX);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGy"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelY);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpGz"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f"), _INFO.AccelZ);
		wp->SetText(Temp);
	}

	wp = (WndProperty*)pWnd->FindByName(TEXT("prpTemp"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f %sC"), _INFO.OutsideAirTemperature, MsgToken(2179));
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpRh"));
	if(wp){
		_stprintf(Temp, TEXT("%.2f %%"), _INFO.RelativeHumidity);
		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpDeltaPress"));
	if(wp){
		LockDeviceData();
		_stprintf(Temp, TEXT("%.2f Pa"), m_delta_press);
		UnlockDeviceData();

		wp->SetText(Temp);
	}
	wp = (WndProperty*)pWnd->FindByName(TEXT("prpAbsPress"));
	if(wp){
		LockDeviceData();
		_stprintf(Temp, TEXT("%.2f hPa"), m_abs_press/100.);
		UnlockDeviceData();

		wp->SetText(Temp);
	}
}









static void OnEnterClicked(WndButton* pWnd) {

    (void)pWnd;

    if (IGC_Index >= iNoIGCFiles) {
        IGC_Index = iNoIGCFiles - 1;
    }
    ReadFlarmIGCFile( global_d, IGC_Index);


    if (IGC_Index >= 0) {
      if(pWnd) {
        WndForm * pForm = pWnd->GetParentWndForm();
        if(pForm) {
  //        pForm->SetModalResult(mrOK);


        }
      }
    }
}


static void OnCloseClicked(WndButton* pWnd) {

  IGC_Index = -1;
  /*
uint16_t RecSequence;
uint8_t RecCommand;
uint8_t Size;
uint8_t Blk;
  SendBinBlock(d, 100, EXIT, NULL, 0);
  RecBinBlock(d, &RecSequence, &RecCommand, NULL, &Size);
*/

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


int cnt=0;

if(recEnable)
  while (cnt < len)
  {
 //   if(deb_)  {StartupStore(TEXT("< %02x"), (uint8_t)String[cnt]);}
    RingBuff[InCnt++] = (TCHAR) String[cnt++];
    InCnt %= REC_BUFFERSIZE;
  } //  (cnt < len)

return  true;
}



bool RecChar8( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{
  Poco::Thread::sleep(1);
  *inchar = d->Com->GetChar();

  /*
  uint16_t TimeCnt =0;
  uint8_t Tmp;
  while(OutCnt == InCnt)
  {
    Poco::Thread::sleep(5);
    if(TimeCnt++ > Timeout)
      return true;
  }
  Tmp = RingBuff[OutCnt++];
  OutCnt %= REC_BUFFERSIZE;
  if(inchar)
    *inchar = Tmp;
  else
    return true;
*/
 return false;
}





bool RecChar16( DeviceDescriptor_t *d,uint16_t *inchar, uint16_t Timeout)
{
  ConvUnion tmp;
  bool error  = RecChar8(d, &(tmp.byte[0]), Timeout);
       error |= RecChar8(d, &(tmp.byte[1]), Timeout);
  *inchar =  tmp.val;
return error;
}


bool RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize)
{

bool error  = false;
uint8_t inchar;
uint8_t Version;
uint16_t CRC_in, CRC_calc=0;

#define REC_TIMEOUT 5000
  do{
    error = RecChar8(d, &inchar, REC_TIMEOUT);
    if(error)
    {
        { if(deb_)StartupStore(TEXT("STARTFRAME timeout!" )); }
      if(error) return error;
    }
  } while( (inchar != STARTFRAME) && !error);

  if(!error)
    { if(deb_ )StartupStore(TEXT("STARTFRAME timeout!"));}
  else
    { if(deb_)StartupStore(TEXT("STARTFRAME fail!" )); }

  error |= RecChar16(d, blocksize , REC_TIMEOUT); CRC_calc = crc_update16 (CRC_calc,*blocksize);{ if(deb_)StartupStore(TEXT("Block Size %u"  ), *blocksize); }
  error |= RecChar8(d, &Version   , REC_TIMEOUT); CRC_calc = crc_update   (CRC_calc,Version)   ;{ if(deb_)StartupStore(TEXT("Block Ver %u"   ), Version);    }
  error |= RecChar16(d, Sequence  , REC_TIMEOUT); CRC_calc = crc_update16 (CRC_calc,*Sequence) ;{ if(deb_)StartupStore(TEXT("Block Seq %u"   ), *Sequence);  }
  error |= RecChar8(d, Command    , REC_TIMEOUT); CRC_calc = crc_update   (CRC_calc,*Command)  ;{ if(deb_)StartupStore(TEXT("Block Cmd %02X" ), *Command);   }
  error |= RecChar16(d,&CRC_in   , REC_TIMEOUT);                                               { if(deb_)StartupStore(TEXT("Block CRC %04X" ), CRC_in);     }
  if(deb_) {StartupStore(TEXT("Header  received!" ));}
  if(*blocksize > 8)
    for (uint16_t i = 0 ; i < (*blocksize-8) ; i++)
    {
       error |= RecChar8(d,&inchar   , REC_TIMEOUT);
       if(inchar == ESCAPE)
       {
         error |= RecChar8(d,&inchar   , REC_TIMEOUT);
         if(inchar == ESC_ESC)
         {
           pBlock[i] = 0x78;
    //       StartupStore(TEXT("ESC_ESC received!"));
         }
         if(inchar == ESC_START)
         {
           pBlock[i] = 0x73;
    //       StartupStore(TEXT("ESC_START received!"));
         }

       }
       else
         pBlock[i] = inchar;
       CRC_calc = crc_update (CRC_calc,pBlock[i]);
   //    if(deb_) { StartupStore(TEXT("Block[%u]  %02X" ),i, pBlock[i]);}
    }
*blocksize -= 8;
 if(CRC_calc != CRC_in)
 {
   {StartupStore(TEXT("Rec Block CRC error!" ));}
   error = true;
 }
 else
  if(error)
    {StartupStore(TEXT("Rec Block error  received!" ));}
  else
    {if(deb_) StartupStore(TEXT("Rec Block received OK!" ));}
  return error;
}





ListElement* dlgIGCSelectListShowModal( DeviceDescriptor_t *d) {


  d->Com->StopRxThread();
  d->Com->SetRxTimeout(10000);                     // set RX timeout to 50[ms]
  LockComm();

    IGC_Index = -1;
    iNoIGCFiles = 8;
    uint8_t blk[10];
    uint16_t RecSequence;
    uint8_t RecCommand;
    uint8_t pBlock[1024];
    uint16_t blocksize;

    if (d && d->Com)
    {
        global_d = d;
      d->Com->WriteString(TEXT("$PFLAX\r"));  // set to binary
      if(deb_)   StartupStore(TEXT("$PFLAX\r "));

      recEnable = true;


      if(deb_)StartupStore(TEXT("PING "));
      SendBinBlock(d, Sequence++, PING, NULL, 0);
      RecBinBlock(d, &RecSequence, &RecCommand, &pBlock[0], &blocksize);


  //    bool RecBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize)

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
    UnlockComm();
    StartupStore(TEXT("Leaving IGC dialog "));
    d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
    d->Com->StartRxThread();                       // restart RX thread


    return pIGCResult;
}
