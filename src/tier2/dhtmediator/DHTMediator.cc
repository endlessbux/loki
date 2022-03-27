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
    return sendPutRequest(key, value, ttl, false, EVIDENCE);
}


int DHTMediator::storeCertificate(Certificate cert,
                                  NodeHandle handle) {

    if(!cert.isValid()) {
        // only store valid certificates
        return 0;
    }

    BinaryValue hashedHandle = BinaryValue(handle.hash());
    OverlayKey key = OverlayKey::sha1(hashedHandle);
    BinaryValue value = BinaryValue(&cert);
    simtime_t _ttl = cert.getExpiration() - simTime();
    int ttl = _ttl.inUnit(SIMTIME_S);
    return sendPutRequest(key, value, ttl, true, NODEINFO);
}


int DHTMediator::sendPutRequest(
        const OverlayKey& key,
        const BinaryValue value,
        const int ttl,
        const bool isModifiable,
        const int kind) {
    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
    dhtPutMsg->setKey(key);
    dhtPutMsg->setValue(value);
    dhtPutMsg->setTtl(ttl);
    dhtPutMsg->setIsModifiable(isModifiable);
    dhtPutMsg->setKind(kind);
    //RECORD_STATS(numSent++; numGetSent++);

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
    GetCall* resultMsg = new GetCall();
    resultMsg->setDhtEntry(entry);
    sendInternalRpcCall(TIER3_COMP, resultMsg, context);
}


void DHTMediator::handlePutResponse(DHTputCAPIResponse* msg,
                                    DHTStatsContext* context) {
    int nonce = msg->getNonce();
    PutCall* successMsg = new PutCall();
    successMsg->setConfirmedNonce(nonce);
    sendInternalRpcCall(TIER3_COMP, successMsg, context);
}
