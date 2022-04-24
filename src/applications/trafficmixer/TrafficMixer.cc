#include "TrafficMixer.h"
#include "common/GlobalNodeList.h"
#include "overlay/permissionedchord/ChordMessage_m.h"

using namespace std;
using loki::StorePeerHandleCall;
using loki::StoreOwnCertificateCall;


// CIRCUIT HANDLING
//void Circuit::destroy() {
//    // Send DestroyCircuitCall to nodes
//}
//
//void Circuit::handleOnionMessage(OnionMessage* msg) {
//    if(isExitNode())
//        handleExitRequest(msg);
//    else
//        relayMessage(msg);
//
//    TrafficMixer::sendUdpRpcCall(dest, msg);
//}
//
//void Circuit::handleExitRequest(OnionMessage* msg) {
//    cPacket* packet = msg->peel(symmetricKey);
//    if(packet) {
//
//    }
//}
//
//void Circuit::relayMessage(OnionMessage* msg) {
//
//}

// END OF CIRCUIT HANDLING

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
    maxPoolSize = 5;
    maxOpenCircuits = 1;
    circuitLength = 2;

    isBuildingCircuit = false;


    // statistics
    numSent = 0;
    numRelayed = 0;
    numPutCalls = 0;
    numGetCalls = 0;

    WATCH(numSent);
    WATCH(numRelayed);
    WATCH(numPutCalls);
    WATCH(numGetCalls);
    WATCH(isOwnCertificateShared);
    WATCH(maxPoolSize);
    WATCH_MAP(relayPool);

    state = INIT;
    applicationPort = 31415;

    requestTimer = new cMessage("Request Timer");
    buildCircuitTimer = new cMessage("Build Circuit Timer");

    bindToPort(applicationPort);
    bindAndListenTcp(TargetServer::tcpPort);
}


void TrafficMixer::finishApp() {
    changeState(SHUTDOWN);
}


void TrafficMixer::handleTimerEvent(cMessage* msg) {
    printLog("handleTimerEvent");
    if(msg == requestTimer) {
        sendRequestToTarget();
        return;
    } else if(msg == buildCircuitTimer) {
        buildCircuit();
        return;
    }
}


void TrafficMixer::sendRequestToTarget() {
    printLog("sendRequestToTarget");
    if(ownCircuits.size() == 0) {
        cancelEvent(requestTimer);
        scheduleAt(simTime() + SimTime(intuniform(5, 15) * 60, SIMTIME_S), requestTimer);
        return;
    }

    TCPCall* request = new TCPCall();
    request->setTargetAddress(TargetServer::address);
    int randInt = intuniform(0, ownCircuits.size() - 1);
    map<OverlayKey, CircuitManager>::iterator it = ownCircuits.begin();
    advance(it, randInt);
    it->second.sendRequest(request);
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

    if(!isBuildingCircuit && ownCircuits.size() < maxOpenCircuits) {
        buildNewCircuit();
    } else if(isBuildingCircuit) {
        extendCircuit();
    }

}


void TrafficMixer::extendCircuit() {
    printLog("extendCircuit");
    if(selectedNodes.size() > 0) {
        isBuildingCircuit = true;
        NodeHandle nextNode = getNextSelectedRelay();
        nextNode.setPort(applicationPort);

        Certificate cert = getOwnCertificate();
        string key = generateRandomKey(uniform(0, 1));
        pendingCircuit.extend(nextNode, cert, key);
    }
}


void TrafficMixer::buildNewCircuit() {
    printLog("buildNewCircuit");

    isBuildingCircuit = true;
    selectedNodes = getNRandomRelays(circuitLength);

    EV << "    Selected nodes: [" << endl;
    for(auto node: selectedNodes)
        EV << "        " << node.getIp() << ":" << node.getPort() << endl;

    EV << "    ]" << endl;

    NodeHandle entryNode = getNextSelectedRelay();
    entryNode.setPort(applicationPort);

    EV << "    Entry node: " << entryNode.getIp()
       << ":" << entryNode.getPort() << endl;
    Certificate cert = getOwnCertificate();
    string key = generateRandomKey(uniform(0, 1));

    pendingCircuit = CircuitManager(this);
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


void TrafficMixer::internalHandleRpcMessage(BaseRpcMessage* msg) {
    printLog("internalHandleRpcMessage");

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
                        EV << "    Getting node certificate..." << endl;
                        getCertificate(handle);
                    } else {
                        EV << "    Storing node information..." << endl;
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
        return;
    }
    RPC_ON_RESPONSE(NotifyCircuit) {
        handleNotifyCircuitResponse(_NotifyCircuitResponse);
        return;
    }
    RPC_DELEGATE(CreateCircuit, handleCreateCircuitCall)
    RPC_DELEGATE(ExtendCircuit, handleExtendCircuitCall)
    RPC_SWITCH_END( )

    if(!rpcHandled) {
        BaseResponseMessage* responseMsg = dynamic_cast<BaseResponseMessage*>(msg);
        if(responseMsg != 0) {
            StoredCall retrievedCall = getCallFromResponse(responseMsg);
            internalHandleResponseMessage(responseMsg, retrievedCall);
            return;
        }
    }
}


void TrafficMixer::internalHandleRpcTimeout(BaseCallMessage* msg,
                                            const TransportAddress& dest,
                                            cObject* context,
                                            int rpcId,
                                            const OverlayKey& destKey) {
    printLog("internalHandleRpcTimeout");
    // TODO
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

    TCPResponse* response = dynamic_cast<TCPResponse*>(msg);
    OnionMessage* onionMsg = dynamic_cast<OnionMessage*>(msg);

    if(response) {
        handleTCPResponse(response);
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
    // common nodes can't do anything with evidence
    delete call;
    delete msg;
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
}


void TrafficMixer::handlePutEvidenceResponse(PutEvidenceResponse* msg, PutEvidenceCall* call) {
    printLog("handlePutEvidenceResponse");
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
    CircuitRelay relay = CircuitRelay(this, msg, privateKey);
    OverlayKey circuitID = relay.getCircuitID();
    EV << "    Storing CircuitID: " << circuitID.toString() << endl;
    extCircuits[circuitID] = relay;

    // TODO: uncomment
//    StorePeerHandleCall* msgToTier3 = new StorePeerHandleCall();
//    msgToTier3->setHandle(msg->getSrcNode());
//    msgToTier3->setCert(msg->getCreatorCert());
//    msgToTier3->setCommand(loki::STORE_LOCALLY);
//    sendInternalRpcCall(TIER3_COMP, msgToTier3);
}

void TrafficMixer::handleExtendCircuitCall(ExtendCircuitCall* msg) {
    printLog("handleExtendCircuitCall");
    NodeHandle srcNode = msg->getSrcNode();
    Certificate prevNodeCert = msg->getPrevNodeCert();


    string privateKey = getOwnCertificate().getKeySet().getPrivateKey();
    EV << "    Generating relay information..." << endl;
    CircuitRelay relay = CircuitRelay(this, msg, privateKey);
    OverlayKey circuitID = relay.getCircuitID();
    EV << "    Storing CircuitID: " << circuitID.toString() << endl;
    extCircuits[circuitID] = relay;

    // TODO: uncomment
//    StorePeerHandleCall* msgToTier3 = new StorePeerHandleCall();
//    msgToTier3->setHandle(srcNode);
//    msgToTier3->setCert(prevNodeCert);
//    msgToTier3->setCommand(loki::STORE_LOCALLY);
//    sendInternalRpcCall(TIER3_COMP, msgToTier3);
}

void TrafficMixer::handleOnionMessage(OnionMessage* msg) {
    printLog("handleOnionMessage");
    OverlayKey circuitID = msg->getCircuitID();
    if(ownCircuits.find(circuitID) != ownCircuits.end()) {
        EV << "    Received OnionMessage from own circuit" << endl;
        CircuitManager manager = ownCircuits[circuitID];
        cPacket* payload = manager.unwrapPayload(msg);
        handleOnionResponse(payload, circuitID);
    } else if(extCircuits.find(circuitID) != extCircuits.end()) {
        EV << "    Received OnionMessage from external circuit" << endl;
        extCircuits[circuitID].handleOnionMessage(msg);
    } else if(isBuildingCircuit) {
        EV << "    Received OnionMessage from pending circuit" << endl;
        cPacket* payload = pendingCircuit.unwrapPayload(msg);
        if(payload)
            handleOnionResponse(payload, circuitID);
        else
            EV << "    Payload failed to decapsulate; aborting..." << endl;
    } else {
        EV << "    Received unknown OnionMessage; aborting..." << endl;
    }
}

void TrafficMixer::handleKeepAliveCall(KeepAliveCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    extCircuits[circuitID].propagateKeepAliveCall();
}

void TrafficMixer::handleRelayDisconnectCall(RelayDisconnectCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    NodeHandle disconnectedNode = msg->getDisconnectedNode();
    extCircuits[circuitID].propagateRelayDisconnectCall(disconnectedNode);
}

void TrafficMixer::handleDestroyCircuitCall(DestroyCircuitCall* msg) {
    OverlayKey circuitID = msg->getCircuitID();
    if(extCircuits.find(circuitID) == extCircuits.end()) {
        return;
    }
    extCircuits[circuitID].propagateDestroyCircuitCall();
    extCircuits.erase(circuitID);
}

void TrafficMixer::handleNotifyCircuitResponse(NotifyCircuitResponse* msg) {
    printLog("handleNotifyCircuitResponse");
    OverlayKey circuitID = msg->getPrevCircuitID();
    assert(extCircuits.find(circuitID) != extCircuits.end());

    extCircuits[circuitID].handleNotifyCircuitResponse(msg);
}

void TrafficMixer::handleKeepAliveResponse(KeepAliveResponse* msg) {
    printLog("handleKeepAliveResponse");
}

void TrafficMixer::handleOnionResponse(cPacket* msg, OverlayKey circuitID) {
    printLog("handleOnionResponse");
    TCPResponse* tcpMsg = dynamic_cast<TCPResponse*>(msg);
    BuildCircuitResponse* bcrMsg = dynamic_cast<BuildCircuitResponse*>(msg);
    if(tcpMsg) {
        EV << "    Received a response from the target server" << endl;
    } else if(bcrMsg) {
        handleBuildCircuitResponse(bcrMsg, circuitID);
    } else {
        EV << "    Received an unrecognised payload" << endl;
    }
}


void TrafficMixer::handleBuildCircuitResponse(BuildCircuitResponse* msg, OverlayKey circuitID) {
    printLog("handleBuildCircuitResponse");
    pendingCircuit.handleBuildCircuitResponse(msg);
    if(selectedNodes.size() > 0) {
        // the circuit building process wasn't finished
        cancelEvent(buildCircuitTimer);
        scheduleAt(simTime(), buildCircuitTimer);
    } else {
        // the circuit building process was completed
        OverlayKey circuitID = pendingCircuit.getCircuitIDAtPosition(0);
        ownCircuits[circuitID] = pendingCircuit;
        isBuildingCircuit = false;

        cancelEvent(buildCircuitTimer);
        scheduleAt(simTime(), buildCircuitTimer);

        cancelEvent(requestTimer);
        scheduleAt(simTime(), requestTimer);
    }
}


void TrafficMixer::handleTCPResponse(TCPResponse* msg) {
    printLog("handleTCPResponse");
    int nonce = msg->getNonce();
    if(pendingTcpRequests.find(nonce) == pendingTcpRequests.end()) {
        EV << "    The received nonce doesn't correspond to a pending call; "
           << "aborting..." << endl;
        return;
    }
    OverlayKey circuitID = pendingTcpRequests[nonce];
    pendingTcpRequests.erase(nonce);
    CircuitRelay relay = extCircuits[circuitID];
    relay.handleExitResponse(msg);
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

TCPCall* TrafficMixer::buildTCPCall() {
    TCPCall* tcpMsg = new TCPCall();
    tcpMsg->setSenderAddress(thisNode);
    tcpMsg->setByteLength(100);
    tcpMsg->setKind(4);
    return tcpMsg;
}

TCPResponse* TrafficMixer::buildTCPResponse() {
    TCPResponse* tcpMsg = new TCPResponse();
    tcpMsg->setByteLength(100);
    tcpMsg->setKind(4);
    return tcpMsg;
}

void TrafficMixer::sendTcpPing(TransportAddress* addr) {
    printLog("sendTcpPing");
    RECORD_STATS(numSent++);
    RECORD_STATS(numTcpRequests++);

    TransportAddress remoteAddress = TransportAddress(addr->getIp(), 24000);
    TCPCall* tcpMsg = buildTCPCall();
    establishTcpConnection(remoteAddress);
    sendTcpData(tcpMsg, remoteAddress);
    EV << thisNode.getIp() << ": Connecting to "<< addr->getIp()
       << " and sending PING."<< std::endl;
}

void TrafficMixer::handleTcpPong(TCPResponse* tcpMsg) {
    if(pendingTcpRequests.find(tcpMsg->getNonce()) == pendingTcpRequests.end()) {
        return;
    }
    OverlayKey circuitID = pendingTcpRequests[tcpMsg->getNonce()];
    CircuitRelay relay = extCircuits[circuitID];
    relay.handleExitResponse(tcpMsg);
}
