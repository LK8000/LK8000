/************************************************************************
*
* File:			utm.h
* RCS:			$Header: /cvsroot/stelvio/stelvio/NavStar/Utm.h,v 1.2 2002/04/23 05:02:00 steve_l Exp $
* Author:		Steve Loughran
* Created:		2001
* Language:		C++
* Package:		
* Status:		Experimental
* @doc
*
************************************************************************/
#pragma once

//=======================================================================
/**
 * UTM support goes here
 */
//=======================================================================
class CUtmPoint 
{
	protected:
	double m_easting;
	double m_northing;
	int m_xzone;
	char m_yzone;
public:

//=======================================================================
//=======================================================================
	CUtmPoint()
	{Clear();}

//=======================================================================
//=======================================================================
CUtmPoint(const CPosition &p)
	{
	FromPosition(p);
	}

//=======================================================================
//=======================================================================
CUtmPoint(const CUtmPoint& that)
	{
	m_easting=that.m_easting;
	m_northing=that.m_northing;
	m_xzone=that.m_xzone;
	m_yzone=that.m_yzone;
	}

//=======================================================================
//=======================================================================
void Clear() 
	{
	m_easting=m_northing=0;
	m_xzone=0;
	m_yzone=0;
	}

//=======================================================================
/**
@func Build a position string 
@parm	target. must be 30 characters or longer.
*/
//=======================================================================

void GetString(TCHAR *position) const;

//=======================================================================
/**
@func get the position of a UTM point
@parm	point out
*/
//=======================================================================

void ToPosition(CPosition &pos) const;

//=======================================================================
/**
@func turn a position into a UTM point
@parm position
@rdesc	true if it was in range
*/
//=======================================================================

bool FromPosition(const CPosition &pos);

//=======================================================================
/**
range test
*/
//=======================================================================

static bool IsPositionInUtmSpace(const CPosition &pos)
	{ return pos.GetLatitude()<=84 && pos.GetLatitude()>=-80;}



};


