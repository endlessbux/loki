#include "DHTMediator.h"
#include "common/RpcMacros.h"
#include "common/CommonMessages_m.h"
#include "DHTOperationMessages_m.h"
#include <bits/stdc++.h>


Define_Module(DHTMediator);


DHTMediator::DHTMediator() {

}


DHTMediator::~DHTMediator() {

}


bool DHTMediator::internalHandleRpcCall(BaseCallMessage* msg) {
    // check if the message is any of:
    //      GetCertificateCall, PutCertificateCall,
    //      GetEvidenceCall, PutEvidenceCall
    printLog("internalHandleRpcCall");
    RPC_SWITCH_START(msg)
    RPC_DELEGATE(GetCertificate, handleGetCertificateCall)
    RPC_DELEGATE(PutCertificate, handlePutCertificateCall)
    RPC_DELEGATE(GetEvidence, handleGetEvidenceCall)
    RPC_DELEGATE(PutEvidence, handlePutEvidenceCall)
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}


//void DHTMediator::handleGetResponse(DHTgetCAPIResponse* msg,
//                                    DHTStatsContext* context) {
//    DHTTestApp::handleGetResponse(msg, context);
//    string debugMsg = "";
//
//    // get the corresponding call
//    BaseCallMessage* call = getCallFromResponse((BaseResponseMessage*)msg);
//
//    if(!call) {
//        // Corresponding call not found
//        return;
//    }
//
//    // check which kind of put operation was performed and call
//    // the corresponding procedure
//    RPC_SWITCH_START(call)
//    RPC_ON_CALL(GetCertificate) {
//        debugMsg += "\n    Found GetCertificateCall result; sending it to upper tier";
//        sendGetCertificateResponse(_GetCertificateCall, msg);
//    }
//    RPC_ON_CALL(GetEvidence) {
//        debugMsg += "\n    Found GetEvidenceCall result; sending it to upper tier";
//        sendGetEvidenceResponse(_GetEvidenceCall, msg);
//    }
//    RPC_SWITCH_END( )
//
//    printLog("handleGetResponse", debugMsg);
//}


void DHTMediator::handleRpcResponse(BaseResponseMessage* msg,
                                    const RpcState& state, simtime_t rtt) {
    printLog("handleRpcResponse");

    // handle each different possible response from DHT
    StoredCall callData = getCallFromResponse(msg);
    BaseCallMessage* call = get<1>(callData);
    if(!call) {
        EV << "    The received response doesn't match a stored call; "
           << "aborting..." << endl;
        return;
    }

    EV << "    Corresponding Nonce: " << std::to_string(call->getNonce()) << endl;

    switch(get<0>(callData)) {
    case UNDEFINED:
        return;
    case PUT_CERTIFICATE:
    {
        DHTputCAPIResponse* putCert = check_and_cast<DHTputCAPIResponse*>(msg);
        sendPutCertificateResponse(get<1>(callData), putCert);
        break;
    }
    case GET_CERTIFICATE:
    {
        DHTgetCAPIResponse* getCert = check_and_cast<DHTgetCAPIResponse*>(msg);
        sendGetCertificateResponse(get<1>(callData), getCert);
        break;
    }
    case PUT_EVIDENCE:
    {
        DHTputCAPIResponse* putEv = check_and_cast<DHTputCAPIResponse*>(msg);
        sendPutEvidenceResponse(get<1>(callData), putEv);
        break;
    }
    case GET_EVIDENCE:
    {
        DHTgetCAPIResponse* getEv = check_and_cast<DHTgetCAPIResponse*>(msg);
        sendGetEvidenceResponse(get<1>(callData), getEv);
        break;
    }
    }
}


//void DHTMediator::handlePutResponse(DHTputCAPIResponse* msg,
//                                    DHTStatsContext* context) {
//    printLog("handlePutResponse");
//
//    DHTTestApp::handlePutResponse(msg, context);
//
//    // get the corresponding call
//    BaseCallMessage* call = getCallFromResponse((BaseResponseMessage*)msg);
//
//    if(!call) {
//        // Corresponding call not found
//        EV << "    Corresponding call message was not found; aborting..." << endl;
//        return;
//    }
//
//    EV << "    Corresponding Nonce: " << std::to_string(call->getNonce()) << endl;
//
//    // check which kind of put operation was performed and call
//    // the corresponding procedure
//    RPC_SWITCH_START(call)
//    RPC_ON_CALL(PutCertificate) {
//        EV << "    Found PutCertificateCall result; sending it to upper tier" << endl;
//        sendPutCertificateResponse(_PutCertificateCall, msg);
//    }
//    RPC_ON_CALL(PutEvidence) {
//        EV << "    Found PutEvidenceCall result; sending it to upper tier" << endl;
//        sendPutEvidenceResponse(_PutEvidenceCall, msg);
//    }
//    RPC_SWITCH_END( )
//}


// CERTIFICATE HANDLING
void DHTMediator::handleGetCertificateCall(GetCertificateCall* msg) {
    printLog("handleGetCertificateCall");

    NodeHandle handle = msg->getNode();
    OverlayKey certKey = getOverlayKeyFromNodeHandle(handle);

    EV << "    Node: " + handle.getIp().str();
    EV << "    OverlayKey: " + certKey.toString(16);

    int dhtRequestNonce = sendGetRequest(certKey);
    EV << "    Nonce: " + std::to_string(dhtRequestNonce);

    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::handlePutCertificateCall(PutCertificateCall* msg) {
    printLog("handlePutCertificateCall");
    Certificate cert = msg->getCert();
    NodeHandle handle = msg->getHandle();

    EV << "    Node: " << handle.getIp().str() << endl;

    int dhtRequestNonce = storeCertificate(cert, handle);
    EV << "    Nonce: " << dhtRequestNonce << endl;

    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::sendGetCertificateResponse(BaseCallMessage* call,
                                             DHTgetCAPIResponse* result) {
    printLog("sendGetCertificateResponse");

    Certificate fetchedCert;
    bool isSuccess = result->getIsSuccess();

    EV << "    isSuccess: " << std::to_string(isSuccess) << endl;
    EV << "    Result size: " << std::to_string(result->getResultArraySize()) << endl;


    if(isSuccess && result->getResultArraySize() > 0) {
        // there should only be one result
        DhtDumpEntry entry = result->getResult(0);
        BinaryValue serialisedCert = entry.getValue();
        string jsonCert(serialisedCert.begin(), serialisedCert.end());

        EV << "    JSON:\n" << jsonCert << endl;

        fetchedCert = getCertificateFromJSON(jsonCert);
    } else {
        EV << "    No result." << endl;
        fetchedCert = Certificate();
    }

    GetCertificateResponse* response = new GetCertificateResponse();
    response->setCert(fetchedCert);


    EV << "    Nonce: " << std::to_string(call->getNonce()) << endl;

    sendRpcResponse(call, response);
}


/**
 * Sends a response to the given PutCertificateCall procedure
 */
void DHTMediator::sendPutCertificateResponse(BaseCallMessage* call,
                                             DHTputCAPIResponse* result) {
    printLog("sendPutCertificateResponse");


    bool isSuccess = result->getIsSuccess();

    EV << "    isSuccess: " << std::to_string(isSuccess) << endl;
    EV << "    Nonce: " + std::to_string(call->getNonce()) << endl;


    PutCertificateResponse* response = new PutCertificateResponse();
    response->setIsSuccess(isSuccess);

    sendRpcResponse(call, response);
}
//-- END OF CERTIFICATE HANDLING


// EVIDENCE HANDLING
void DHTMediator::handleGetEvidenceCall(GetEvidenceCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    int dhtRequestNonce = sendGetRequest(circuitID);
    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::handlePutEvidenceCall(PutEvidenceCall* msg) {
    CircuitEvidence receivedEvidence = msg->getEvidence();
    int timeToLive = 3600 * 24 * 365;
    int dhtRequestNonce = storeEvidence(receivedEvidence, timeToLive);
    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::sendGetEvidenceResponse(BaseCallMessage* call,
                                          DHTgetCAPIResponse* result) {
    CircuitEvidence fetchedEvidence;
    bool isSuccess = result->getIsSuccess();

    if(isSuccess && result->getResultArraySize() > 0) {
        // there should only be one result
        DhtDumpEntry entry = result->getResult(0);
        BinaryValue serialisedEvidence = entry.getValue();
        string jsonEvidence(serialisedEvidence.begin(), serialisedEvidence.end());
        fetchedEvidence = getCircuitEvidenceFromJSON(jsonEvidence);
    } else {
        fetchedEvidence = CircuitEvidence();
    }

    GetEvidenceResponse* response = new GetEvidenceResponse();
    response->setEvidence(fetchedEvidence);
    sendRpcResponse(call, response);
}


void DHTMediator::sendPutEvidenceResponse(BaseCallMessage* call,
                                          DHTputCAPIResponse* result) {
    bool isSuccess = result->getIsSuccess();

    PutEvidenceResponse* response = new PutEvidenceResponse();
    response->setIsSuccess(isSuccess);

    sendRpcResponse(call, response);
}
//-- END OF EVIDENCE HANDLING


int DHTMediator::sendGetRequest(const OverlayKey& key) {
    DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();
    dhtGetMsg->setKey(key);
    RECORD_STATS(numSent++; numGetSent++);

    DHTStatsContext* context = new DHTStatsContext(
        globalStatistics->isMeasuring(),
        simTime(),
        key
    );
    return sendInternalRpcCall(TIER1_COMP, dhtGetMsg, context);
}


int DHTMediator::storeEvidence(CircuitEvidence evidence, int ttl) {
    OverlayKey key = evidence.getCircuitID();
    string jsonEvidence = evidence.toJsonString();
    BinaryValue value = BinaryValue(jsonEvidence);
    return sendPutRequest(key, value, ttl, false, EVIDENCE);
}


int DHTMediator::storeCertificate(Certificate cert,
                                  NodeHandle handle) {
    printLog("storeCertificate");

    if(!cert.isValid() ||
        handle.isUnspecified()) {
        // only store valid certificates where the TransportAddress is specified
        return -1;
    }

    EV << "    Producing overlay key from node handle..." << endl;
    OverlayKey key = getOverlayKeyFromNodeHandle(handle);
    EV << "    Converting certificate to JSON format..." << endl;
    string jsonCert = cert.toJsonString();
    EV << "    " << jsonCert << endl;
    BinaryValue value = BinaryValue(jsonCert);
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
    printLog("sendPutRequest");

    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
    dhtPutMsg->setKey(key);
    dhtPutMsg->setValue(value);
    dhtPutMsg->setTtl(ttl);
    dhtPutMsg->setIsModifiable(isModifiable);
    dhtPutMsg->setKind(kind);
    dhtPutMsg->setId(1);
    RECORD_STATS(numSent++; numPutSent++);



    DHTStatsContext* context = new DHTStatsContext(
        globalStatistics->isMeasuring(),
        simTime(),
        dhtPutMsg->getKey(),
        dhtPutMsg->getValue()
    );
    return sendInternalRpcCall(TIER1_COMP, dhtPutMsg, context);
}


OverlayKey DHTMediator::getOverlayKeyFromNodeHandle(NodeHandle handle) {
    printLog("getOverlayKeyFromNodeHandle");

    if(handle.getIp().isUnspecified()) {
        EV << "    Handle is unspecified" << endl;
        return OverlayKey::UNSPECIFIED_KEY;
    }
    std::string hashStr = handle.getIp().str();

    EV << "    Hashing handle: " << handle.getIp().str() << endl;
    EV << "    Handle hash bin: " << hashStr << endl;

    BinaryValue hashedHandle = BinaryValue(hashStr);
    OverlayKey computedKey = OverlayKey::sha1(hashedHandle);

    EV << "    Returning sha1(" << hashedHandle.str() << "): "
       << computedKey.toString(16) << endl;

    return computedKey;
}
