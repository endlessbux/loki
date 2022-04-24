package oversim.tier2.dhtmediator;

import oversim.tier2.dhttestapp.DHTTestApp;
import oversim.common.ITier;


//
// Simple module of the DHT test application
//
// @author Gregoire Menuel, Ingmar Baumgart
//
simple DHTMediator extends DHTTestApp
{
    parameters:
        @class(DHTMediator);
        testInterval = 0;				// time interval for DHT operations (put/get/modify)
		testTtl = 3600*24*365;  		// TTL for stored test records
        activeNetwInitPhase = false;	// send messages when network is in init phase?
        p2pnsTraffic = false;			// model p2pns application traffic
}


module DHTMediatorModules like ITier
{
    gates:
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input udpIn;    // gate from the UDP layer
        input trace_in;    // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier
        output udpOut;    // gate to the UDP layer

    submodules:
        dhtMediator: DHTMediator;
    connections allowunconnected:
        from_lowerTier --> dhtMediator.from_lowerTier;
        to_lowerTier <-- dhtMediator.to_lowerTier;
        trace_in --> dhtMediator.trace_in;
}

