/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   RotateScreen.h
 * Author: Bruno de Lacheisserie
 *
 * Created on April 29, 2015, 9:25 PM
 */

#ifndef ROTATESCREEN_H
#define	ROTATESCREEN_H

enum class DisplayOrientation_t : uint8_t;

/**
 * @brief indicate if screen size or orientation can change
 */
bool CanRotateScreen();

/**
 * @brief rotate device screen or change MainWindow orientation
 * @param rotation angle [ 90 | 180 ]
 * @return // We return true only if we need to perform a ChangeScreen inside LK (not used).
 */
bool RotateScreen(short angle);

class ScopeLockScreen final {
public:
    ScopeLockScreen();
    ~ScopeLockScreen();
private:
    int previous_state;
};

/**
 * read display Orientation in "/boot/config.uEnv"
 */
gcc_pure
DisplayOrientation_t GetScreenOrientation();

#endif	/* ROTATESCREEN_H */

