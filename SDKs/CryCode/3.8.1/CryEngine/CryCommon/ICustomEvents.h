#ifndef __ICUSTOMEVENTS_H__
#define __ICUSTOMEVENTS_H__

#pragma once

#define CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE 6

// Represents an event
typedef uint32 TCustomEventId;

// Invalid event id
static const TCustomEventId CUSTOMEVENTID_INVALID = 0;

///////////////////////////////////////////////////
// Custom event listener
///////////////////////////////////////////////////
struct ICustomEventListener
{
	// <interfuscator:shuffle>
	virtual ~ICustomEventListener(){}
	virtual void OnCustomEvent( const TCustomEventId eventId ) = 0;
	// </interfuscator:shuffle>
};

///////////////////////////////////////////////////
// Custom event manager interface
///////////////////////////////////////////////////
struct ICustomEventManager
{
	// <interfuscator:shuffle>
	virtual ~ICustomEventManager(){}

	// Registers custom event listener
	virtual bool RegisterEventListener( ICustomEventListener* pListener, const TCustomEventId eventId ) = 0;
	
	// Unregisters custom event listener
	virtual bool UnregisterEventListener( ICustomEventListener* pListener, const TCustomEventId eventId ) = 0;

	// Unregisters all listeners associated to an event
	virtual bool UnregisterEvent( TCustomEventId eventId ) = 0;

	// Clear event data
	virtual void Clear() = 0;

	// Fires custom event
	virtual void FireEvent( const TCustomEventId eventId ) = 0;

	// Gets next free event id
	virtual TCustomEventId GetNextCustomEventId() = 0;
	// </interfuscator:shuffle>
};

#endif
