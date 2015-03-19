/* 
 * File:   CharUpper.h
 * Author: user
 *
 * Created on July 8, 2014, 11:01 PM
 */

#ifndef CHARUPPER_H
#define	CHARUPPER_H

#include <ctype.h>

inline char* CharUpper(char* lpsz) {
    if(lpsz) {
        for(char* Char = &lpsz[0]; (*Char) != '\0'; ++Char) {
            (*Char) = toupper(*Char);
        }
    }
    return lpsz;
}

inline char* CharLower(char* lpsz) {
    if(lpsz) {
        for(char* Char = &lpsz[0]; (*Char) != '\0'; ++Char) {
            (*Char) = tolower(*Char);
        }
    }
    return lpsz;
}


#endif	/* CHARUPPER_H */

