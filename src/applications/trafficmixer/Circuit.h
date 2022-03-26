#ifndef __CIRCUIT_H_
#define __CIRCUIT_H_

#include <omnetpp.h>
#include <vector>
#include <tuple>
#include "common/OverlayKey.h"
#include "common/NodeHandle.h"
#include "common/BinaryValue.h"


using namespace std;
typedef tuple<OverlayKey, NodeHandle, BinaryValue> relayData;

class Circuit : public cSimpleModule {
    private:
        vector<relayData> relayChain;
        cMessage keepAlive_timer;
};

#endif
