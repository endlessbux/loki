#include "TrustedAuthority.h"


Define_Module(TrustedAuthority);


TransportAddress TrustedAuthority::address = TransportAddress::UNSPECIFIED_NODE;
int TrustedAuthority::port = 31415;

TrustedAuthority::TrustedAuthority() {

}


TrustedAuthority::~TrustedAuthority() {

}


void TrustedAuthority::initializeApp(int stage) {
    if(stage != MIN_STAGE_APP) {
        return;
    }

    thisNode.setPort(TrustedAuthority::port);
    TrustedAuthority::address = thisNode;

    isDeanonymising = false;


    deanonymisationTimer = new cMessage("Deanonymisation Timer");
    scheduleAt(simTime() + SimTime::parse("120s"), deanonymisationTimer);


    bindToPort(TrustedAuthority::port);

    WATCH(isDeanonymising);
    WATCH_VECTOR(requestRegister);
}


void TrustedAuthority::finishApp() {

}


void TrustedAuthority::handleUDPMessage(cMessage* msg) {
    printLog("handleUDPMessage");

    GetEvidenceResponse* evidenceMsg = dynamic_cast<GetEvidenceResponse*>(msg);
    if(evidenceMsg) {
        handleGetEvidenceResponse(evidenceMsg);
    }
}


bool TrustedAuthority::internalHandleRpcCall(BaseCallMessage* msg) {
    printLog("internalHandleRpcCall");
    TrafficReport* reportMsg = dynamic_cast<TrafficReport*>(msg);
    if(reportMsg) {
        handleTrafficReport(reportMsg);
        return true;
    }

    RegistrationCall* registrationMsg = dynamic_cast<RegistrationCall*>(msg);
    if(registrationMsg) {
        handleRegistrationCall(registrationMsg);
        return true;
    }

    return false;
}


void TrustedAuthority::internalHandleRpcResponse(BaseResponseMessage* msg,
                                                 cObject* context, int rpcId,
                                                 simtime_t rtt) {
    printLog("internalHandleRpcResponse");

    GetEvidenceResponse* evidenceMsg = dynamic_cast<GetEvidenceResponse*>(msg);
    if(evidenceMsg) {
        handleGetEvidenceResponse(evidenceMsg);
    }
}


void TrustedAuthority::internalHandleRpcMessage(BaseRpcMessage* msg) {
    printLog("internalHandleRpcMessage");

    TrafficReport* reportMsg = dynamic_cast<TrafficReport*>(msg);
    RegistrationCall* registrationMsg = dynamic_cast<RegistrationCall*>(msg);
    GetEvidenceResponse* evidenceMsg = dynamic_cast<GetEvidenceResponse*>(msg);
    if(reportMsg) {
        handleTrafficReport(reportMsg);
    } else if(registrationMsg) {
        handleRegistrationCall(registrationMsg);
    } else if(evidenceMsg) {
        handleGetEvidenceResponse(evidenceMsg);
    } else {
        EV << "    RPCMessage unhandled: " << msg->getName() << endl;
    }

}


uint32_t TrustedAuthority::sendUdpRpcCall(TransportAddress dest,
                                          BaseCallMessage* msg, cObject* context,
                                          simtime_t timeout, int retries, int rpcId,
                                          RpcListener* rpcListener) {
    printLog("sendUdpRpcCall");
    dest.setPort(TrustedAuthority::port);
    EV << "    Sending message to: " << dest.getIp()
       << ":" << dest.getPort() << endl;
    return BaseApp::sendUdpRpcCall(dest, msg, context, timeout,
                                   retries, rpcId, rpcListener);
}


void TrustedAuthority::handleTimerEvent(cMessage* msg) {
    if(msg == deanonymisationTimer) {
        if(isDeanonymising) {
            traceCircuitOrigin(
                    pendingCircuitTrace[pendingCircuitTrace.size() - 1]);
        } else {
            traceRandomRequest();
        }
        cancelEvent(deanonymisationTimer);
        scheduleAt(simTime() + SimTime::parse("30s"), deanonymisationTimer);
    }
}


void TrustedAuthority::handleTrafficReport(TrafficReport* reportMsg) {
    printLog("handleTrafficReport");
    EV << "    Storing traffic data..." << endl;
    using std::begin;
    using std::end;

    int dataSetSize = reportMsg->getRecordsArraySize();
    for(int i = 0; i < dataSetSize; i++) {
        requestRegister.push_back(reportMsg->getRecords(i));
    }
//    ReportResponse* response = new ReportResponse();
//    sendRpcResponse(reportMsg, response);
}


void TrustedAuthority::handleRegistrationCall(RegistrationCall* registrationMsg) {
    printLog("handleRegistrationCall");
    Certificate cert = registrationMsg->getCert();
    TransportAddress nodeAddr = (TransportAddress)registrationMsg->getNode();
    RegisteredUser user = {nodeAddr, cert};
    registeredUsers[cert.toStringID()] = user;
}


void TrustedAuthority::handleGetEvidenceResponse(GetEvidenceResponse* msg) {
    printLog("handleGetEvidenceResponse");
    CircuitEvidence evidence = msg->getEvidence();
    if(evidence.getType() == 0) {
        EV << "    Circuit evidence not found; aborting..." << endl;
        stopDeanonymising();
        cancelEvent(deanonymisationTimer);
        scheduleAt(simTime() + SimTime::parse("5s"), deanonymisationTimer);
        return;
    }

    EvidenceType type = static_cast<EvidenceType>(evidence.getType());
    OverlayKey circuitID = evidence.getCircuitID();
    string jsonCall = evidence.getJsonCall();

    pendingCircuitTrace.push_back(circuitID);
    pendingCircuitEvidence.push_back(evidence);

    EV << "    Evidence type: " << type << endl;
    EV << "    Serialised call: " << jsonCall << endl;
    if(type == CIRCUIT_CREATION) {
        EV << "    Found circuit creation evidence; Storing user information..."
           << endl;
        // Circuit creation evidence; store information
        CreateCircuitCall creationCall = getCreateCircuitCallFromJSON(jsonCall);
        Certificate creatorCert = creationCall.getCreatorCert();
        RegisteredUser user = registeredUsers[creatorCert.toStringID()];
        CircuitData data(user, pendingCircuitTrace, pendingCircuitEvidence);
        deanonymisedCircuits[pendingCircuitTrace[0]] = data;
        stopDeanonymising();
        cancelEvent(deanonymisationTimer);
        scheduleAt(simTime() + SimTime::parse("60s"), deanonymisationTimer);

    } else {
        EV << "    Found circuit extension evidence; ";
        // Circuit extension evidence; follow trace
        ExtendCircuitCall extensionCall = getExtendCircuitCallFromJSON(jsonCall);
        OverlayKey prevCircuitID = extensionCall.getPrevCircuitID();
        EV << "Following trace for circuitID: " << prevCircuitID.toString() << endl;
        traceCircuitOrigin(prevCircuitID);
    }
}


void TrustedAuthority::traceRandomRequest() {

    TrafficRecord record;
    OverlayKey circuitID;
    bool isRequestPicked = false;
    int tries = -1;

    while(!isRequestPicked && tries < 20) {
        tries++;
        record = getRandomTrafficRecord();
        circuitID = record.getCircuitID();
        isRequestPicked = deanonymisedCircuits.find(circuitID) ==
                          deanonymisedCircuits.end();
    }

    if(isRequestPicked)
        traceCircuitOrigin(circuitID);
    else
        EV << "    No anonymous circuit in list; aborting..." << endl;
}


void TrustedAuthority::traceCircuitOrigin(OverlayKey circuitID) {
    startDeanonymising();

    GetEvidenceCall* call = new GetEvidenceCall();
    call->setCircuitID(circuitID);

    map<string, RegisteredUser>::iterator it = registeredUsers.begin();
    advance(it, intuniform(0, registeredUsers.size() - 1));
    TransportAddress randNode = get<0>(it->second);

    sendUdpRpcCall(randNode, call);
}


TrafficRecord TrustedAuthority::getRandomTrafficRecord() {
    assert(requestRegister.size() > 0);
    int randInt = intuniform(0, requestRegister.size() - 1);

    return requestRegister[randInt];
}
