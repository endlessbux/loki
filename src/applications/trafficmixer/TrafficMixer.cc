#include "TrafficMixer.h"
#include "common/GlobalNodeList.h"
#include "overlay/permissionedchord/ChordMessage_m.h"
#include "applications/trafficmixer/mixerpackets/TrafficReport_m.h"

using namespace std;
using loki::StorePeerHandleCall;
using loki::StoreOwnCertificateCall;


Define_Module(TrafficMixer);

TrafficMixer::TrafficMixer() {
}


TrafficMixer::~TrafficMixer() {
    cancelAndDelete(requestTimer);
}


void TrafficMixer::initializeApp(int stage) {
    if (stage != MIN_STAGE_APP) {
        return;
    }


    isOwnCertificateShared = false;
    certStorageRetries = 0;
    maxCertRetries = 5;
    maxPoolSize = 50;
    maxOpenCircuits = 5;
    circuitLength = 3;
    maxRequestRetries = 3;
    timeoutLimit = SimTime::parse("10s");

    isCircuitBuilding = false;


    globalStatistics = GlobalStatisticsAccess().get();
    numOnionSent = 0;
    bytesOnionSent = 0;
    numOnionReceived = 0;
    bytesOnionReceived = 0;
    numOnionRelayed = 0;
    bytesOnionRelayed = 0;
    numExitRequests = 0;
    bytesExitRequests = 0;
    numExitResponse = 0;
    bytesExitResponse = 0;
    numTargetSent = 0;
    bytesTargetSent = 0;
    bytesTargetReceived = 0;
    numTargetSuccess = 0;
    numTargetFailure = 0;
    numBuildCircuitSent = 0;
    bytesBuildCircuitSent = 0;
    numBuildCircuitReceived = 0;
    bytesBuildCircuitReceived = 0;
    numCreateSuccess = 0;
    numCreateFailure = 0;
    numExtendSuccess = 0;
    numExtendFailure = 0;

    WATCH(isOwnCertificateShared);
    WATCH(maxPoolSize);
    WATCH_SET(addressPool);
    WATCH_MAP(relayPool);
    WATCH_MAP(ownCircuits);
    WATCH_PTRMAP(extCircuits);

    WATCH(numOnionRelayed);
    WATCH(numExitRequests);
    WATCH(numTargetSent);
    WATCH(bytesTargetSent);
    WATCH(bytesTargetReceived);
    WATCH(numTargetSuccess);
    WATCH(numTargetFailure);
    WATCH(numCreateSuccess);
    WATCH(numCreateFailure);
    WATCH(numExtendSuccess);
    WATCH(numExtendFailure);

    state = INIT;
    applicationPort = 31415;

    peerLookupTimer = new cMessage("PeerLookupTimer");
    scheduleAt(simTime() + SimTime::parse("5s"), peerLookupTimer);

    requestTimer = new cMessage("RequestTimer");
    buildCircuitTimer = new cMessage("BuildCircuitTimer");

    trafficReportTimer = new cMessage("TrafficReportTimer");
    scheduleAt(simTime() + SimTime::parse("60s"), trafficReportTimer);

    bindToPort(applicationPort);

    relayPoolSize.setName("TrafficMixer: Relay pool size");
    onionsRelayed.setName("TrafficMixer: Relayed onions");
    circuitBuildingTime.setName("TrafficMixer: Circuit building time");
    requestLatencies.setName("TrafficMixer: Request latencies");
    ownCircuitsLengths.setName("TrafficMixer: Circuits lengths");
}


void TrafficMixer::finishApp() {
    changeState(SHUTDOWN);
    releaseTrafficHistory();

    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->recordOutVector(
            "TrafficMixer: Measured time",
            time.dbl());

    // get data storage and save amount of stored bytes and records
    DHTDataStorage* dataStorage = check_and_cast<DHTDataStorage*>
            (getParentModule()->getParentModule()->getSubmodule("tier1")
             ->getSubmodule("dhtDataStorage"));
    DhtDumpVector localDht = *dataStorage->dumpDht();

    int numStoredRecords = (int)localDht.size();
    int numBitsStored = 0;
    for(int i = 0; i < localDht.size(); i++) {
        DhtDumpEntry entry = localDht[i];
        numBitsStored += (int)entry.getKey().getLength() +
                         (int)entry.getValue().size() * 8;
    }
    int numBytesStored = (numBitsStored / 8) + (numBitsStored % 8 != 0);

    globalStatistics->recordOutVector("DHTDataStorage: Stored entries",
                                      numStoredRecords);
    globalStatistics->recordOutVector("DHTDataStorage: Stored bytes",
                                      numBytesStored);
    globalStatistics->recordOutVector("TrafficMixer: Own circuits",
                                      ownCircuits.size());
    map<OverlayKey, CircuitManager>::iterator it;
    for(it = ownCircuits.begin(); it != ownCircuits.end(); it++) {
        ownCircuitsLengths.record(it->second.circuitOrder.size());
    }

    globalStatistics->recordOutVector("TrafficMixer: External circuits",
                                      extCircuits.size());
    globalStatistics->recordOutVector("TrafficMixer: Sent target request",
                                      numTargetSent);
    globalStatistics->recordOutVector("TrafficMixer: Sent target success",
                                      numTargetSuccess);
    globalStatistics->recordOutVector("TrafficMixer: Sent target failure",
                                      numTargetFailure);
    globalStatistics->recordOutVector("TrafficMixer: Bytes target sent",
                                      bytesTargetSent);
    globalStatistics->recordOutVector("TrafficMixer: Bytes target received",
                                      bytesTargetReceived);
    globalStatistics->recordOutVector("TrafficMixer: Exit sent",
                                      numExitRequests);
    globalStatistics->recordOutVector("TrafficMixer: Exit sent bytes",
                                      bytesExitRequests);
    globalStatistics->recordOutVector("TrafficMixer: Exit received",
                                      numExitResponse);
    globalStatistics->recordOutVector("TrafficMixer: Exit received bytes",
                                      bytesExitResponse);
    globalStatistics->recordOutVector("TrafficMixer: Onion relayed",
                                      numOnionRelayed);
    globalStatistics->recordOutVector("TrafficMixer: Onion bytes relayed",
                                      bytesOnionRelayed);
    globalStatistics->recordOutVector("TrafficMixer: Build circuit sent",
                                      numBuildCircuitSent);
    globalStatistics->recordOutVector("TrafficMixer: Build circuit bytes sent",
                                      bytesBuildCircuitSent);
    globalStatistics->recordOutVector("TrafficMixer: Build circuit received",
                                      numBuildCircuitReceived);
    globalStatistics->recordOutVector("TrafficMixer: Build circuit bytes received",
                                      bytesBuildCircuitReceived);
    globalStatistics->recordOutVector("TrafficMixer: Create success",
                                      numCreateSuccess);
    globalStatistics->recordOutVector("TrafficMixer: Create failure",
                                      numCreateFailure);
    globalStatistics->recordOutVector("TrafficMixer: Extend success",
                                      numExtendSuccess);
    globalStatistics->recordOutVector("TrafficMixer: Extend failure",
                                      numExtendFailure);
    globalStatistics->recordOutVector("TrafficMixer: Onion received",
                                      numOnionReceived);
    globalStatistics->recordOutVector("TrafficMixer: Onion bytes received",
                                      bytesOnionReceived);
    globalStatistics->recordOutVector("TrafficMixer: Onion sent",
                                      numOnionSent);
    globalStatistics->recordOutVector("TrafficMixer: Onion bytes sent",
                                      numOnionReceived);

    DHT* dht = check_and_cast<DHT*>(
            getParentModule()->getParentModule()->getSubmodule("tier1")
            ->getSubmodule("dht"));

    globalStatistics->recordOutVector(
            "DHT: Sent total messages",
            dht->maintenanceMessages + dht->normalMessages);
    globalStatistics->recordOutVector(
            "DHT: Sent total bytes",
            dht->numBytesMaintenance + dht->numBytesNormal);

    globalStatistics->recordOutVector(
            "DHT: Sent maintenance messages",
            dht->maintenanceMessages);
    globalStatistics->recordOutVector(
            "DHT: Sent normal messages",
            dht->normalMessages);
    globalStatistics->recordOutVector(
            "DHT: Sent maintenance bytes",
            dht->numBytesMaintenance);
    globalStatistics->recordOutVector(
            "DHT: Sent normal bytes",
            dht->numBytesNormal);
}


void TrafficMixer::handleTimerEvent(cMessage* msg) {
    printLog("handleTimerEvent");
    if(msg == requestTimer) {
        sendRequestToTarget();
    } else if(msg == buildCircuitTimer) {
        buildCircuit();
    } else if(msg == trafficReportTimer) {
        releaseTrafficHistory();
    } else if(msg == peerLookupTimer) {
        getRandomPeerCertificate();
    } else if(requestsTimeoutTimers.find(msg) != requestsTimeoutTimers.end()) {
        handleRequestTimeout(msg);
    }
}


void TrafficMixer::handleRequestTimeout(cMessage* msg) {
    requestsTimeoutTimers.erase(msg);
    if(pendingOnionRequestsTimes.find(msg->getKind()) !=
       pendingOnionRequestsTimes.end()) {
        pendingOnionRequestsTimes.erase(msg->getKind());
        pendingOnionRequestsCircuits.erase(msg->getKind());
        RECORD_STATS(numTargetFailure++);
    }
}


void TrafficMixer::sendRequestToTarget() {
    printLog("sendRequestToTarget");
    if(ownCircuits.size() == 0) {
        EV << "    The bootstrap process hasn't finished; rescheduling..." << endl;
        cancelEvent(requestTimer);
        scheduleAt(simTime() + SimTime(intuniform(5, 15) * 60, SIMTIME_S), requestTimer);
        return;
    }

    UDPCall* request = new UDPCall();
    request->setTargetAddress(TargetServer::address);
    request->setBitLength(UDPCALL_L(request));
    int randInt = intuniform(0, ownCircuits.size() - 1);
    map<OverlayKey, CircuitManager>::iterator it = ownCircuits.begin();
    advance(it, randInt);

    int nonce = simTime().inUnit(SIMTIME_MS);
    request->setInternalID(nonce);
    pendingOnionRequestsTimes[nonce] = simTime();
    pendingOnionRequestsCircuits[nonce] = it->first;
    it->second.sendRequest(request);

    cMessage* timeoutTimer = new cMessage("Request Timeout", nonce);
    scheduleAt(simTime() + timeoutLimit, timeoutTimer);
    requestsTimeoutTimers.insert(timeoutTimer);
    RECORD_STATS(numTargetSent++);
    RECORD_STATS(bytesTargetSent += request->getByteLength());
}


void TrafficMixer::buildCircuit() {
    printLog("buildCircuit");
    cancelEvent(buildCircuitTimer);
    if(state != READY) {
        EV << "    state != READY; aborting..." << endl;
        return;
    }

    if(relayPool.size() < circuitLength) {
        EV << "    Not enough relays in the pool; aborting..." << endl;
        return;
    }

    if(!isCircuitBuilding && ownCircuits.size() < maxOpenCircuits) {
        buildNewCircuit();
    } else if(isCircuitBuilding) {
        extendCircuit();
    }

}


void TrafficMixer::extendCircuit() {
    printLog("extendCircuit");
    if(selectedNodes.size() > 0 &&
       pendingCircuit.pendingRelay.first.isUnspecified()) {
        NodeHandle nextNode = getNextSelectedRelay();
        nextNode.setPort(applicationPort);

        Certificate cert = getOwnCertificate();
        string key = generateRandomKey(uniform(0, 1));
        pendingCircuit.extend(nextNode, cert, key);
    } else {
        EV << "    Something went wrong; aborting..." << endl;
        stopBuildingCircuit(false);
    }
}


void TrafficMixer::buildNewCircuit() {
    printLog("buildNewCircuit");

    startBuildingCircuit();

    NodeHandle entryNode = getNextSelectedRelay();
    entryNode.setPort(applicationPort);

    EV << "    Entry node: " << entryNode.getIp()
       << ":" << entryNode.getPort() << endl;
    Certificate cert = getOwnCertificate();
    string key = generateRandomKey(uniform(0, 1));

    pendingCircuit.create(entryNode, cert, key);
}


void TrafficMixer::changeState(int toState) {
    state = toState;
    switch(toState) {
    case INIT:
        // share certificate information with the rest of the network through the DHT

        break;
    case READY:
        // gather relays information
        // build and maintain circuits
        overlayReadyTime = simTime();
        scheduleAt(simTime(), buildCircuitTimer);
        break;
    case SHUTDOWN:
        // close up existing circuits

        break;
    }
}


uint32_t TrafficMixer::sendUdpRpcCall(TransportAddress dest,
                                      BaseCallMessage* msg, cObject* context,
                                      simtime_t timeout, int retries, int rpcId,
                                      RpcListener* rpcListener) {
    printLog("sendUdpRpcCall");
    dest.setPort(applicationPort);
    EV << "    Sending message to: " << dest.getIp()
       << ":" << dest.getPort() << endl;
    return BaseApp::sendUdpRpcCall(dest, msg, context, timeout, retries, rpcId, rpcListener);
}


void TrafficMixer::sendExitRequest(OverlayKey circuitID, UDPCall* udpCall) {
    printLog("sendExitRequest");

    TransportAddress target = udpCall->getTargetAddress();
    int nonce = BaseApp::sendUdpRpcCall(target, udpCall);
    pendingUDPRequests[nonce] = circuitID;

    EV << "    Stored nonce: " << nonce << endl;

    storeTraffic(circuitID, target);
}


void TrafficMixer::internalHandleRpcMessage(BaseRpcMessage* msg) {
    printLog("internalHandleRpcMessage");

    BaseResponseMessage* responseMsg = dynamic_cast<BaseResponseMessage*>(msg);
    if(responseMsg != 0) {
        StoredCall retrievedCall = getCallFromResponse(responseMsg);
        if(get<0>(retrievedCall) != UNDEFINED) {
            internalHandleResponseMessage(responseMsg, retrievedCall);
            return;
        }
    }
    if(!dynamic_cast<StorePeerHandleCall*>(msg)) {
        // message came from another node
        RECORD_STATS(numUdpReceived++; bytesUdpReceived += msg->getByteLength());
    }


    OnionMessage* onionMsg = dynamic_cast<OnionMessage*>(msg);
    if(onionMsg) {
        handleOnionMessage(onionMsg);
        return;
    }

    RPC_SWITCH_START(msg)
    RPC_ON_CALL(StorePeerHandle) {
        EV << "    Received StorePeerHandleCall" << endl;
        NodeHandle handle = _StorePeerHandleCall->getHandle();
        if(relayPool.find(handle) != relayPool.end()) {
            EV << "    Peer handle is already in the pool; aborting..." << endl;
            return;
        }
        // Store peer handle in relay pool
        if(state == READY) {
            //double likelihood = 1.0 * (maxPoolSize - relayPool.size()) / maxPoolSize;
            double likelihood = 0.2;
            if(relayPool.size() < maxPoolSize || uniform(0, 1) < likelihood) {
                if(relayPool.find(handle) == relayPool.end()) {
                    // relay is not in the pool
                    if(_StorePeerHandleCall->getCommand() == loki::GET_FROM_DHT) {
                        if(addressPool.find(handle) == addressPool.end() &&
                           relayPool.find(handle) == relayPool.end()) {
                            EV << "    Storing node in the address pool..." << endl;
                            addressPool.insert(handle);
                        } else {
                            EV << "    Node is already known;"
                               << " aborting..." << endl;
                        }
                    } else {
                        EV << "    Storing node in the relay pool..." << endl;
                        if(addressPool.find(handle) != addressPool.end()) {
                            addressPool.erase(handle);
                        }
                        Certificate cert = _StorePeerHandleCall->getCert();
                        addRelayToPool(handle, cert);
                    }
                } else {
                    EV << "    Node is already in relay pool" << endl;
                }
            } else {
                EV << "    Ignoring request..." << endl;
            }
        } else {
            EV << "    Node state != READY; aborting..." << endl;
        }
        delete msg;
        return;
    }
    RPC_ON_RESPONSE(NotifyCircuit) {
        handleNotifyCircuitResponse(_NotifyCircuitResponse);
        return;
    }
    RPC_ON_RESPONSE(UDP) {
        handleUDPResponse(_UDPResponse);
    }
    RPC_DELEGATE(CreateCircuit, handleCreateCircuitCall)
    RPC_DELEGATE(ExtendCircuit, handleExtendCircuitCall)
    RPC_DELEGATE(GetEvidence, handleGetEvidenceCall);
    RPC_SWITCH_END( )
}


void TrafficMixer::internalHandleRpcTimeout(BaseCallMessage* msg,
                                            const TransportAddress& dest,
                                            cObject* context,
                                            int rpcId,
                                            const OverlayKey& destKey) {
    printLog("internalHandleRpcTimeout");
    assert(false);
}


void TrafficMixer::internalHandleResponseMessage(BaseResponseMessage* msg, StoredCall retrievedCall) {
    printLog("internalHandleResponseMessage");
    CallType type = get<0>(retrievedCall);
    BaseCallMessage* call = get<1>(retrievedCall);

    switch(type) {
    case UNDEFINED:
    {
        EV << "    The received response doesn't match a stored call; "
           << "aborting..." << endl;
        return;
    }
    case PUT_CERTIFICATE:
    {
        PutCertificateCall* castedCall = check_and_cast<PutCertificateCall*>(call);
        PutCertificateResponse* castedResponse = check_and_cast<PutCertificateResponse*>(msg);
        handlePutCertificateResponse(castedResponse, castedCall);
        break;
    }
    case GET_CERTIFICATE:
    {
        GetCertificateCall* castedCall = check_and_cast<GetCertificateCall*>(call);
        GetCertificateResponse* castedResponse = check_and_cast<GetCertificateResponse*>(msg);
        handleGetCertificateResponse(castedResponse, castedCall);
        break;
    }
    case PUT_EVIDENCE:
    {
        PutEvidenceCall* castedCall = check_and_cast<PutEvidenceCall*>(call);
        PutEvidenceResponse* castedResponse = check_and_cast<PutEvidenceResponse*>(msg);
        handlePutEvidenceResponse(castedResponse, castedCall);
        break;
    }
    case GET_EVIDENCE:
    {
        GetEvidenceCall* castedCall = check_and_cast<GetEvidenceCall*>(call);
        GetEvidenceResponse* castedResponse = check_and_cast<GetEvidenceResponse*>(msg);
        handleGetEvidenceResponse(castedResponse, castedCall);
        break;
    }
    }
}


void TrafficMixer::handleReadyMessage(CompReadyMessage* msg) {
    printLog("handleReadyMessage");
    if(msg->getReady() && msg->getComp() == OVERLAY_COMP) {
        // overlay is ready; set own overlay key and start application
        EV << "    Overlay is ready; setting overlay key and storing certificate..." << endl;
        OverlayKey ownKey = overlay->getThisNode().getKey();
        thisNode.setKey(ownKey);
        thisNode.setPort(applicationPort);
        storeOwnCertificate();
    } else {
        EV << "    Comp: " << msg->getComp() << endl;
        EV << "    Ready: " << msg->getReady() << endl;
    }
}


bool TrafficMixer::handleRpcCall(BaseCallMessage* msg) {
    printLog("handleRpcCall");

    RPC_SWITCH_START( msg )
    RPC_DELEGATE(CreateCircuit, handleCreateCircuitCall)
    RPC_DELEGATE(ExtendCircuit, handleExtendCircuitCall)
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}


void TrafficMixer::handleRpcResponse(BaseResponseMessage* msg, const RpcState& state,
                                     simtime_t rtt) {
//    printLog("handleRpcResponse");
//
//    RPC_SWITCH_START( msg )
//    RPC_ON_RESPONSE( GetEvidence ) {
//        handleGetEvidenceResponse(_GetEvidenceResponse);
//    }
//    RPC_ON_RESPONSE( GetCertificate ) {
//        handleGetCertificateResponse(_GetCertificateResponse);
//    }
//    RPC_ON_RESPONSE( PutEvidence ) {
//        handlePutEvidenceResponse(_PutEvidenceResponse);
//    }
//    RPC_ON_RESPONSE( PutCertificate ) {
//        handlePutCertificateResponse(_PutCertificateResponse);
//    }
//    RPC_SWITCH_END()
}


void TrafficMixer::deliver(OverlayKey& key, cMessage* msg) {
    printLog("deliver");

}


void TrafficMixer::handleUDPMessage(cMessage* msg) {
    printLog("handleUDPMessage");
    OnionMessage* onion = dynamic_cast<OnionMessage*>(msg);
    if(onion) {
        handleOnionMessage(onion);
    }

    RPC_SWITCH_START(msg)
    RPC_ON_CALL(CreateCircuit) {
        handleCreateCircuitCall(_CreateCircuitCall);
    }
    RPC_ON_CALL(ExtendCircuit) {
        handleExtendCircuitCall(_ExtendCircuitCall);
    }
    RPC_ON_CALL(KeepAlive) {
        handleKeepAliveCall(_KeepAliveCall);
    }
    RPC_ON_CALL(RelayDisconnect) {
        handleRelayDisconnectCall(_RelayDisconnectCall);
    }
    RPC_ON_CALL(DestroyCircuit) {
        handleDestroyCircuitCall(_DestroyCircuitCall);
    }
    RPC_ON_RESPONSE(NotifyCircuit) {
        handleNotifyCircuitResponse(_NotifyCircuitResponse);
    }
    RPC_ON_RESPONSE(KeepAlive) {
        handleKeepAliveResponse(_KeepAliveResponse);
    }
    RPC_SWITCH_END( )

    if(!rpcHandled) {
        EV << "    Received an unknown packet; aborting..." << endl;
    }
}


void TrafficMixer::handleDataReceived(TransportAddress address, cPacket* msg, bool urgent) {
    printLog("handleDataReceived");

    UDPResponse* response = dynamic_cast<UDPResponse*>(msg);
    OnionMessage* onionMsg = dynamic_cast<OnionMessage*>(msg);

    if(response) {
        handleUDPResponse(response);
    } else if(onionMsg) {
        handleOnionMessage(onionMsg);
    } else {
        EV << "    An unknown packet was received from "
           << address.getIp().str() << ":" << address.getPort()
           << "; aborting" << endl;
    }
}


void TrafficMixer::handleGetEvidenceResponse(GetEvidenceResponse* msg, GetEvidenceCall* call) {
    printLog("handleGetEvidenceResponse");
    OverlayKey searchedCircuitID = call->getCircuitID();
    CircuitEvidence evidence = msg->getEvidence();
    evidence.setCircuitID(searchedCircuitID);
    msg->setEvidence(evidence);
    sendMessageToUDP(TrustedAuthority::address, msg);
    delete call;
}


void TrafficMixer::handleGetCertificateResponse(GetCertificateResponse* msg, GetCertificateCall* call) {
    printLog("handleGetCertificateResponse");

    NodeHandle handle = call->getNode();
    Certificate cert = msg->getCert();
    if(cert != Certificate()) {
        EV << "    Certificate is defined; adding it to the relay pool..." << endl;
        addRelayToPool(handle, cert);
    } else {
        EV << "    Certificate is undefined; aborting..." << endl;
    }
    delete msg;
    delete call;
}


void TrafficMixer::handlePutEvidenceResponse(PutEvidenceResponse* msg, PutEvidenceCall* call) {
    printLog("handlePutEvidenceResponse");
    delete msg;
    delete call;
}


void TrafficMixer::handlePutCertificateResponse(PutCertificateResponse* msg, PutCertificateCall* call) {
    printLog("handlePutCertificateResponse");

    if(!msg->getIsSuccess() && certStorageRetries < maxCertRetries) {
        // own certificate wasn't stored, try again
        EV << "    Put operation Unsuccessful; retrying..." << endl;
        certStorageRetries++;
        storeOwnCertificate();
    } else if(certStorageRetries >= maxCertRetries) {
        // can't store the certificate at this time. Shutdown...
        EV << "    Maximum attempts reached; SHUTDOWN..." << endl;
        changeState(SHUTDOWN);
    } else {
        EV << "    Put operation Successful" << endl;
        // check if the stored certificate is this node's
        Certificate cert = call->getCert();
        Certificate ownCert = getOwnCertificate();
        if(cert == ownCert) {
            EV << "    Own certificate was stored; state = READY" << endl;
            isOwnCertificateShared = true;
            changeState(READY);
        }
    }

    delete msg;
    delete call;

}


void TrafficMixer::handleCreateCircuitCall(CreateCircuitCall* msg) {
    printLog("handleCreateCircuitCall");
    string privateKey = getOwnCertificate().getKeySet().getPrivateKey();
    EV << "    Generating Circuit information..." << endl;
    CircuitRelay* relay = new CircuitRelay(this, msg, privateKey);
    OverlayKey circuitID = relay->getCircuitID();
    EV << "    Storing CircuitID: " << circuitID.toString() << endl;
    extCircuits[circuitID] = relay;

    RECORD_STATS(numBuildCircuitReceived++;
                 bytesBuildCircuitReceived += msg->getByteLength());


    StorePeerHandleCall* msgToTier3 = new StorePeerHandleCall();
    msgToTier3->setHandle(msg->getSrcNode());
    msgToTier3->setCert(msg->getCreatorCert());
    msgToTier3->setCommand(loki::STORE_LOCALLY);
    sendInternalRpcCall(TIER3_COMP, msgToTier3);
}

void TrafficMixer::handleExtendCircuitCall(ExtendCircuitCall* msg) {
    printLog("handleExtendCircuitCall");
    NodeHandle srcNode = msg->getSrcNode();
    Certificate prevNodeCert = msg->getPrevNodeCert();

    RECORD_STATS(numBuildCircuitReceived++;
                 bytesBuildCircuitReceived += msg->getByteLength());

    string privateKey = getOwnCertificate().getKeySet().getPrivateKey();
    EV << "    Generating relay information..." << endl;
    CircuitRelay* relay = new CircuitRelay(this, msg, privateKey);
    OverlayKey circuitID = relay->getCircuitID();
    EV << "    Storing CircuitID: " << circuitID.toString() << endl;
    extCircuits[circuitID] = relay;

    StorePeerHandleCall* msgToTier3 = new StorePeerHandleCall();
    msgToTier3->setHandle(srcNode);
    msgToTier3->setCert(prevNodeCert);
    msgToTier3->setCommand(loki::STORE_LOCALLY);
    sendInternalRpcCall(TIER3_COMP, msgToTier3);
}

void TrafficMixer::handleOnionMessage(OnionMessage* msg) {
    RECORD_STATS(numOnionReceived++;
                 bytesOnionReceived += msg->getByteLength());
    printLog("handleOnionMessage");
    OverlayKey circuitID = msg->getCircuitID();
    if(ownCircuits.find(circuitID) != ownCircuits.end()) {
        EV << "    Received OnionMessage from own circuit" << endl;
        CircuitManager manager = ownCircuits[circuitID];
        cPacket* payload = manager.unwrapPayload(msg);
        handleOnionResponse(payload, circuitID);
    } else if(extCircuits.find(circuitID) != extCircuits.end()) {
        EV << "    Received OnionMessage from external circuit" << endl;
        extCircuits[circuitID]->handleOnionMessage(msg);
    } else if(isCircuitBuilding) {
        EV << "    Received OnionMessage from pending circuit" << endl;
        cPacket* payload = pendingCircuit.unwrapPayload(msg);
        if(payload) {
            handleOnionResponse(payload, circuitID);
        } else {
            EV << "    Payload failed to decapsulate; aborting..." << endl;
            //stopBuildingCircuit(false);
            delete payload;
        }
    } else {
        EV << "    Received unknown OnionMessage; aborting..." << endl;
        delete msg;
    }
}

void TrafficMixer::handleKeepAliveCall(KeepAliveCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    extCircuits[circuitID]->propagateKeepAliveCall();
}

void TrafficMixer::handleRelayDisconnectCall(RelayDisconnectCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    NodeHandle disconnectedNode = msg->getDisconnectedNode();
    extCircuits[circuitID]->propagateRelayDisconnectCall(disconnectedNode);
}

void TrafficMixer::handleDestroyCircuitCall(DestroyCircuitCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    extCircuits[circuitID]->propagateDestroyCircuitCall();
    extCircuits.erase(circuitID);
}

void TrafficMixer::handleNotifyCircuitResponse(NotifyCircuitResponse* msg) {
    printLog("handleNotifyCircuitResponse");
    OverlayKey circuitID = msg->getPrevCircuitID();
    assert(extCircuits.find(circuitID) != extCircuits.end());

    RECORD_STATS(numBuildCircuitReceived++;
                 bytesBuildCircuitReceived += msg->getByteLength());

    CircuitRelay* relay = extCircuits[circuitID];
    relay->handleNotifyCircuitResponse(msg);
    extCircuits[relay->getNextCircuitID()] = relay;
    delete msg;
}

void TrafficMixer::handleKeepAliveResponse(KeepAliveResponse* msg) {
    printLog("handleKeepAliveResponse");
}

void TrafficMixer::handleOnionResponse(cPacket* msg, OverlayKey circuitID) {
    printLog("handleOnionResponse");
    UDPResponse* udpMsg = dynamic_cast<UDPResponse*>(msg);
    BuildCircuitResponse* bcrMsg = dynamic_cast<BuildCircuitResponse*>(msg);
    if(udpMsg) {
        EV << "    Received a response from the target server" << endl;
        uint64_t internalID = udpMsg->getInternalID();
        if(pendingOnionRequestsTimes.find(internalID) !=
           pendingOnionRequestsTimes.end()) {
            RECORD_STATS(numTargetSuccess++;
                         bytesTargetReceived += udpMsg->getByteLength());
            simtime_t sendTime = pendingOnionRequestsTimes[internalID];
            simtime_t elapsedTime = simTime() - sendTime;
            requestLatencies.record(elapsedTime);
            pendingOnionRequestsTimes.erase(internalID);
        }
        delete msg;
        cancelEvent(requestTimer);
        scheduleAt(simTime() + SimTime::parse("5s"), requestTimer);
    } else if(bcrMsg) {
        handleBuildCircuitResponse(bcrMsg, circuitID);
    } else {
        EV << "    Received an unrecognised payload" << endl;
        delete msg;
    }
}


void TrafficMixer::handleBuildCircuitResponse(BuildCircuitResponse* msg, OverlayKey circuitID) {
    printLog("handleBuildCircuitResponse");
    bool isCreateResponse = pendingCircuit.circuitOrder.size() == 0;
    bool isBootstrapCircuit = ownCircuits.size() == 0;
    bool isCircuitBuilt = pendingCircuit.handleBuildCircuitResponse(msg);
    if(!isCircuitBuilt) {
        EV << "    The circuit failed building; aborting..." << endl;
        stopBuildingCircuit(false);
        if(isCreateResponse) {
            RECORD_STATS(numCreateFailure++);
        } else {
            RECORD_STATS(numExtendFailure++);
        }
        delete msg;
        return;
    }
    if(isCreateResponse) {
        RECORD_STATS(numCreateSuccess++);
    } else {
        RECORD_STATS(numExtendSuccess++);
    }

    if(selectedNodes.size() > 0) {
        // the circuit building process wasn't finished
        cancelEvent(buildCircuitTimer);
        scheduleAt(simTime(), buildCircuitTimer);
    } else {
        // the circuit building process was completed
        if(isBootstrapCircuit) {
            simtime_t bootstrapTime = simTime() - overlayReadyTime;
            globalStatistics->recordHistogram("TrafficMixer: Bootstrap Time",
                                              bootstrapTime.dbl());
        }
        simtime_t buildingTime = simTime() - circuitBuildingBeginTime;
        circuitBuildingTime.record(buildingTime);

        stopBuildingCircuit();
        cancelEvent(requestTimer);
        scheduleAt(simTime(), requestTimer);
    }
    delete msg;
}


void TrafficMixer::handleUDPResponse(UDPResponse* msg) {
    printLog("handleTCPResponse");

    RECORD_STATS(numExitResponse++;
                 bytesExitResponse += msg->getByteLength());

    int nonce = msg->getNonce();
    if(pendingUDPRequests.find(nonce) == pendingUDPRequests.end()) {
        EV << "    The received nonce doesn't correspond to a pending call; "
           << "aborting..." << endl;
        return;
    }
    OverlayKey circuitID = pendingUDPRequests[nonce];
    pendingUDPRequests.erase(nonce);
    CircuitRelay* relay = extCircuits[circuitID];
    relay->handleExitResponse(msg);
}


void TrafficMixer::handleGetEvidenceCall(GetEvidenceCall* msg) {
    getEvidence(msg->getCircuitID());
}


void TrafficMixer::getRandomPeerCertificate() {
    printLog("getRandomPeerCertificate");

    cancelEvent(peerLookupTimer);

    if(addressPool.size() == 0) {
        EV << "    The address pool is empty; rescheduling..." << endl;
        scheduleAt(simTime() + SimTime::parse("5s"), peerLookupTimer);
        return;
    }

    bool isNewHandleFound = false;

    set<NodeHandle> tempPool = addressPool;
    set<NodeHandle>::iterator it;
    NodeHandle randHandle;
    while(!isNewHandleFound && tempPool.size() > 0) {
        it = tempPool.begin();
        int randInt = intuniform(0, tempPool.size() - 1);
        advance(it, randInt);
        randHandle = *it;
        if(relayPool.find(randHandle) == relayPool.end()) {
            isNewHandleFound = true;
        } else {
            tempPool.erase(it);
        }
    }

    if(isNewHandleFound) {
        randHandle.setPort(applicationPort);
        getCertificate(randHandle);
    }

    EV << "    Scheduling next lookup event...";
    if(relayPool.size() >= maxPoolSize || !isNewHandleFound) {
        scheduleAt(simTime() + SimTime::parse("30s"), peerLookupTimer);
    } else {
        scheduleAt(simTime() + SimTime::parse("5s"), peerLookupTimer);
    }
}


/**
 * Sends a PutEvidenceCall to the DHT
 *
 * @param evidence CircuitEvidence the evidence packet to be stored
 */
void TrafficMixer::putEvidence(CircuitEvidence evidence) {
    printLog("putEvidence");
    EV << "    CircuitID: " << evidence.getCircuitID().toString(16) << endl;

    PutEvidenceCall* call = new PutEvidenceCall();
    call->setEvidence(evidence);
    int nonce = sendInternalRpcCall(TIER2_COMP, call);
    putCallInPendingList(call, nonce);
}

/**
 * Sends a PutCertificateCall to the DHT
 *
 * @param cert Certificate the certificate to be stored
 * @param node NodeHandle the node owning the certificate
 */
void TrafficMixer::putCertificate(Certificate cert, NodeHandle handle) {
    printLog("putCertificate");
    EV << "    Node: " + handle.getIp().str() << endl;

    PutCertificateCall* call = new PutCertificateCall();
    call->setCert(cert);
    call->setHandle(handle);
    int nonce = sendInternalRpcCall(TIER2_COMP, call);
    putCallInPendingList(call, nonce);
}

/**
 * Sends a GetEvidenceCall to the DHT
 *
 * @param circuitID OverlayKey the ID corresponding to a created/extended circuit
 */
void TrafficMixer::getEvidence(OverlayKey circuitID) {
    string debugMsg = "\n    CircuitID: " + circuitID.toString(16);
    printLog("getEvidence", debugMsg);
    GetEvidenceCall* call = new GetEvidenceCall();
    call->setCircuitID(circuitID);
    int nonce = sendInternalRpcCall(TIER2_COMP, call);
    putCallInPendingList(call, nonce);
}

/**
 * Sends a GetCertificateCall to the DHT
 *
 * @param handle NodeHandle the handle of the node for which the certificate is
 *                          being requested
 */
void TrafficMixer::getCertificate(NodeHandle handle) {
    string debugMsg = "\n    Node: " + handle.getIp().str();
    printLog("getCertificate", debugMsg);
    GetCertificateCall* call = new GetCertificateCall();
    call->setNode(handle);
    int nonce = sendInternalRpcCall(TIER2_COMP, call);
    putCallInPendingList(call, nonce);
}

UDPCall* TrafficMixer::buildUDPCall() {
    UDPCall* udpMsg = new UDPCall();
    udpMsg->setSenderAddress(thisNode);
    udpMsg->setByteLength(100);
    udpMsg->setKind(4);
    return udpMsg;
}

UDPResponse* TrafficMixer::buildUDPResponse() {
    UDPResponse* udpMsg = new UDPResponse();
    udpMsg->setByteLength(100);
    udpMsg->setKind(4);
    return udpMsg;
}

void TrafficMixer::sendUDPPing(TransportAddress* addr) {
    printLog("sendTcpPing");

    TransportAddress remoteAddress = TransportAddress(addr->getIp(), 24000);
    UDPCall* tcpMsg = buildUDPCall();
    establishTcpConnection(remoteAddress);
    sendTcpData(tcpMsg, remoteAddress);
    EV << thisNode.getIp() << ": Connecting to "<< addr->getIp()
       << " and sending PING."<< std::endl;
}

void TrafficMixer::handleUDPPong(UDPResponse* udpMsg) {
    if(pendingUDPRequests.find(udpMsg->getNonce()) == pendingUDPRequests.end()) {
        return;
    }
    OverlayKey circuitID = pendingUDPRequests[udpMsg->getNonce()];
    CircuitRelay* relay = extCircuits[circuitID];
    relay->handleExitResponse(udpMsg);
}


void TrafficMixer::storeTraffic(OverlayKey circuitID, TransportAddress target) {
    TrafficRecord record = TrafficRecord();
    record.setCircuitID(circuitID);
    record.setTarget(target);
    record.setTimestamp(simTime());
    exitTrafficHistory.push_back(record);
}


void TrafficMixer::releaseTrafficHistory() {
    printLog("releaseTrafficHistory");

    if(exitTrafficHistory.size() > 0) {
        EV << "    Sending traffic report..." << endl;
        TrafficReport* report = new TrafficReport();

        report->setRecordsArraySize(exitTrafficHistory.size());
        for(size_t i = 0; i < exitTrafficHistory.size(); i++) {
            report->setRecords(i, exitTrafficHistory[i]);
        }
        sendUdpRpcCall(TrustedAuthority::address, report);
        exitTrafficHistory = vector<TrafficRecord>();
    } else {
        EV << "    No recorded traffic history; rescheduling..." << endl;
    }
    cancelEvent(trafficReportTimer);
    scheduleAt(simTime() + SimTime::parse("60s"), trafficReportTimer);
}
