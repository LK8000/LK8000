/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: DeviceRegister.cpp
 * Author: Bruno de Lacheisserie
 */

#include "DeviceRegister.h"
#include "Compiler.h"
#include "options.h"
#include "types.h"
#include "Comm/device.h"
#include <cassert>

#include "devDisabled.h"
#include "devGeneric.h"
#include "devCAI302.h"
#include "devEW.h"
#include "devCaiGpsNav.h"
#include "devNmeaOut.h"
#include "devPosiGraph.h"
#include "devBorgeltB50.h"
#include "devVolkslogger.h"
#include "devEWMicroRecorder.h"
#include "devLX.h"
#include "devLXMiniMap.h"
#include "devLX16xx.h"
#include "devVaulter.h"
#include "devLXV7.h"
#include "devLXV7_EXP.h"
#include "devLXNano.h"
#include "devZander.h"
#include "devFlymasterF1.h"
#include "devCompeo.h"
#include "devFlytec.h"
#include "devLK8EX1.h"
#include "devDigifly.h"
#include "devXCOM760.h"
#include "devPVCOM.h"
#include "devCondor.h"
#include "devIlec.h"
#include "devIMI.h"
#include "devWesterboer.h"
#include "devFlyNet.h"
#include "devKRT2.h"
#include "devAR620x.h"
#include "devATR833.h"
#include "devLXNano3.h"
#include "devXCTracer.h"
#include "devGPSBip.h"
#include "devFanet.h"
#include "devCProbe.h"
#include "devFlarm.h"
#include "devBlueFlyVario.h"
#include "devLXV7easy.h"
#include "devOpenVario.h"
#include "devLX_EOS_ERA.h"
#include "devRCFenix.h"
#include "devXCVario.h"
#include "devGenericAutopilot.h"
#include "devAirControlDisplay.h"
#include "devFlyBeeper.h"

namespace {

constexpr const
DeviceRegister_t device_list[] = {
  
    disRegister(), // must be first
    InternalRegister(), // must be second
    genRegister(), // must be three, since we Sort(3) in dlgConfiguration
    cai302Register(),
    ewRegister(),
    CDevCAIGpsNav::Register(),
    NmeaOut::Register(),
    pgRegister(),
    b50Register(),
    vlRegister(),
    ewMicroRecorderRegister(),
    DevLX::Register(),
    DevLXMiniMap::Register(),
    DevLX16xx::Register(),
    DevLXV7::Register(),
    DevLXV7_EXP::Register(),
    DevLXNano::Register(),
    zanderRegister(),
    flymasterf1Register(),
    flymasterGPSRegister(),
    CompeoRegister(),
    xcom760Register(),
    condorRegister(),
    Condor3Register(),
    DigiflyRegister(), // 100209
    IlecRegister(),
    CDevIMI::Register(),
    FlytecRegister(),
    LK8EX1Register(),
    WesterboerRegister(),
    FlyNetRegister(),
    CDevCProbe::Register(),
    CDevFlarm::Register(),
    BlueFlyRegister(),
    LXV7easyRegister(),
    DevLXNanoIII::Register(),
    XCTracerRegister(),
    Stodeus::GPSBipRegister(),
    PVCOMRegister(),
    KRT2Register(),
    AR620xRegister(),
    ATR833Register(),
    DevVaulter::Register(),
    DevOpenVario::Register(),
    DevLX_EOS_ERA::Register(),
    GXAirCom::Register(),
    DevRCFenix::Register(),
    Fanet::Register(),
    XCVario::Register(),
    GenericAutopilot::Register(),
    AirControlDisplay::Register(),
    Stodeus::UltraBipRegister(),
    Stodeus::BlueBipRegister(),
    FlyBeeper::Register(),
};


} // namespace

void DeviceRegister_t::Install(DeviceDescriptor_t* d) const {
  lk::strcpy(d->Name, Name);
  Installer(d);
}


const DeviceRegister_t* devRegisterIterator::begin() {
  return std::begin(device_list);
}

const DeviceRegister_t* devRegisterIterator::end() {
  return std::end(device_list);
}

const DeviceRegister_t* GetRegisteredDevice(const TCHAR* Name) {
  devRegisterIterator it;
  auto pDev = std::find_if(it.begin(), it.end(), [&](auto& d) {
      return (_tcscmp(d.Name, Name) == 0);
    });

  if (pDev != it.end()) {
    return pDev;
  }
  return nullptr;
}
