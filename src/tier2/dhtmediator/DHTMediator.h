#ifndef __DHTMEDIATOR_H_
#define __DHTMEDIATOR_H_

#include <map>
#include "tier2/dhttestapp/DHTTestApp.h"
#include "tier2/dhtmediator/DHTOperationMessages_m.h"
#include "applications/trafficmixer/mixerpackets/CircuitEvidence.h"
#include "applications/trafficmixer/mixerpackets/Certificate.h"
#include "common/CommonMessages_m.h"

using std::map;
using std::tuple;
using std::get;


enum CallType {
    UNDEFINED = 0,
    PUT_CERTIFICATE = 1,
    GET_CERTIFICATE = 2,
    PUT_EVIDENCE = 3,
    GET_EVIDENCE = 4
};

typedef tuple<CallType, BaseCallMessage*> StoredCall;


class DHTMediator : public DHTTestApp {

    public:
        enum EntryType {
            NODEINFO   = 1,
            EVIDENCE    = 2
        };

        DHTMediator();
        virtual ~DHTMediator();

    protected:

        /**
         * See BaseRpc.h
         */
        virtual bool internalHandleRpcCall(BaseCallMessage* msg) override;

    private:
        map<int, StoredCall> pendingRpcCalls;

        /**
         * see DHTTestApp.h
         */
        void handleRpcResponse(BaseResponseMessage* msg, const RpcState& state,
                               simtime_t rtt) override;

        // INTERNAL RPC HANDLING:
        void handleGetCertificateCall(GetCertificateCall* msg);

        void handlePutCertificateCall(PutCertificateCall* msg);

        void sendGetCertificateResponse(BaseCallMessage* call, DHTgetCAPIResponse* result);

        void sendPutCertificateResponse(BaseCallMessage* call, DHTputCAPIResponse* result);


        void handleGetEvidenceCall(GetEvidenceCall* msg);

        void handlePutEvidenceCall(PutEvidenceCall* msg);

        void sendGetEvidenceResponse(BaseCallMessage* call, DHTgetCAPIResponse* result);

        void sendPutEvidenceResponse(BaseCallMessage* call, DHTputCAPIResponse* result);

        //-- END OF INTERNAL RPC HANDLING


        /**
         * Put a call RPC message in local memory to retrieve it upon response.
         *
         * @param call BaseCallMessage the request from TIER3 to be remembered
         * @param dhtRequestNonce int the nonce of the corresponding DHT API operation
         */
        void putCallInPendingList(BaseCallMessage* call, int dhtRequestNonce) {
            printLog("putCallInPendingList");

            CallType type;
            BaseCallMessage* storedMessage;
            OverlayCtrlInfo* controlInfo;
            RPC_SWITCH_START(call)
            RPC_ON_CALL(PutCertificate) {
                type = PUT_CERTIFICATE;
                storedMessage = _PutCertificateCall->dup();
                controlInfo = check_and_cast<OverlayCtrlInfo*>
                        (_PutCertificateCall->getControlInfo())->dup();
            }
            RPC_ON_CALL(GetCertificate) {
                type = GET_CERTIFICATE;
                storedMessage = _GetCertificateCall->dup();
                controlInfo = check_and_cast<OverlayCtrlInfo*>
                        (_GetCertificateCall->getControlInfo())->dup();
            }
            RPC_ON_CALL(PutEvidence) {
                type = PUT_EVIDENCE;
                storedMessage = _PutEvidenceCall->dup();
                controlInfo = check_and_cast<OverlayCtrlInfo*>
                        (_PutEvidenceCall->getControlInfo())->dup();

            }
            RPC_ON_CALL(GetEvidence) {
                type = GET_EVIDENCE;
                storedMessage = _GetEvidenceCall->dup();
                controlInfo = check_and_cast<OverlayCtrlInfo*>
                        (_GetEvidenceCall->getControlInfo())->dup();

            }
            RPC_SWITCH_END()

            storedMessage->setControlInfo(controlInfo);
            StoredCall record(type, storedMessage);
            pendingRpcCalls[dhtRequestNonce] = record;
        }

        /**
         * Get the call RPC message which triggered a DHT operation.
         *
         * @param response BaseResponseMessage the response received from TIER1
         * @return the corresponding BaseCallMessage that triggered the DHT operation;
         *         if no such message is found, nullptr
         */
        StoredCall getCallFromResponse(BaseResponseMessage* response) {
            int nonce = response->getNonce();
            if(pendingRpcCalls.find(nonce) != pendingRpcCalls.end()) {
                StoredCall retrievedCall = pendingRpcCalls[nonce];
                pendingRpcCalls.erase(nonce);

                return retrievedCall;
            }
            StoredCall noMatch(UNDEFINED, nullptr);
            return noMatch;
        }

        int storeEvidence(CircuitEvidence evidence, int ttl);

        int storeCertificate(Certificate cert, NodeHandle nodeHandle);

        /**
         * Requests a key-value data item through TIER1.
         *
         * @param key The key of the requested data item
         * @return nonce of the DHT get request
         */
        int sendGetRequest(const OverlayKey& key);

        /**
         * Distributes a key-value data item through TIER1.
         *
         * @param key The overlay key assigned to the data item
         * @param value The value of the distributed data item
         * @param ttl The data item's Time-To-Live parameter in seconds
         * @param isModifiable Determines whether the data item can be
         *        modified after being distributed; defaults to true
         * @return nonce of the DHT put request
         */
        int sendPutRequest(
            const OverlayKey& key,
            const BinaryValue value,
            const int ttl,
            const bool isModifiable = true,
            const int kind = 1
        );

        OverlayKey getOverlayKeyFromNodeHandle(NodeHandle handle);

        void printLog(string function, string message = "") {
            EV << "[DHTMediator::" << function
               << " @ " << thisNode.getIp().str()
               << "(" << thisNode.getKey().toString(16) << ")"
               << message << "]" << endl;
        }
};

#endif
