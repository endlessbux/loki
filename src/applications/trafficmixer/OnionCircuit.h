#ifndef __ONIONCIRCUIT_H_
#define __ONIONCIRCUIT_H_

#include <vector>
#include <memory>
#include <omnetpp.h>
#include "common/OverSimDefs.h"
#include "common/InitStages.h"
#include "common/NodeHandle.h"
#include "common/CommonMessages_m.h"


class OnionCircuit : public cSimpleModule {
    public:
        OnionCircuit();
        ~OnionCircuit() {};
        void initializeCircuit();

    protected:
        bool sendMessage(cMessage* msg);
        bool relayMessage(cMessage* msg);

    private:
        std::vector<NodeHandle*> relays;
};

#endif
