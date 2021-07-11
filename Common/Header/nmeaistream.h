/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef NMEAISTREAM_H_
#define NMEAISTREAM_H_

class tnmeastring
{
public:
	inline tnmeastring(TCHAR* szString) : m_szString(szString), m_Char() {
	}

	~tnmeastring(void) {
        if(m_Char) {
            *(m_szString-1) = m_Char;
        }
	}

	inline TCHAR* GetNextString(){
        if(m_Char) {
            *(m_szString-1) = m_Char;
        }
		TCHAR* szOut = m_szString;
		if(*(m_szString)) {
			while((*m_szString != 0) && (*m_szString != 10) && (*m_szString != L',')) {
                ++m_szString;
            }
            m_Char = *(m_szString);
			*(m_szString++) = 0;
		}
		return szOut;
	}

protected:
	TCHAR *m_szString;
    TCHAR m_Char;
};


#endif /* NMEAISTREAM_H_ */
