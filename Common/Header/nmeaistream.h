/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef NMEAISTREAM_H_
#define NMEAISTREAM_H_

class wnmeastring
{
public:
	inline wnmeastring(wchar_t* szString) : m_szString(szString), m_Char() {
	}

	~wnmeastring(void) {
        if(m_Char) {
            *(m_szString-1) = m_Char;
        }
	}

	inline wchar_t* GetNextString(){
        if(m_Char) {
            *(m_szString-1) = m_Char;
        }
		wchar_t* szOut = m_szString;
		if(*(m_szString)) {
			for( ;(*m_szString != 0) && (*m_szString != 10) && (*m_szString != L','); ++m_szString);
            m_Char = *(m_szString);
			*(m_szString++) = 0;
		}
		return szOut;
	}

protected:
	wchar_t *m_szString;
    wchar_t m_Char;
};


#endif /* NMEAISTREAM_H_ */
