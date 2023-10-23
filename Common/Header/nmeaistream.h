/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef NMEAISTREAM_H_
#define NMEAISTREAM_H_

class nmeastring
{
public:
	inline nmeastring(const char* szString) : string_copy(strdup(szString)) {
        m_szString = string_copy;
	}

	~nmeastring(void) {
        free(string_copy);
	}

	inline char* GetNextString(){
        if(m_Char) {
            *(m_szString-1) = m_Char;
        }
		char* szOut = m_szString;
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
    char *string_copy;
	char *m_szString;
    char m_Char = '\0';
};


#endif /* NMEAISTREAM_H_ */
