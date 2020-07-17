package org.LK8000.eink.device;

import android.view.View;

public class EPDbutton implements View.OnClickListener {

	static final EPDController epd = EPDFactory.getEPDController();

	private String mode;

	public EPDbutton(String mode) {

		this.mode = mode;

	}

	public EPDbutton() {

		this.mode = epd.getDefault();

	}

	@Override
	public void onClick(View v) {

		View rootView = v.getRootView();
		epd.setEpdMode(rootView, 0, 0, 0, 0, 0, 0, this.mode);

	}

	public void setEPDMode(View v) {

		View rootView = v.getRootView();
		epd.setEpdMode(v, 0, 0, 0, 0, 0, 0, epd.getDefault());

	}

}
