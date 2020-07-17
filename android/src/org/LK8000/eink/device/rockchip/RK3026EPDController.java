/* generic EPD Controller for Android devices,
 * based on https://github.com/unwmun/refreshU */

package org.LK8000.eink.device.rockchip;

import org.LK8000.eink.device.EPDController;

public class RK3026EPDController extends RK30xxEPDController implements EPDController {
	@Override
	public void setEpdMode(android.view.View targetView, int mode, long delay, int x, int y, int width, int height,
			String epdMode) {
		requestEpdMode(targetView, epdMode, true);
	}

	@Override
	public String getDefault() {
		return DEFAULT;
	}
}