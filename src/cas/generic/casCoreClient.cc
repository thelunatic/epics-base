/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/*
 *      $Id$
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 */

#define epicsExportSharedSymbols
#include "casCoreClient.h"
#include "casAsyncPVExistIOI.h"
#include "casAsyncPVAttachIOI.h"

casCoreClient::casCoreClient ( caServerI & serverInternal ) :
    eventSys ( *this )
{
	assert ( & serverInternal );
	ctx.setServer ( & serverInternal );
	ctx.setClient ( this );
}
 
casCoreClient::~casCoreClient()
{
    // only used by io that does not have a channel
	while ( casAsyncIOI * pIO = this->ioList.get() ) {
        pIO->removeFromEventQueue ();
		delete pIO;
	}
    if ( this->ctx.getServer()->getDebugLevel() > 0u ) {
		errlogPrintf ( "CAS: Connection Terminated\n" );
    }

    // this will clean up the event queue because all 
    // channels have been deleted and any events left on 
    // the queue are there because they are going to
    // execute a subscription delete
    {
        epicsGuard < casClientMutex > guard ( this->mutex );
        this->eventSys.process ( guard );
    }
}

void casCoreClient::show ( unsigned level ) const
{
	printf ( "Core client\n" );
    epicsGuard < epicsMutex > guard ( this->mutex );
	this->eventSys.show ( level );
	this->ctx.show ( level );
    this->mutex.show ( level );
}

//
// one of these for each CA request type that has
// asynchronous completion
//
caStatus casCoreClient::asyncSearchResponse (
    epicsGuard < casClientMutex > &, const caNetAddr &, 
    const caHdrLargeArray &, const pvExistReturn &,
    ca_uint16_t, ca_uint32_t )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::createChanResponse (         
    epicsGuard < casClientMutex > &,
    const caHdrLargeArray &, const pvAttachReturn & )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::readResponse ( 
    epicsGuard < casClientMutex > &, casChannelI *, 
    const caHdrLargeArray &, const gdd &, const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::readNotifyResponse ( 
    epicsGuard < casClientMutex > &, casChannelI *, 
    const caHdrLargeArray &, const gdd &, const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::writeResponse ( 
    epicsGuard < casClientMutex > &, casChannelI &, 
    const caHdrLargeArray &, const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::writeNotifyResponse ( 
    epicsGuard < casClientMutex > &, casChannelI &, 
    const caHdrLargeArray &, const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::monitorResponse ( 
    epicsGuard < casClientMutex > &, casChannelI &, 
    const caHdrLargeArray &, const gdd &, const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::accessRightsResponse ( 
    epicsGuard < casClientMutex > &, casChannelI * )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::enumPostponedCreateChanResponse ( 
    epicsGuard < casClientMutex > &, casChannelI &, 
    const caHdrLargeArray &, unsigned )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::channelCreateFailedResp ( 
    epicsGuard < casClientMutex > &, const caHdrLargeArray &, 
    const caStatus )
{
	return S_casApp_noSupport;
}
caStatus casCoreClient::channelDestroyEvent ( 
    epicsGuard < casClientMutex > &, 
    casChannelI * const, ca_uint32_t )
{
	return S_casApp_noSupport;
}

void casCoreClient::casChannelDestroyNotify ( 
    casChannelI &, bool immediatedSestroyNeeded )
{
    assert ( 0 );
}

caNetAddr casCoreClient::fetchLastRecvAddr () const
{
	return caNetAddr(); // sets addr type to UDF
}

ca_uint32_t casCoreClient::datagramSequenceNumber () const
{
	return 0;
}

ca_uint16_t casCoreClient::protocolRevision() const
{
    return 0;
}

// this is a pure virtual function, but we nevertheless need a  
// noop to be called if they post events when a channel is being 
// destroyed when we are in the casStrmClient destructor
void casCoreClient::eventSignal()
{
}

caStatus casCoreClient::casMonitorCallBack ( 
    epicsGuard < casClientMutex > &, casMonitor &, const gdd & )
{
    return S_cas_internal;
}

casMonitor & casCoreClient::monitorFactory ( 
    casChannelI & chan,
    caResId clientId, 
    const unsigned long count, 
    const unsigned type, 
    const casEventMask & mask )
{
    casMonitor & mon = this->ctx.getServer()->casMonitorFactory ( 
            chan, clientId, count, type, mask, *this );
    this->eventSys.installMonitor ();
    return mon;
}

void casCoreClient::destroyMonitor ( casMonitor & mon )
{
    this->eventSys.removeMonitor ();
    assert ( mon.numEventsQueued() == 0 );
    this->ctx.getServer()->casMonitorDestroy ( mon );
}

void casCoreClient::installAsynchIO ( casAsyncPVAttachIOI & io )
{
    epicsGuard < epicsMutex > guard ( this->mutex );
    this->ioList.add ( io );
}

void casCoreClient::uninstallAsynchIO ( casAsyncPVAttachIOI & io )
{
    epicsGuard < epicsMutex > guard ( this->mutex );
    this->ioList.remove ( io );
}

void casCoreClient::installAsynchIO ( casAsyncPVExistIOI & io )
{
    epicsGuard < epicsMutex > guard ( this->mutex );
    this->ioList.add ( io );
}

void casCoreClient::uninstallAsynchIO ( casAsyncPVExistIOI & io )
{
    epicsGuard < epicsMutex > guard ( this->mutex );
    this->ioList.remove ( io );
}
