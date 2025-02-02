#ifndef __TRAFFICMIXER_H_
#define __TRAFFICMIXER_H_

#include <omnetpp.h>
#include <vector>
#include <memory>
#include <iterator>
#include "tier2/dhtmediator/DHTMediator.h"
#include "tier2/dhtmediator/DHTOperationMessages_m.h"
#include "common/BaseApp.h"
#include "common/NodeHandle.h"
#include "common/OverlayAccess.h"
#include "overlay/permissionedchord/PermissionedChord.h"
#include "overlay/permissionedchord/ChordMessage_m.h"
#include "mixerpackets/Certificate.h"
#include "mixerpackets/CircuitEvidence.h"
#include "mixerpackets/OnionMessage.h"
#include "mixerpackets/UDPMessage_m.h"
#include "applications/trafficmixer/CircuitRelay.h"
#include "applications/trafficmixer/CircuitManager.h"
#include "TargetServer.h"
#include "TrustedAuthority.h"
#include "mixerpackets/TrafficReport_m.h"
#include "common/GlobalStatistics.h"
#include "applications/dht/DHTDataStorage.h"
#include "applications/dht/DHT.h"


using loki::PermissionedChord;
using loki::JoinCall;
using namespace std;

//typedef tuple<OverlayKey, TransportAddress, simtime_t> TrafficRecord;


class TrafficMixer : public BaseApp {
    friend class CircuitRelay;
    friend class CircuitManager;

    public:

        enum States {
            INIT        = 0,
            SCOUTING    = 1,
            READY       = 2,
            SHUTDOWN    = 3
        };
        int state;
        int applicationPort;


        TrafficMixer();
        virtual ~TrafficMixer();
        void initializeApp(int stage);
        void finishApp();
        void handleTimerEvent(cMessage* msg);
        void sendRequestToTarget();
        void buildCircuit();
        void extendCircuit();
        void buildNewCircuit();
        void sendThroughUDP(cPacket* msg, const NodeHandle& address);

        Certificate getOwnCertificate() {
            return ((PermissionedChord*)OverlayAccess().get(this))->cert;
        }

    protected:

        virtual void changeState(int toState);

        uint32_t sendUdpRpcCall(TransportAddress dest,
                                BaseCallMessage* msg,
                                cObject* context = NULL,
                                simtime_t timeout = -1,
                                int retries = 0, int rpcId = -1,
                                RpcListener* rpcListener = NULL);

        void sendExitRequest(OverlayKey circuitID, UDPCall* udpCall);

        void handleRequestTimeout(cMessage* msg);

        void internalHandleRpcMessage(BaseRpcMessage* msg) override;

        virtual void internalHandleRpcTimeout(
                BaseCallMessage* msg,
                const TransportAddress& dest,
                cObject* context,
                int rpcId,
                const OverlayKey& destKey
        ) override;

        void internalHandleResponseMessage(BaseResponseMessage* msg, StoredCall retrievedCall);

        virtual void handleReadyMessage(CompReadyMessage* msg) override;

        virtual bool handleRpcCall(BaseCallMessage* msg) override;

        virtual void handleRpcResponse(BaseResponseMessage* msg, const RpcState& state,
                                       simtime_t rtt) override;

        virtual void deliver(OverlayKey& key, cMessage* msg) override;

        virtual void handleUDPMessage(cMessage* msg) override;

        virtual void handleDataReceived(TransportAddress address, cPacket* msg, bool urgent) override;

    private:
        bool isOwnCertificateShared;
        int certStorageRetries;     // amount of retries for storing the Certificate on the DHT
        int maxCertRetries;         // maximum amount of RegistrationCall retries
        size_t maxPoolSize;         // maximum size of the relay pool
        size_t maxOpenCircuits;     // maximum amount of parallel open circuits
        size_t circuitLength;       // maximum length of each circuit
        size_t maxRequestRetries;   // maximum amount of retries for OnionMessage requests
        simtime_t timeoutLimit;     // amount of time before an anonymised request times out

        map<int, StoredCall> pendingRpcCalls;
        CircuitManager pendingCircuit;
        vector<NodeHandle> selectedNodes;
        bool isCircuitBuilding;

        simtime_t circuitBuildingBeginTime;
        simtime_t overlayReadyTime;

        set<NodeHandle> addressPool;
        map<NodeHandle, Certificate> relayPool;
        map<OverlayKey, CircuitManager> ownCircuits;
        map<OverlayKey, CircuitRelay*> extCircuits;
        map<int, OverlayKey> pendingUDPRequests;
        map<int, simtime_t> pendingOnionRequestsTimes;
        map<int, OverlayKey> pendingOnionRequestsCircuits;
        vector<TrafficRecord> exitTrafficHistory;

        // statistics
        GlobalStatistics* globalStatistics;
        int numOnionSent;
        int bytesOnionSent;
        int numOnionReceived;
        int bytesOnionReceived;
        int numOnionRelayed;
        int bytesOnionRelayed;
        int numExitRequests;
        int bytesExitRequests;
        int numExitResponse;
        int bytesExitResponse;
        int numTargetSent;
        int bytesTargetSent;
        int bytesTargetReceived;
        int numTargetSuccess;
        int numTargetFailure;
        int numBuildCircuitSent;
        int bytesBuildCircuitSent;
        int numBuildCircuitReceived;
        int bytesBuildCircuitReceived;
        int numCreateSuccess;
        int numCreateFailure;
        int numExtendSuccess;
        int numExtendFailure;

        // timers
        cMessage* peerLookupTimer;
        cMessage* requestTimer;
        cMessage* buildCircuitTimer;
        cMessage* trafficReportTimer;
        set<cMessage*> requestsTimeoutTimers;
        cOutVector relayPoolSize;
        cOutVector onionsRelayed;
        cOutVector circuitBuildingTime;
        cOutVector requestLatencies;
        cOutVector ownCircuitsLengths;


        // Responses from DHTMediator:  GetEvidenceResponse, GetCertificateResponse,
        //                              PutEvidenceResponse, PutCertificateResponse
        void handleGetEvidenceResponse(GetEvidenceResponse* msg, GetEvidenceCall* call);

        void handleGetCertificateResponse(GetCertificateResponse* msg, GetCertificateCall* call);

        void handlePutEvidenceResponse(PutEvidenceResponse* msg, PutEvidenceCall* call);

        void handlePutCertificateResponse(PutCertificateResponse* msg, PutCertificateCall* call);


        // Calls from TrafficMixer:     CreateCircuitCall, TriggerExtensionCall,
        //                              ExtendCircuitCall, OnionMessage
        void handleCreateCircuitCall(CreateCircuitCall* msg);

        void handleExtendCircuitCall(ExtendCircuitCall* msg);

        void handleOnionMessage(OnionMessage* msg);

        void handleKeepAliveCall(KeepAliveCall* msg);

        void handleRelayDisconnectCall(RelayDisconnectCall* msg);

        void handleDestroyCircuitCall(DestroyCircuitCall* msg);


        // Responses from TrafficMixer
        void handleNotifyCircuitResponse(NotifyCircuitResponse* msg);

        void handleKeepAliveResponse(KeepAliveResponse* msg);

        void handleOnionResponse(cPacket* msg, OverlayKey circuitID);

        void handleBuildCircuitResponse(BuildCircuitResponse* msg, OverlayKey circuitID);

        void handleUDPResponse(UDPResponse* msg);


        // Timeouts from TrafficMixer:  CreateCircuitCall, ExtendCircuitCall,
        //                              OnionMessage


        // Calls from TrustedAuthority
        void handleGetEvidenceCall(GetEvidenceCall* msg);


        // Procedures
        void getRandomPeerCertificate();

        void putEvidence(CircuitEvidence evidence);

        void putCertificate(Certificate cert, NodeHandle handle);

        void getEvidence(OverlayKey circuitID);

        void getCertificate(NodeHandle handle);

        bool isOwnCertificateAvailable() {
            return getOwnCertificate().isValid() && isOwnCertificateShared;
        }

        void storeOwnCertificate() {
            putCertificate(getOwnCertificate(), thisNode);
        }

        bool isNoncePending(int nonce) {
            return pendingRpcCalls.find(nonce) != pendingRpcCalls.end();
        }

        void putCallInPendingList(BaseCallMessage* call, int nonce) {
            CallType type;
            BaseCallMessage* storedMessage;
            RPC_SWITCH_START(call)
            RPC_ON_CALL(PutCertificate) {
                type = PUT_CERTIFICATE;
                storedMessage = _PutCertificateCall->dup();
            }
            RPC_ON_CALL(GetCertificate) {
                type = GET_CERTIFICATE;
                storedMessage = _GetCertificateCall->dup();
            }
            RPC_ON_CALL(PutEvidence) {
                type = PUT_EVIDENCE;
                storedMessage = _PutEvidenceCall->dup();
            }
            RPC_ON_CALL(GetEvidence) {
                type = GET_EVIDENCE;
                storedMessage = _GetEvidenceCall->dup();
            }
            RPC_SWITCH_END()

            StoredCall record(type, storedMessage);
            pendingRpcCalls[nonce] = record;
        }

        StoredCall getCallFromResponse(BaseResponseMessage* response) {
            printLog("getCallFromResponse");
            int nonce = response->getNonce();
            EV << "    Received nonce: " << nonce << endl;

            if(isNoncePending(nonce)) {
                StoredCall retrievedCall = pendingRpcCalls[nonce];
                pendingRpcCalls.erase(nonce);
                return retrievedCall;
            }
            StoredCall noMatch(UNDEFINED, nullptr);
            return noMatch;
        }

        void addRelayToPool(NodeHandle handle, Certificate cert) {
            handle.setPort(applicationPort);
            if(relayPool.size() >= maxPoolSize) {
                // remove random relay from pool and add this one instead
                removeRandomRelay();
                relayPool[handle] = cert;
            } else {
                relayPool[handle] = cert;
                cancelEvent(buildCircuitTimer);
                scheduleAt(simTime(), buildCircuitTimer);
            }
            relayPoolSize.record(relayPool.size());
        }

        void removeRandomRelay() {
            map<NodeHandle, Certificate>::iterator it = relayPool.begin();
            int randInt = intuniform(0, relayPool.size() - 1);
            std::advance(it, randInt);
            relayPool.erase(it);
        }

        vector<NodeHandle> getNRandomRelays(int numRelays = 1) {
            vector<NodeHandle> selection;
            NodeHandle randomRelay;
            map<NodeHandle, Certificate> relaySet = relayPool;
            map<NodeHandle, Certificate>::iterator it;
            int randInt;

            for(int i = 0; i < numRelays; i++) {
                it = relaySet.begin();
                randInt = intuniform(0, relaySet.size() - 1);
                std::advance(it, randInt);
                randomRelay = it->first;

                relaySet.erase(randomRelay);
                selection.push_back(randomRelay);
            }

            return selection;
        }

        void printLog(string function, string message = "") {
            EV << "[TrafficMixer::" << function
               << " @ " << thisNode.getIp().str()
               << "(" << thisNode.getKey().toString(16) << ")"
               << message << "]" << endl;
        }

        UDPCall* buildUDPCall();

        UDPResponse* buildUDPResponse();

        void sendUDPPing(TransportAddress* addr);

        void handleUDPPong(UDPResponse* tcpMsg);

        NodeHandle getNextSelectedRelay() {
            NodeHandle node = selectedNodes[selectedNodes.size() - 1];
            selectedNodes.pop_back();
            return node;
        }

        void storeTraffic(OverlayKey circuitID, TransportAddress target);

        void releaseTrafficHistory();

        void startBuildingCircuit() {
            isCircuitBuilding = true;
            selectedNodes = getNRandomRelays(circuitLength);
            circuitBuildingBeginTime = simTime();

            EV << "    Selected nodes: [" << endl;
            for(auto node: selectedNodes)
                EV << "        " << node.getIp() << ":" << node.getPort() << endl;

            EV << "    ]" << endl;

            pendingCircuit = CircuitManager(this);
        }

        void stopBuildingCircuit(bool isBuildSuccess = true) {
            isCircuitBuilding = false;
            cancelEvent(buildCircuitTimer);
            if(isBuildSuccess) {
                OverlayKey circuitID = pendingCircuit.getCircuitIDAtPosition(0);
                ownCircuits[circuitID] = pendingCircuit;
                scheduleAt(simTime() + SimTime::parse("5s"), buildCircuitTimer);
            } else {
                scheduleAt(simTime(), buildCircuitTimer);
            }
        }
};


#endif
