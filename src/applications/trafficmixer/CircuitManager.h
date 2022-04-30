#ifndef __CIRCUITMANAGER_H_
#define __CIRCUITMANAGER_H_

#include "common/NodeHandle.h"
#include "applications/trafficmixer/mixerpackets/OnionMessage.h"
#include "applications/trafficmixer/mixerpackets/Certificate.h"
#include "applications/trafficmixer/mixerpackets/CircuitMaintenanceMessages_m.h"
#include "applications/trafficmixer/mixerpackets/CircuitBuildingMessages_m.h"
#include "applications/trafficmixer/mixerpackets/UDPMessage_m.h"



using namespace std;
class TrafficMixer;
typedef tuple<OverlayKey, TransportAddress, string> RelayInfo;


class CircuitManager : public cObject {
    friend class TrafficMixer;

    private:
        TrafficMixer* mix;
        list<OverlayKey> circuitOrder;
        set<TransportAddress> involvedNodes;
        pair<TransportAddress, string> pendingRelay;
        map<OverlayKey, RelayInfo> circuitData;

    public:
        CircuitManager();
        CircuitManager(TrafficMixer* mix);
        ~CircuitManager() { }

        int size() { return circuitOrder.size(); }
        string str() const {
            string path = "";
            bool isFirst = true;
            for(TransportAddress node : involvedNodes) {
                if(isFirst) {
                    isFirst = false;
                } else {
                    path += " --> ";
                }
                path += node.getIp().str();
            }
            return path;
        }

        friend ostream& operator<<(ostream& stream,
                                   const CircuitManager& manager);

    protected:
        uint32_t sendRequest(cPacket* msg);
        void create(NodeHandle entryNode, Certificate cert, string key);
        void extend(NodeHandle nextNode, Certificate cert, string key);
        void keepAlive();
        void destroy();
        void handleFailure(NodeHandle failedNode);
        bool handleBuildCircuitResponse(BuildCircuitResponse* msg);
        cPacket* unwrapPayload(OnionMessage* msg);
        void handleUDPResponse(UDPResponse* response);

    private:
        cPacket* peelOnionMessage(OnionMessage* msg);
        OnionMessage* wrapPacket(cPacket* msg);
        OverlayKey getCircuitIDAtPosition(int position);
        RelayInfo getRelayInfoAtPosition(int position);
        NodeHandle getNodeAtPosition(int position);
        NodeHandle getNodeFromOverlayKey(OverlayKey key);
        NodeHandle getEntryNode();
        string getKeyFromCircuitID(OverlayKey circuitID);
        string getKeyFromOnionMessage(OnionMessage* msg);
        void printLog(string functionName);
};

inline ostream& operator<<(ostream& stream,
                    const CircuitManager& manager) {
    stream << manager.str();
    return stream;
}

#endif
