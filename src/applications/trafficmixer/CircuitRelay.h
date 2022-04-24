#ifndef __CIRCUITRELAY_H_
#define __CIRCUITRELAY_H_

#include <omnetpp.h>
#include <vector>
#include <tuple>
#include "common/OverlayKey.h"
#include "common/NodeHandle.h"
#include "mixerpackets/KeyExchange.h"
#include "mixerpackets/OnionMessage.h"
#include "mixerpackets/CircuitMaintenanceMessages_m.h"
#include "mixerpackets/CircuitBuildingMessages_m.h"
#include "mixerpackets/CircuitEvidence.h"
#include "mixerpackets/UDPMessage_m.h"


using namespace std;
class TrafficMixer;

class CircuitRelay {
    friend class TrafficMixer;
    private:
        TrafficMixer* mix;
        TransportAddress prevNode;
        TransportAddress nextNode;
        CircuitEvidence evidence;
        OverlayKey nextCircuitID;
        string symmetricKey;

    public:
        CircuitRelay() {
            prevNode = TransportAddress::UNSPECIFIED_NODE;
            nextNode = TransportAddress::UNSPECIFIED_NODE;
            symmetricKey = "";
            mix = nullptr;
            nextCircuitID = OverlayKey::UNSPECIFIED_KEY;
        }
        CircuitRelay(TrafficMixer* mix);
        CircuitRelay(TrafficMixer* mix, NodeHandle node,
                     KeyExchange keyExchange, string privateKey);
        CircuitRelay(TrafficMixer* mix, CreateCircuitCall* call, string privateKey);
        CircuitRelay(TrafficMixer* mix, ExtendCircuitCall* call, string privateKey);
        ~CircuitRelay() { };

        NodeHandle getPrevNode() { return prevNode; };
        NodeHandle getNextNode() { return nextNode; };
        void setPrevNode(NodeHandle node) { prevNode = node; };
        void setNextNode(NodeHandle node) { nextNode = node; };
        bool isExitNode() { return nextNode.isUnspecified(); };
        OverlayKey getCircuitID() { return evidence.getCircuitID(); };
        OverlayKey getNextCircuitID() { return nextCircuitID; };

    protected:
        void leave();
        void handleTriggerExtensionCall(TriggerExtensionCall* msg);
        void handleDestroyCircuitCall(DestroyCircuitCall* msg);
        void handleNotifyCircuitResponse(NotifyCircuitResponse* msg);
        void handleOnionMessage(OnionMessage* msg);
        void handleExitResponse(UDPResponse* msg);
        void handleFailure(NodeHandle failedNode);
        void propagateKeepAliveCall();
        void propagateDestroyCircuitCall();
        void propagateRelayDisconnectCall(NodeHandle disconnectedNode);

    private:
        void relayMessage(OnionMessage* msg);
        void wrapAndRelay(cPacket* msg);
        void sendExitRequest(OnionMessage* msg);
        void sendDestroySignal(DestroyCircuitCall* msg);
        OnionMessage* getOnionMessage();
        void printLog(string function);

};

#endif
