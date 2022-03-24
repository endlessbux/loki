#ifndef __TRAFFICMIXER_H_
#define __TRAFFICMIXER_H_

#include <omnetpp.h>
#include <vector>
#include <memory>
#include "tier2/dhtmediator/DHTMediator.h"
#include "common/BaseApp.h"
#include "common/NodeHandle.h"



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


        TrafficMixer();
        virtual ~TrafficMixer() {};
        void initializeApp(int stage);
        void finishApp();
        void handleTimerEvent(cMessage* msg);

    protected:

        virtual void changeState(int state);

    private:
        DHTMediator dht;

};


#endif
