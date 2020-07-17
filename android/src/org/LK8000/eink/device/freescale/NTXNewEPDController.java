/* Tested on Tolino Vision2/Shine3 and a Nook Glowlight 3. */

package org.LK8000.eink.device.freescale;

import org.LK8000.eink.device.EPDController;

public class NTXNewEPDController extends NTXEEPDController implements EPDController {
	@Override
	public void setEpdMode(android.view.View targetView, int mode, long delay, int x, int y, int width, int height,
			String epdMode) {
		requestEpdMode(targetView, mode, delay, x, y, width, height);
	}

	@Override
	public String getDefault() {
		return "EPD_A2";
	}
}
