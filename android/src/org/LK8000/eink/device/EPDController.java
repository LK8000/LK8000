/* generic EPD Controller for Android devices,
 * based on https://github.com/unwmun/refreshU */

package org.LK8000.eink.device;

public interface EPDController {

	void setEpdMode(android.view.View targetView, int mode, long delay, int x, int y, int width, int height,
			String epdMode);

	public String getDefault();
}
