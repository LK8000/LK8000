/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sms.h
 * Author: Shachar Liberman
 *
 * Created on 09.09.2017
 */

package org.LK8000;

import android.telephony.SmsManager;

public class SmsUtil {

    public static boolean sendSMS(String phoneNumbers, String message) {
        String numbers[] = phoneNumbers.split(",");
        SmsManager sms = SmsManager.getDefault();
        for (String number : numbers){
            sms.sendTextMessage(number, null, message, null, null);
        }
        return true;  // (should return messages delivery status.
    }
}