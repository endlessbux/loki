#ifndef __TRAFFICMIXER_H_
#define __TRAFFICMIXER_H_

#include <omnetpp.h>
#include <vector>
#include <memory>
#include "applications/dht/DHT.h"
#include "common/BaseApp.h"
#include "common/NodeHandle.h"
#include "OnionCircuit.h"
#include "OnionKey.h"
#include "IdentityKey.h"



class DHTDataStorage;


class TrafficMixer : public BaseApp {
    public:
        enum States {
            INIT        = 0,
            AWAIT       = 1,
            BOOTSTRAP   = 2,
            READY       = 3,
            SHUTDOWN    = 4
        };

        // statistics
        int numSent;
        int numRelayed;

        // timers
        cMessage* keepAliveTimer;

        TrafficMixer();
        virtual ~TrafficMixer() {};
        void initializeApp(int stage);
        void finishApp();
        void handleTimerEvent(cMessage* msg);

    protected:
        DHT dht;
        IdentityKey* identityKey;
        OnionKey* onionKey;

        virtual void changeState(int state);

        /**
         * Returns the amount of required peers to start building circuits
         */
        uint getNumRequiredPeers();

        /**
         * Maintains the amount of operational circuits equal to
         * numCircuits value.
         */
        void maintainCircuits();

    private:
        States state;

        size_t poolSize;                       // amount of nodes in the relay pool
        std::vector<NodeHandle*> relayPool; // the relays to be used to build
                                            // circuits

        size_t numCircuits;                        // amount of circuits to be built
        std::vector<OnionCircuit*> circuits;    // built circuits which will be used
                                                // to redirect traffic


        //void handleDataReceived(TransportAddress address, cPacket* msg, bool urgent);
        //void handleConnectionEvent(EvCode code, TransportAddress address);

        /**
         *
         */

        /**
         * Sends a packet to unused circuits to keep them operational
         */
        void keepCircuitsAlive();

        /**
         * Populates the relay pool with the set amount of nodes (poolSize)
         */
        void populatePool();
};


#endif
