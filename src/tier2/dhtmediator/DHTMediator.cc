#include "DHTMediator.h"
#include "common/RpcMacros.h"
#include "common/CommonMessages_m.h"
#include "DHTOperationMessage_m.h"


Define_Module(DHTMediator);


DHTMediator::DHTMediator() {

}


DHTMediator::~DHTMediator() {

}


void DHTMediator::handleTimerEvent(cMessage* msg) {

}


void DHTMediator::sendGetRequest(const OverlayKey& key) {
    DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();
    dhtGetMsg->setKey(key);
    RECORD_STATS(numSent++; numGetSent++);

    DHTStatsContext* context = new DHTStatsContext(
        globalStatistics->isMeasuring(),
        simTime(),
        key
    );
    sendInternalRpcCall(TIER1_COMP, dhtGetMsg, context);
}


int DHTMediator::storeEvidence(const CircuitEvidence evidence,
                               const int ttl) {
    OverlayKey key = evidence.getCircuitID();
    BinaryValue value = BinaryValue(evidence.dup());
    return sendPutRequest(key, value, ttl, false);
}


int DHTMediator::sendPutRequest(
        const OverlayKey& key,
        const BinaryValue value,
        const int ttl,
        const bool isModifiable) {
    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
    dhtPutMsg->setKey(key);
    dhtPutMsg->setValue(value);
    dhtPutMsg->setTtl(ttl);
    dhtPutMsg->setIsModifiable(true);
    RECORD_STATS(numSent++; numGetSent++);

    DHTStatsContext* context = new DHTStatsContext(
        globalStatistics->isMeasuring(),
        simTime(),
        dhtPutMsg->getKey(),
        dhtPutMsg->getValue()
    );
    sendInternalRpcCall(TIER1_COMP, dhtPutMsg, context);
    return dhtPutMsg->getNonce();
}


void DHTMediator::handleGetResponse(DHTgetCAPIResponse* msg,
                                    DHTStatsContext* context) {
    DhtDumpEntry entry = msg->getResult(0);
    StorageGetResultMessage* resultMsg = new StorageGetResultMessage();
    resultMsg->setDhtEntry(entry);
    sendInternalRpcCall(TIER3_COMP, resultMsg, context);
}


void DHTMediator::handlePutResponse(DHTputCAPIResponse* msg,
                                    DHTStatsContext* context) {
    int nonce = msg->getNonce();
    StorageSuccessMessage* successMsg = new StorageSuccessMessage();
    successMsg->setConfirmedNonce(nonce);
    sendInternalRpcCall(TIER3_COMP, successMsg, context);
}
