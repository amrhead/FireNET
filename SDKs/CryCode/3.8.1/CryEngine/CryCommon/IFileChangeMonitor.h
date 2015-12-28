////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2006-2007.
// -------------------------------------------------------------------------
//  File name:   IFileChangeMonitor.h
//  Version:     v1.00
//  Created:     27/07/2007 by Adam Rutkowski
//  Compilers:   Visual Studio.NET 2005
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _IFILECHANGEMONITOR_H_
#define _IFILECHANGEMONITOR_H_
#pragma once

struct IFileChangeListener
{
	enum EChangeType
	{
		//! error or unknown change type
		eChangeType_Unknown,
		//! the file was created
		eChangeType_Created,
		//! the file was deleted
		eChangeType_Deleted,
		//! the file was modified (size changed,write)
		eChangeType_Modified,
		//! this is the old name of a renamed file
		eChangeType_RenamedOldName,
		//! this is the new name of a renamed file
		eChangeType_RenamedNewName
	};

	virtual void OnFileChange(const char* sFilename, EChangeType eType) = 0;
};

struct IFileChangeMonitor
{
	// <interfuscator:shuffle>
	// Register the path of a file or directory to monitor
	// Path is relative to game directory, e.g. "Libs/WoundSystem/" or "Libs/WoundSystem/HitLocations.xml"
	virtual bool RegisterListener(IFileChangeListener *pListener, const char* sMonitorItem) = 0;
	// This function can be used to monitor files of specific type, e.g.
	//   RegisterListener(pListener, "Animations", "caf")
	virtual bool RegisterListener(IFileChangeListener *pListener, const char* sFolder, const char* sExtension) = 0;
	virtual bool UnregisterListener(IFileChangeListener *pListener) = 0;
	// </interfuscator:shuffle>
};

#endif //_IFILECHANGEMONITOR_H_
