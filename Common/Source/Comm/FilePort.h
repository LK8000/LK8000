/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   FilePort.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 22, 2016, 8:29 PM
 */

#ifndef FilePort_H
#define FilePort_H

#include "ComPort.h"


#ifdef WIN32
#ifdef PPC2002

#endif
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>


#endif



class FilePort :  public ComPort {
public:

    FilePort(int idx, const tstring& sName);

    ~FilePort();
    
    bool Initialize() override;
    bool Close() override;

    void Flush() override {};
    void Purge() override {};
    void CancelWaitEvent() override {};

    bool IsReady() override {
        return FileStream != nullptr;
    }

    int SetRxTimeout(int TimeOut) override;
    unsigned long SetBaudrate(unsigned long) override { return 0U; }
    unsigned long GetBaudrate() const override {  return 0U; }

    void UpdateStatus() override {return;};

    bool Write(const void *data, size_t length) override;
    size_t Read(void *data, size_t size) override {  return 0; };

    Poco::Event FileStopEvt;
protected:

    unsigned RxThread() override;

    int ReadLine(char *pString, size_t size);

    unsigned long m_dwWaitTime;
    FILE *FileStream;

private:

};

#endif /* FilePort_H */

