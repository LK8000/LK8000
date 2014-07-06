/*	DEMO SCREENSHOT
	// **** LINE ZERO

	SelectObject(hdc, LK8PanelMediumFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	_stprintf(Buffer,_T("1-3 cruise"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
 	TextInBox(hdc, Buffer, qcolumn[0],qrow[0], 0, TextDisplayMode, false);

	SelectObject(hdc, LK8PanelMediumFont);
	_stprintf(Buffer,_T("Calcinate"));
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	TextDisplayMode.AsFlag.AlligneCenter = 1;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[1], 0, TextDisplayMode, false);

	SelectObject(hdc, LK8PanelMediumFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer,_T("15:04:26"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 1; // outlined 
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[0], 0, TextDisplayMode, false);

	// **** LINE 1  qrow: 3,4,2 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("185.2"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[3], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("Km"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 0;
	// 6: differenza tra altezza char Big e Small, /2
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[4], 0, TextDisplayMode, false);
	_stprintf(Buffer, TEXT("Distance"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[2], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	//_stprintf(Buffer, TEXT("%s123%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2179_")));
	_stprintf(Buffer, TEXT("123%s%s"), gettext(_T("_@M2179_")),gettext(_T("_@M2183_")));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[9],qrow[3], 0, TextDisplayMode, false); 
	_stprintf(Buffer, TEXT("TO"));
	SelectObject(hdc, LK8PanelSmallFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[2], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("32.7"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[3], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("Req.EFF"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[2], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("41"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[3], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("Avr.EFF"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[2], 0, TextDisplayMode, false); 

	// **** LINE 2
	

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("+5679"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[6], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("ft"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 0;
	// 6: differenza tra altezza char Big e Small, /2
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[7], 0, TextDisplayMode, false);
	_stprintf(Buffer, TEXT("Alt.Arriv"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[5], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	//_stprintf(Buffer, TEXT("%s123%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2179_")));
	_stprintf(Buffer, TEXT("056%s"), gettext(_T("_@M2179_")));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[9],qrow[6], 0, TextDisplayMode, false); 
	_stprintf(Buffer, TEXT("Bearing"));
	SelectObject(hdc, LK8PanelSmallFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[5], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("23"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[6], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("Inst.EFF"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[5], 0, TextDisplayMode, false); 

	// **** LINE 3

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("10402"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[9], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("ft"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 0;
	// 6: differenza tra altezza char Big e Small, /2
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[10], 0, TextDisplayMode, false);
	_stprintf(Buffer, TEXT("Altitude"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[8], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	//_stprintf(Buffer, TEXT("%s123%s"), gettext(_T("_@M2182_")), gettext(_T("_@M2179_")));
	_stprintf(Buffer, TEXT("251%s"), gettext(_T("_@M2179_")));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[9],qrow[9], 0, TextDisplayMode, false); 
	_stprintf(Buffer, TEXT("Track"));
	SelectObject(hdc, LK8PanelSmallFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[8], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("148"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[9], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("Km/h"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 0;
	// 6: differenza tra altezza char Big e Small, /2
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[10], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("GrSpeed"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[8], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("104"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[9], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("FL"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[8], 0, TextDisplayMode, false); 

	// ***** LINE 4

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer,TEXT("268")_T(DEG)_T("/14"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[5],qrow[12], 0, TextDisplayMode, false);
	_stprintf(Buffer, TEXT("Wind"));
	SelectObject(hdc, LK8PanelSmallFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[4],qrow[11], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("2.3"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[12], 0, TextDisplayMode, false); 
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("m/s"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[9],qrow[13], 0, TextDisplayMode, false);
	_stprintf(Buffer, TEXT("Thermal"));
	SelectObject(hdc, LK8PanelSmallFont);
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[8],qrow[11], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("0.9"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[12], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("m/s"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 0;
	TextDisplayMode.AsFlag.WhiteBold = 0;
	// 6: differenza tra altezza char Big e Small, /2
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[13], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("AvTherm"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[12],qrow[11], 0, TextDisplayMode, false); 

	SelectObject(hdc, LK8PanelBigFont);
	TextDisplayMode.AsFlag.Color = TEXTWHITE;
	_stprintf(Buffer, TEXT("1.2"));
	TextDisplayMode.AsFlag.AlligneCenter = 0;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.WhiteBold = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[12], 0, TextDisplayMode, false);
	SelectObject(hdc, LK8PanelSmallFont);
	_stprintf(Buffer, TEXT("MacCready"));
	TextDisplayMode.AsFlag.Color = TEXTLIGHTGREEN;
	TextDisplayMode.AsFlag.AlligneRight = 1;
	TextDisplayMode.AsFlag.AlligneCenter = 0;
 	TextInBox(hdc, Buffer, qcolumn[16],qrow[11], 0, TextDisplayMode, false); 
*/
