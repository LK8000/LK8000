/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LoadGpxTask.cpp
 * Author: Alberto Realis-Luc
 *
 * Created on 20 December 2013, 10:43
 */

#include "externs.h"
#include "Waypointparser.h"
#include "xmlParser.h"
#include "utils/stringext.h"
#include <string>



typedef std::map<std::wstring,WAYPOINT> mapCode2Waypoint_t;

class GpxObsZoneUpdater {
public:

	GpxObsZoneUpdater() :
			mIdx(), mType(), mR1(), mA1(), mR2(), mA2(), mA12(), mLine() {
	}

	size_t mIdx;
	int mType;
	double mR1;
	double mA1;
	double mR2;
	double mA2;
	double mA12;
	bool mLine;

	void UpdateTask() {
		if(mA1==180.0) {
			if(mIdx==0) {
				StartLine=0;
				StartRadius=(DWORD)mR1;
			} else if(mIdx==(size_t)getFinalWaypoint()) {
				FinishLine=0;
				FinishRadius=(DWORD)mR1;
			} else {
				Task[mIdx].AATType= CIRCLE;
				Task[mIdx].AATCircleRadius=mR1;
			}
		} else {

			switch(mType){
				case 0: // - Fixed value,
					if(mLine) {
						StartupStore(_T("..Cup Task : \"Fixed\" LINE Turnpoint is not supported%s"),NEWLINE);
						UpdateFixedLine();
					} else {
						UpdateFixedSector();
					}
					break;
				case 1: // - Symmetrical,
					if(mLine) {
						StartupStore(_T("..Cup Task : \"Symmetrical\" LINE Turnpoint is not supported%s"),NEWLINE);
						UpdateSymLine();
					} else {
						UpdateSymSector();
					}
					break;
				case 2: // - To next point,
					if(mLine) {
						if(mIdx>0) {
							StartupStore(_T("..Cup Task : \"To next point\" LINE Turnpoint is not supported%s"),NEWLINE);
						}
						UpdateToNextLine();
					} else {
						UpdateToNextSector();
					}
					break;
				case 3: // - To previous point,
					if(mLine) {
						if(mIdx<(size_t)getFinalWaypoint()) {
							StartupStore(_T("..Cup Task : \"To previous point\" LINE Turnpoint is not supported%s"),NEWLINE);
						}
						UpdateToPrevLine();
					} else {
						UpdateToPrevSector();
					}
					break;
				case 4: // - To start point
					if(mLine) {
						StartupStore(_T("..Cup Task : \"To start point\" LINE Turnpoint is not supported%s"),NEWLINE);
						UpdateToStartLine();
					} else {
						UpdateToStartSector();
					}
					break;
			}
		}
	}

	void UpdateFixedLine() {
		if((mIdx==0)||(mIdx==(size_t)getFinalWaypoint())) {
			UpdateSymLine();
		} else {
			StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"),NEWLINE);
			mA1=90.0;
			UpdateFixedSector();
		}
	}

	void UpdateFixedSector() {
		if(mIdx==0) {
			StartLine=2;
			StartRadius=(DWORD)mR1;
		} else if(mIdx==(size_t)getFinalWaypoint()) {
			FinishLine=2;
			FinishRadius=(DWORD)mR1;
		} else {
			Task[mIdx].AATType= SECTOR;
			Task[mIdx].AATSectorRadius=mR1;
			Task[mIdx].AATStartRadial=mA12-180.0-(mA1);
			Task[mIdx].AATFinishRadial=mA12-180.0+(mA1);
		}
	}

	void UpdateSymLine() {
		if(mIdx==0) {
			StartLine=1;
			StartRadius=(DWORD)mR1;
		} else if(mIdx==(size_t)getFinalWaypoint()) {
			FinishLine=1;
			FinishRadius=(DWORD)mR1;
		} else {
			StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"),NEWLINE);
			mA1=90.0;
			UpdateSymSector();
		}
	}

	void UpdateSymSector() {
		if(mIdx==0) {
			UpdateToNextSector();
		} else if(mIdx==((size_t)getFinalWaypoint())) {
			UpdateToPrevSector();
		} else {
			const WAYPOINT *CurrPt=TaskWayPoint(mIdx);
			const WAYPOINT *PrevPt=TaskWayPoint(mIdx-1);
			const WAYPOINT *NextPt=TaskWayPoint(mIdx+1);
			double InB=0;
			double OutB=0;
			// bearing to prev
			DistanceBearing(CurrPt->Latitude,CurrPt->Longitude,PrevPt->Latitude,PrevPt->Longitude,NULL,&InB);
			// bearing to next
			DistanceBearing(CurrPt->Latitude,CurrPt->Longitude,NextPt->Latitude,NextPt->Longitude,NULL,&OutB);
			mA12=BiSector(InB,OutB);

			UpdateFixedSector();
		}
	}

	void UpdateToNextLine() {
		if((mIdx==0)||(mIdx==(size_t)getFinalWaypoint())) {
			UpdateSymLine();
		} else {
			StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"),NEWLINE);
			mA1=90.0;
			UpdateToNextSector();
		}
	}

	void UpdateToNextSector() {
		if(ValidTaskPoint(mIdx+1)) {
			const WAYPOINT *CurrPt=TaskWayPoint(mIdx);
			const WAYPOINT *NextPt=TaskWayPoint(mIdx+1);
			// bearing to next
			DistanceBearing(CurrPt->Latitude,CurrPt->Longitude,NextPt->Latitude,NextPt->Longitude,NULL,&mA12);

			UpdateFixedSector();
		}
	}

	void UpdateToPrevLine() {
		if((mIdx==0)||(mIdx==(size_t)getFinalWaypoint())) {
			UpdateSymLine();
		} else {
			StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"),NEWLINE);
			mA1=90.0;
			UpdateToPrevSector();
		}
	}

	void UpdateToPrevSector() {
		if(ValidTaskPoint(mIdx-1)) {
			const WAYPOINT *CurrPt=TaskWayPoint(mIdx);
			const WAYPOINT *PrevPt=TaskWayPoint(mIdx-1);
			// bearing to prev
			DistanceBearing(CurrPt->Latitude,CurrPt->Longitude,PrevPt->Latitude,PrevPt->Longitude,NULL,&mA12);

			UpdateFixedSector();
		}
	}

	void UpdateToStartLine() {
		if((mIdx==0)||(mIdx==(size_t)getFinalWaypoint())) {
			UpdateSymLine();
		} else {
			StartupStore(_T("..Cup Task : LINE Turnpoint is only supported for Start or Finish%s"),NEWLINE);
			mA1=90.0;
			UpdateToStartSector();
		}
	}

	void UpdateToStartSector() {
		if(mIdx>0) {
			const WAYPOINT *CurrPt=TaskWayPoint(mIdx);
			const WAYPOINT *StartPt=TaskWayPoint(0);
			// bearing to prev
			DistanceBearing(CurrPt->Latitude,CurrPt->Longitude,StartPt->Latitude,StartPt->Longitude,NULL,&mA12);

			UpdateFixedSector();
		}
	}
};



//#define GetAttribute(node, name, val) FromString(node.getAttribute(name, 0), val)



bool LoadGpxTask(LPCTSTR szFileName) {
	LockTaskData();
	StartupStore(_T(". LoadTask : <%s>%s"), szFileName, NEWLINE);
	ClearTask();
	FILE* stream = _tfopen(szFileName, TEXT("rb"));
	if(stream) {
		fseek(stream, 0, SEEK_END); // seek to end of file
		long size = ftell(stream); // get current file pointer
		fseek(stream, 0, SEEK_SET); // seek back to beginning of file
		char * buff = (char*) calloc(size + 1, sizeof (char));
		long nRead = fread(buff, sizeof (char), size, stream);
		if(nRead != size) {
			fclose(stream);
			free(buff);
			return false;
		}
		fclose(stream);
		TCHAR * szXML = (TCHAR*) calloc(size + 1, sizeof (TCHAR));
		utf2unicode(buff, szXML, size + 1);
		free(buff);
		XMLNode rootNode = XMLNode::parseString(szXML, _T("lk-task"));
		if(rootNode) {
			if(rootNode.isEmpty()) {
				free(szXML);
				return false;
			}
			//LoadWayPointList(rootNode.getChildNode(_T("waypoints"), 0));
			//if (!LoadTaskPointList(rootNode.getChildNode(_T("taskpoints"), 0))) {
			//	free(szXML);
			//	return false;
			//}
			//if(!LoadStartPointList(rootNode.getChildNode(_T("startpoints"), 0))) {
			//	free(szXML);
			//	return false;
			//}

			//TODO: here we load just the first route may be there are others routes in the GPX file...
			XMLNode routeNode=rootNode.getChildNode(TEXT("rte"));
			if(routeNode.isEmpty()) { //ERROR no route found in GPX file
				free(szXML);
				return false;
			}
			int numWPnodes=routeNode.nChildNode();
			if(numWPnodes<1) { //ERROR no waypoints found in route in GPX file
				free(szXML);
				return false;
			}

			const TCHAR* text=NULL;
			//LPCTSTR szAttr = NULL;


			XMLNode WPnode;
			WAYPOINT newPoint;
			for(int i=0,idx=0;i<numWPnodes;i++) {
				memset(&newPoint, 0, sizeof (newPoint));

				/*
				GetAttribute(node, _T("code"), szAttr);
				if(szAttr) {
					_tcscpy(newPoint.Code, szAttr);
				}
				GetAttribute(node, _T("flags"), newPoint.Flags);
				GetAttribute(node, _T("comment"), szAttr);
				if(szAttr) {
					newPoint.Comment = (TCHAR*) malloc((_tcslen(szAttr) + 1) * sizeof (TCHAR));
					if (newPoint.Comment) {
						_tcscpy(newPoint.Comment, szAttr);
					}
				}
				GetAttribute(node, _T("details"), szAttr);
				if(szAttr) {
					newPoint.Details = (TCHAR*) malloc((_tcslen(szAttr) + 1) * sizeof (TCHAR));
					if(newPoint.Details) {
						_tcscpy(newPoint.Details, szAttr);
					}
				}
				GetAttribute(node, _T("format"), newPoint.Format);
				GetAttribute(node, _T("freq"), szAttr);
				if(szAttr) {
					_tcscpy(newPoint.Freq, szAttr);
				}
				GetAttribute(node, _T("runwayLen"), newPoint.RunwayLen);
				GetAttribute(node, _T("runwayDir"), newPoint.RunwayDir);
				GetAttribute(node, _T("country"), szAttr);
				if(szAttr) {
					_tcscpy(newPoint.Country, szAttr);
				}
				GetAttribute(node, _T("style"), newPoint.Style);
				*/





				/////////////////////


				WPnode=routeNode.getChildNode(i);
				if(_tcscmp(WPnode.getName(),TEXT("rtept"))==0) {
					text=WPnode.getAttribute(TEXT("lat"));
					if(text==NULL) break; //WP without latitude skip it
					newPoint.Latitude=_tcstod(text,NULL);



					text=WPnode.getAttribute(TEXT("lon"));
					if(text==NULL) break; //WP without longitude skip it
					newPoint.Longitude=_tcstod(text,NULL);
					idx++;
					text=WPnode.getAttribute(TEXT("ele"));
					if(text!=NULL) newPoint.Altitude=_tcstod(text,NULL);
					else newPoint.Altitude=0; //WP altitude missing we put it to 0
					text=WPnode.getAttribute(TEXT("name"));
					if(text!=NULL) {
						_tcscpy(newPoint.Name, text);
						//East longitudes are positive according to the GPX standard
					} else {
						//wp with no name
						//newPoint.Name=_tcscpy(newPoint.Name,_T("Unnamed WayPoint"));
					}
					Task[idx].Index = FindOrAddWaypoint(&newPoint);
				}
			}
		} //if(rootNode)
		free(szXML);
	} //if(stream)
	RefreshTask();
	TaskModified = false;
	TargetModified = false;
	_tcscpy(LastTaskFileName, szFileName);
	UnlockTaskData();
	return true;
}
