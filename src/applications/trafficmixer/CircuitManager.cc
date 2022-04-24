#include "CircuitManager.h"
#include "applications/trafficmixer/TrafficMixer.h"


CircuitManager::CircuitManager() {
    mix = nullptr;
    pendingRelay = {NodeHandle::UNSPECIFIED_NODE, ""};
}


CircuitManager::CircuitManager(TrafficMixer* mix) {
    this->mix = mix;
}


void CircuitManager::sendRequest(cPacket* msg) {
    printLog("sendRequest");

    OnionMessage* onion = wrapPacket(msg);
    TransportAddress entryNode = (TransportAddress)getEntryNode();
    mix->sendUdpRpcCall(entryNode, onion);
}


void CircuitManager::create(NodeHandle entryNode, Certificate cert, string key) {
    printLog("create");

    CreateCircuitCall* call = new CreateCircuitCall();
    KeyExchange keyExchange = KeyExchange(cert, key);
    pendingRelay = {entryNode, key};
    call->setKeyExchange(keyExchange);
    call->setCreatorCert(cert);

    EV << "    Sending create signal to "
       << entryNode.getIp() << ":" << entryNode.getPort() << endl;
    EV << "    Key:" << key << endl;
    mix->sendUdpRpcCall(entryNode, call);
}


void CircuitManager::extend(NodeHandle nextNode, Certificate cert, string key) {
    printLog("extend");

    TriggerExtensionCall* call = new TriggerExtensionCall();
    KeyExchange keyExchange = KeyExchange(cert, key);
    pendingRelay = {nextNode, key};
    call->setKeyExchange(keyExchange);
    call->setNextNode(nextNode);
    sendRequest(call);
}


void CircuitManager::keepAlive() {
    printLog("keepAlive");

    KeepAliveCall* call = new KeepAliveCall();
    call->setCircuitID(getCircuitIDAtPosition(0));

    TransportAddress entryNode = (TransportAddress)getEntryNode();
    mix->sendUdpRpcCall(entryNode, call);
}


void CircuitManager::destroy() {
    printLog("destroy");

    DestroyCircuitCall* prevCall = nullptr;

    for(int i = circuitOrder.size() - 1; i >= 0; i--) {
        DestroyCircuitCall* destroyCircuit = new DestroyCircuitCall();
        destroyCircuit->setCircuitID(getCircuitIDAtPosition(i));
        if(prevCall) {
            destroyCircuit->encapsulate(prevCall);
        }
        prevCall = destroyCircuit;
    }

    if(prevCall) {
        mix->sendUdpRpcCall(getEntryNode(), prevCall);
    }
}


void CircuitManager::handleFailure(NodeHandle failedNode) {
    printLog("handleFailure");

    if(involvedNodes.find(failedNode) == involvedNodes.end()) {
        return;
    }

    bool isNodeFound = false;
    list<OverlayKey>::iterator it = circuitOrder.begin();
    while(it != circuitOrder.end()) {
        // check if node is the failed one
        if(isNodeFound) {
            OverlayKey deletionKey = *it;
            NodeHandle node = getNodeFromOverlayKey(deletionKey);
            involvedNodes.erase(node);
            circuitData.erase(deletionKey);
            it = circuitOrder.erase(it);
        } else {
            NodeHandle node = getNodeFromOverlayKey(*it);
            if(node == failedNode) {
                isNodeFound = true;
            } else {
                it++;
            }
        }
    }
}


void CircuitManager::handleBuildCircuitResponse(BuildCircuitResponse* msg) {
    printLog("handleBuildCircuitResponse");

    TransportAddress nextNode = msg->getSrcNode();
    OverlayKey circuitID = msg->getCircuitID();

    if(pendingRelay.first.isUnspecified()) {
        EV << "    Received a BuildCircuitResponse from "
           << nextNode.getIp().str() << ":" << nextNode.getPort()
           << " but there is no pending relay; aborting..." << endl;
        return;
    } else if(nextNode.isUnspecified()) {
        EV << "    Received a BuildCircuitResponse from an unspecified node --> "
           << nextNode.getIp().str() << ":" << nextNode.getPort()
           << "; aborting..." << endl;
        return;
    }

    if(pendingRelay.first != nextNode) {
        EV << "    Received a BuildCircuitResponse from "
           << nextNode.getIp().str() << ":" << nextNode.getPort()
           << " while the queried node was " << pendingRelay.first.getIp().str()
           << ":" << pendingRelay.first.getPort() << "; aborting..." << endl;
        return;
    }

    RelayInfo relayInfo(circuitID, nextNode, pendingRelay.second);
    pendingRelay = {TransportAddress::UNSPECIFIED_NODE, ""};
    involvedNodes.insert(nextNode);
    circuitOrder.push_back(circuitID);
    circuitData[circuitID] = relayInfo;
}


cPacket* CircuitManager::unwrapPayload(OnionMessage* msg) {
    printLog("unwrapPayload");
    cPacket* content = nullptr;
    OnionMessage* onion = msg;

    while(onion) {
        EV << "    Peeling Onion at pointer:" << onion << endl;
        content = peelOnionMessage(onion);
        onion = dynamic_cast<OnionMessage*>(content);
    }

    return content;
}


void CircuitManager::handleUDPResponse(UDPResponse* response) {
    printLog("handleTcpResponse");
}


cPacket* CircuitManager::peelOnionMessage(OnionMessage* msg) {
    printLog("peelOnionMessage");
    OverlayKey circuitID = msg->getCircuitID();
    NodeHandle srcNode = msg->getSrcNode();
    srcNode.setPort(mix->applicationPort);
    string key = "";

    EV << "    Unwrapping OnionMessage from: " << srcNode.getIp().str()
       << ":" << srcNode.getPort() << endl;
    if(circuitData.find(circuitID) != circuitData.end()) {
        key = getKeyFromCircuitID(circuitID);
        EV << "    The circuitID exists in local storage" << endl
           << "    CircuitID: " << circuitID.toString() << endl
           << "    Session Key: " << key << endl;
    } else if(!pendingRelay.first.isUnspecified() &&
              (TransportAddress)pendingRelay.first == (TransportAddress)srcNode) {
        EV << "    The source node corresponds to the pending relay" << endl;
        key = pendingRelay.second;
    } else {
        EV << "    Unrecognised source node and circitID:" << endl
           << "        Pending Relay: " << pendingRelay.first.getIp().str()
           << ":" << pendingRelay.first.getPort() << endl
           << "        Exchanged key: " << pendingRelay.second;
    }

    return msg->peel(key);
}


OnionMessage* CircuitManager::wrapPacket(cPacket* msg) {
    assert(circuitOrder.size() > 0);
    OnionMessage* prevMsg = nullptr;
    TrafficDirection direction = OUTWARD;

    for(int i = circuitOrder.size() - 1; i >= 0; i--) {
        RelayInfo data = getRelayInfoAtPosition(i);
        OverlayKey circuitID = get<0>(data);
        string encryptionKey = get<2>(data);
        OnionMessage* builtMsg = new OnionMessage(direction, circuitID, encryptionKey);
        if(prevMsg) {
            builtMsg->encapsulate(prevMsg);
        } else {
            builtMsg->encapsulate(msg);
        }
        prevMsg = builtMsg;
    }

    return prevMsg;
}


OverlayKey CircuitManager::getCircuitIDAtPosition(int position) {
    if(position >= (int)circuitOrder.size()) {
        return OverlayKey::UNSPECIFIED_KEY;
    }

    list<OverlayKey>::iterator it = circuitOrder.begin();
    advance(it, position);
    return *it;
}


RelayInfo CircuitManager::getRelayInfoAtPosition(int position) {
    OverlayKey circuitID = getCircuitIDAtPosition(position);
    if(circuitID.isUnspecified() ||
       circuitData.find(circuitID) == circuitData.end()) {
        RelayInfo noResult(circuitID, NodeHandle::UNSPECIFIED_NODE, "");
        return noResult;
    }
    return circuitData[circuitID];
}


NodeHandle CircuitManager::getNodeAtPosition(int position) {
    RelayInfo data = getRelayInfoAtPosition(position);
    return get<1>(data);
}


NodeHandle CircuitManager::getNodeFromOverlayKey(OverlayKey key) {
    if(circuitData.find(key) == circuitData.end()) {
        return NodeHandle::UNSPECIFIED_NODE;
    }

    RelayInfo data = circuitData[key];
    return get<1>(data);
}


NodeHandle CircuitManager::getEntryNode() {
    return getNodeAtPosition(0);
}


string CircuitManager::getKeyFromCircuitID(OverlayKey circuitID) {
    if(circuitData.find(circuitID) != circuitData.end()) {
        RelayInfo data = circuitData[circuitID];
        return get<2>(data);
    } else {
        return "";
    }
}


string CircuitManager::getKeyFromOnionMessage(OnionMessage* msg) {
    string key = "";
    if(circuitData.find(msg->getCircuitID()) != circuitData.end()) {
        key = getKeyFromCircuitID(msg->getCircuitID());
    } else if(!pendingRelay.first.isUnspecified() &&
              msg->getSrcNode() == pendingRelay.first) {
        key = pendingRelay.second;
    }

    return key;
}


void CircuitManager::printLog(string functionName) {
    mix->printLog("CircuitManager::" + functionName);
}
