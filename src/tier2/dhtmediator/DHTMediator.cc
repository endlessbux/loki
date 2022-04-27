#include "DHTMediator.h"
#include "common/RpcMacros.h"
#include "common/CommonMessages_m.h"
#include "DHTOperationMessages_m.h"
#include "tier2/dhttestapp/GlobalDhtTestMap.h"
#include "common/GlobalNodeListAccess.h"
#include "common/GlobalStatisticsAccess.h"
#include "common/UnderlayConfiguratorAccess.h"
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


void DHTMediator::initializeApp(int stage) {
    if (stage != MIN_STAGE_APP)
        return;


    // fetch parameters
    ttl = par("ttl");


    globalNodeList = GlobalNodeListAccess().get();
    underlayConfigurator = UnderlayConfiguratorAccess().get();
    globalStatistics = GlobalStatisticsAccess().get();

    globalDhtTestMap = dynamic_cast<GlobalDhtTestMap*>(
            (*getSimulation()).getModuleByPath(
                    "globalObserver.globalFunctions[0].function"));

    if (globalDhtTestMap == NULL) {
        throw cRuntimeError("DHTTestApp::initializeApp(): "
                                "GlobalDhtTestMap module not found!");
    }


    //statistics
    numSent = 0;
    numGetSent = 0;
    numGetError = 0;
    numGetSuccess = 0;
    numPutSent = 0;
    numPutError = 0;
    numPutSuccess = 0;

    numGetCertificateSent = 0;
    numGetCertificateSuccess = 0;
    numGetCertificateError = 0;
    numPutCertificateSent = 0;
    numPutCertificateSuccess = 0;
    numPutCertificateError = 0;
    numGetEvidenceSent = 0;
    numGetEvidenceSuccess = 0;
    numGetEvidenceError = 0;
    numPutEvidenceSent = 0;
    numPutEvidenceSuccess = 0;
    numPutEvidenceError = 0;



    WATCH(numSent);
    WATCH(numGetSent);
    WATCH(numGetError);
    WATCH(numGetSuccess);
    WATCH(numPutSent);
    WATCH(numPutError);
    WATCH(numPutSuccess);

    WATCH(numGetCertificateSent);
    WATCH(numGetCertificateSuccess);
    WATCH(numGetCertificateError);
    WATCH(numPutCertificateSent);
    WATCH(numPutCertificateSuccess);
    WATCH(numPutCertificateError);
    WATCH(numGetEvidenceSent);
    WATCH(numGetEvidenceSuccess);
    WATCH(numGetEvidenceError);
    WATCH(numPutEvidenceSent);
    WATCH(numPutEvidenceSuccess);
    WATCH(numPutEvidenceError);
}


void DHTMediator::finishApp() {
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if(time < GlobalStatistics::MIN_MEASURED) {
        return;
    }


    // Statistics over time
    globalStatistics->addStdDev("DHTMediator: Sent Total Messages/s",
                                numSent / time);

    globalStatistics->addStdDev("DHTMediator: Sent GET Messages/s",
                                numGetSent / time);
    globalStatistics->addStdDev("DHTMediator: Sent GET Certificate Messages/s",
                                numGetCertificateSent / time);
    globalStatistics->addStdDev("DHTMediator: Sent GET Evidence Messages/s",
                                numGetEvidenceSent / time);

    globalStatistics->addStdDev("DHTMediator: Failed GET Requests/s",
                                numGetError / time);
    globalStatistics->addStdDev("DHTMediator: Failed GET Certificate Requests/s",
                                numGetCertificateError / time);
    globalStatistics->addStdDev("DHTMediator: Failed GET Evidence Requests/s",
                                numGetEvidenceError / time);

    globalStatistics->addStdDev("DHTMediator: Successful GET Requests/s",
                                numGetSuccess / time);
    globalStatistics->addStdDev("DHTMediator: Successful GET Certificate Requests/s",
                                numGetCertificateSuccess / time);
    globalStatistics->addStdDev("DHTMediator: Successful GET Requests/s",
                                numGetEvidenceSuccess / time);


    globalStatistics->addStdDev("DHTMediator: Sent PUT Messages/s",
                                numPutSent / time);
    globalStatistics->addStdDev("DHTMediator: Sent PUT Certificate Messages/s",
                                numPutCertificateSent / time);
    globalStatistics->addStdDev("DHTMediator: Sent PUT Evidence Messages/s",
                                numPutEvidenceSent / time);

    globalStatistics->addStdDev("DHTMediator: Failed PUT Requests/s",
                                numPutError / time);
    globalStatistics->addStdDev("DHTMediator: Failed PUT Certificate Requests/s",
                                numPutCertificateError / time);
    globalStatistics->addStdDev("DHTMediator: Failed PUT Evidence Requests/s",
                                numPutEvidenceError / time);

    globalStatistics->addStdDev("DHTMediator: Successful PUT Requests/s",
                                numPutSuccess / time);
    globalStatistics->addStdDev("DHTMediator: Successful PUT Certificate Requests/s",
                                numPutCertificateSuccess / time);
    globalStatistics->addStdDev("DHTMediator: Successful PUT Evidence Requests/s",
                                numPutEvidenceSuccess / time);

    // GET Success Ratio
    if ((numGetSuccess + numGetError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: GET Success Ratio",
                (double)numGetSuccess / (double)(numGetSuccess + numGetError));
    }
    // GET Certificate Success Ratio
    if ((numGetCertificateSuccess + numGetCertificateError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: GET Certificate Success Ratio",
                (double)numGetCertificateSuccess /
                (double)(numGetCertificateSuccess + numGetCertificateError));
    }
    // GET Evidence Success Ratio
    if ((numGetEvidenceSuccess + numGetEvidenceError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: GET Evidence Success Ratio",
                (double)numGetEvidenceSuccess /
                (double)(numGetEvidenceSuccess + numGetEvidenceError));
    }


    // PUT Success Ratio
    if ((numPutSuccess + numPutError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: PUT Success Ratio",
                (double)numPutSuccess / (double)(numPutSuccess + numPutError));
    }
    // PUT Certificate Success Ratio
    if ((numPutCertificateSuccess + numPutCertificateError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: PUT Certificate Success Ratio",
                (double)numPutCertificateSuccess /
                (double)(numPutCertificateSuccess + numPutCertificateError));
    }
    // PUT Evidence Success Ratio
    if ((numPutEvidenceSuccess + numPutEvidenceError) > 0) {
        globalStatistics->addStdDev(
                "DHTMediator: PUT Evidence Success Ratio",
                (double)numPutEvidenceSuccess /
                (double)(numPutEvidenceSuccess + numPutEvidenceError));
    }
}


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
    DHTStatsContext* context = check_and_cast<DHTStatsContext*>(state.getContext());


    switch(get<0>(callData)) {
    case UNDEFINED:
        EV << "    Received undefined response; aborting..." << endl;
        return;
    case PUT_CERTIFICATE:
    {
        DHTputCAPIResponse* putCert = check_and_cast<DHTputCAPIResponse*>(msg);
        sendPutCertificateResponse(get<1>(callData), putCert, context);
        break;
    }
    case GET_CERTIFICATE:
    {
        DHTgetCAPIResponse* getCert = check_and_cast<DHTgetCAPIResponse*>(msg);
        sendGetCertificateResponse(get<1>(callData), getCert, context);
        break;
    }
    case PUT_EVIDENCE:
    {
        DHTputCAPIResponse* putEv = check_and_cast<DHTputCAPIResponse*>(msg);
        sendPutEvidenceResponse(get<1>(callData), putEv, context);
        break;
    }
    case GET_EVIDENCE:
    {
        DHTgetCAPIResponse* getEv = check_and_cast<DHTgetCAPIResponse*>(msg);
        sendGetEvidenceResponse(get<1>(callData), getEv, context);
        break;
    }
    }
}


// CERTIFICATE HANDLING
void DHTMediator::handleGetCertificateCall(GetCertificateCall* msg) {
    printLog("handleGetCertificateCall");

    NodeHandle handle = msg->getNode();
    OverlayKey certKey = getOverlayKeyFromNodeHandle(handle);

    EV << "    Node: " + handle.getIp().str();
    EV << "    OverlayKey: " + certKey.toString(16);

    RECORD_STATS(numGetCertificateSent++);

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
                                             DHTgetCAPIResponse* result,
                                             DHTStatsContext* context) {
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
        RECORD_STATS(numGetSuccess++; numGetCertificateSuccess++);
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: GET Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: GET Certificate Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));

        fetchedCert = getCertificateFromJSON(jsonCert);
    } else {
        EV << "    No result." << endl;
        RECORD_STATS(numGetError++; numGetCertificateError++);

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
                                             DHTputCAPIResponse* result,
                                             DHTStatsContext* context) {
    printLog("sendPutCertificateResponse");

    DHTEntry entry = {context->value, simTime() + ttl, simTime()};
    globalDhtTestMap->insertEntry(context->key, entry);


    bool isSuccess = result->getIsSuccess();

    EV << "    isSuccess: " << std::to_string(isSuccess) << endl;
    EV << "    Nonce: " + std::to_string(call->getNonce()) << endl;

    if(isSuccess) {
        RECORD_STATS(numPutSuccess++; numPutCertificateSuccess++);
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: PUT Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: PUT Certificate Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
    } else {
        RECORD_STATS(numPutError++; numPutCertificateError++);
    }

    PutCertificateResponse* response = new PutCertificateResponse();
    response->setIsSuccess(isSuccess);

    sendRpcResponse(call, response);
}
//-- END OF CERTIFICATE HANDLING


// EVIDENCE HANDLING
void DHTMediator::handleGetEvidenceCall(GetEvidenceCall* msg) {
    printLog("handleGetEvidenceCall");
    OverlayKey circuitID = msg->getCircuitID();

    RECORD_STATS(numGetEvidenceSent++);

    int dhtRequestNonce = sendGetRequest(circuitID);
    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::handlePutEvidenceCall(PutEvidenceCall* msg) {
    printLog("handlePutEvidenceCall");
    CircuitEvidence receivedEvidence = msg->getEvidence();
    int timeToLive = 3600 * 24 * 365;

    int dhtRequestNonce = storeEvidence(receivedEvidence, timeToLive);
    putCallInPendingList(msg, dhtRequestNonce);
}


void DHTMediator::sendGetEvidenceResponse(BaseCallMessage* call,
                                          DHTgetCAPIResponse* result,
                                          DHTStatsContext* context) {
    printLog("sendGetEvidenceResponse");

    CircuitEvidence fetchedEvidence;
    bool isSuccess = result->getIsSuccess();

    EV << "    isSuccess: " << std::to_string(isSuccess) << endl;
    EV << "    Result size: " << std::to_string(result->getResultArraySize()) << endl;

    if(isSuccess && result->getResultArraySize() > 0) {
        // there should only be one result
        DhtDumpEntry entry = result->getResult(0);
        BinaryValue serialisedEvidence = entry.getValue();
        string jsonEvidence(serialisedEvidence.begin(), serialisedEvidence.end());
        EV << "    JSON:\n" << jsonEvidence << endl;
        RECORD_STATS(numGetSuccess++; numGetEvidenceSuccess++);
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: GET Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: GET Evidence Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));

        fetchedEvidence = getCircuitEvidenceFromJSON(jsonEvidence);
    } else {
        EV << "    Evidence was not found..." << endl;
        RECORD_STATS(numGetError++; numGetEvidenceError++);

        fetchedEvidence = CircuitEvidence();
    }

    GetEvidenceResponse* response = new GetEvidenceResponse();
    response->setEvidence(fetchedEvidence);
    sendRpcResponse(call, response);
}


void DHTMediator::sendPutEvidenceResponse(BaseCallMessage* call,
                                          DHTputCAPIResponse* result,
                                          DHTStatsContext* context) {
    printLog("sendPutEvidenceResponse");

    DHTEntry entry = {context->value, simTime() + ttl, simTime()};
    globalDhtTestMap->insertEntry(context->key, entry);


    bool isSuccess = result->getIsSuccess();


    if(isSuccess) {
        RECORD_STATS(numPutSuccess++; numPutEvidenceSuccess++);
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: PUT Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
        RECORD_STATS(
                globalStatistics->addStdDev(
                        "DHTMediator: PUT Evidence Latency (s)",
                        SIMTIME_DBL(simTime() - context->requestTime)));
    } else {
        RECORD_STATS(numPutError++; numPutEvidenceError++);
    }


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
    RECORD_STATS(numPutEvidenceSent++);
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
    RECORD_STATS(numPutCertificateSent++);
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
