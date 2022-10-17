package org.LK8000.Air3;

import android.content.Context;
import android.os.PowerManager;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Objects;

public class PowerManagerUtils {
    private static final String CLASS = "android.os.PowerManager";
    private static final String UART_POWER_OPEN = "modulePower";

    public static final Integer FANET_MODULE = 2;

    private static PowerManager getPowerManager(Context context) throws NullPointerException {
        return Objects.requireNonNull((PowerManager) context.getSystemService(Context.POWER_SERVICE));
    }

    private static void invoke(PowerManager powerManager, Integer i, Boolean b) throws ClassNotFoundException, NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Method method = Class.forName(CLASS).getMethod(UART_POWER_OPEN, Integer.TYPE, Boolean.TYPE);
        method.invoke(powerManager, i, b);
    }

    private static void close(PowerManager powerManager, Integer i) {
        try {
            invoke(powerManager, i, Boolean.FALSE);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static boolean open(PowerManager powerManager, Integer i) {
        try {
            invoke(powerManager, i, Boolean.TRUE);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public static boolean openModuleFanet(Context context) {
        PowerManager pm = PowerManagerUtils.getPowerManager(context);
        return PowerManagerUtils.open(pm, PowerManagerUtils.FANET_MODULE);
    }

    public static void closeModuleFanet(Context context) {
        PowerManager pm = PowerManagerUtils.getPowerManager(context);
        PowerManagerUtils.close(pm, PowerManagerUtils.FANET_MODULE);
    }
}
