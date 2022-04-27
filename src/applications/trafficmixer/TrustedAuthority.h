#ifndef __TRUSTEDAUTHORITY_H_
#define __TRUSTEDAUTHORITY_H_

#include "common/TransportAddress.h"
#include "common/BaseApp.h"
#include "mixerpackets/TrafficReport_m.h"
#include "mixerpackets/CircuitEvidence.h"
#include "tier2/dhtmediator/DHTOperationMessages_m.h"
#include "overlay/permissionedchord/ChordMessage_m.h"


using namespace std;
using loki::RegistrationCall;
typedef tuple<TransportAddress, Certificate> RegisteredUser;
typedef vector<OverlayKey> CircuitTrace;
typedef vector<CircuitEvidence> EvidenceTrace;
typedef tuple<RegisteredUser, CircuitTrace, EvidenceTrace> CircuitData;

class TrustedAuthority : public BaseApp {
    public:
        static int port;
        static TransportAddress address;

        map<string, RegisteredUser> registeredUsers;
        vector<TrafficRecord> requestRegister;

        /**
         * OverlayKey --> the deanonymised circuitID
         * CircuitData --> retrieved information about the circuit
         */
        map<OverlayKey, CircuitData> deanonymisedCircuits;
        CircuitTrace pendingCircuitTrace;
        EvidenceTrace pendingCircuitEvidence;
        bool isDeanonymising;


        cMessage* deanonymisationTimer;


        TrustedAuthority();
        ~TrustedAuthority();
        uint32_t sendUdpRpcCall(TransportAddress dest,
                                BaseCallMessage* msg,
                                cObject* context = NULL,
                                simtime_t timeout = -1,
                                int retries = 0, int rpcId = -1,
                                RpcListener* rpcListener = NULL);
        virtual void initializeApp(int stage);
        virtual void finishApp();
        virtual void handleUDPMessage(cMessage* msg) override;
        virtual bool internalHandleRpcCall(BaseCallMessage* msg) override;
        virtual void internalHandleRpcResponse(BaseResponseMessage* msg,
                                               cObject* context, int rpcId,
                                               simtime_t rtt) override;
        virtual void internalHandleRpcMessage(BaseRpcMessage* msg) override;
        virtual void handleTimerEvent(cMessage* msg) override;
        void printLog(string functionName) {
            EV << "[TrustedAuthority::" << functionName
               << " @ " << thisNode.getIp().str()
               << "(" << thisNode.getKey().toString(16) << ")"
               << "]" << endl;
        }
    protected:
        void handleTrafficReport(TrafficReport* reportMsg);

        void handleRegistrationCall(RegistrationCall* registrationMsg);

        void handleGetEvidenceResponse(GetEvidenceResponse* msg);

    private:
        void traceRandomRequest();

        void traceCircuitOrigin(OverlayKey circuitID);

        TrafficRecord getRandomTrafficRecord();

        void stopDeanonymising() {
            printLog("stopDeanonymising");
            isDeanonymising = false;
            pendingCircuitTrace = CircuitTrace();
            pendingCircuitEvidence = EvidenceTrace();
        }

        void startDeanonymising() {
            printLog("startDeanonymising");
            isDeanonymising = true;
        }
};

#endif
