#include "CircuitRelay.h"
#include "TrafficMixer.h"


CircuitRelay::CircuitRelay(TrafficMixer* mix) : CircuitRelay() {
    this->mix = mix;
}


CircuitRelay::CircuitRelay(TrafficMixer* mix, NodeHandle node,
                           KeyExchange keyExchange, string privateKey)
                           : CircuitRelay(mix) {
    printLog("CircuitRelay");
    EV << "    Source node: " << node.getIp() << ":" << node.getPort() << endl;
    EV << "    Key Exchange: " << keyExchange.toJsonString() << endl;
    EV << "    Private Key: " << privateKey << endl;
    symmetricKey = keyExchange.getSymmetricKey(privateKey);
    if(symmetricKey != "") {
        EV << "    Symmetric Key stored with success!" << endl;
        prevNode = node;
    } else {
        EV << "    The Private Key didn't correspond to the expected key exchange." << endl;
    }
}


CircuitRelay::CircuitRelay(TrafficMixer* mix, CreateCircuitCall* call, string privateKey) :
CircuitRelay(mix, call->getSrcNode(), call->getKeyExchange(), privateKey) {
    printLog("CircuitRelay(CreateCircuitCall)");
    // create and propagate evidence
    Certificate ownCert = mix->getOwnCertificate();
    AsymmetricEncryptionFlag signature = AsymmetricEncryptionFlag(ownCert.getKeySet());
    evidence = CircuitEvidence(*call, ownCert, signature);
    mix->putEvidence(evidence);

    // send response to prevNode
    BuildCircuitResponse* response = new BuildCircuitResponse();
    response->setCircuitID(getCircuitID());
    response->setSrcNode(mix->getThisNode());
    response->setBitLength(BUILDCIRCUITRESPONSE_L(response));
    wrapAndRelay(response);
}


CircuitRelay::CircuitRelay(TrafficMixer* mix, ExtendCircuitCall* call, string privateKey) :
CircuitRelay(mix, call->getSrcNode(), call->getKeyExchange(), privateKey) {
    printLog("CircuitRelay(ExtendCircuitCall)");
    // create and propagate evidence
    Certificate ownCert = mix->getOwnCertificate();
    AsymmetricEncryptionFlag signature = AsymmetricEncryptionFlag(ownCert.getKeySet());
    evidence = CircuitEvidence(*call, ownCert, signature);

    mix->putEvidence(evidence);

    // send response to prevNode
    OnionMessage* returnMsg = getOnionMessage();
    BuildCircuitResponse* response = new BuildCircuitResponse();
    response->setCircuitID(getCircuitID());
    response->setSrcNode(mix->getThisNode());
    response->setBitLength(BUILDCIRCUITRESPONSE_L(response));
    returnMsg->encapsulate(response);

    NotifyCircuitResponse* notifyMsg = new NotifyCircuitResponse();
    notifyMsg->setPrevCircuitID(call->getPrevCircuitID());
    notifyMsg->setNextCircuitID(getCircuitID());
    notifyMsg->setSrcNode(mix->getThisNode());
    notifyMsg->setMsg(*returnMsg);
    notifyMsg->setBitLength(NOTIFYCIRCUITRESPONSE_L(notifyMsg));

    delete returnMsg;
    mix->sendRpcResponse(call, notifyMsg);
}


/**
 * Makes the node leave the circuit by sending a RelayDisconnectCall to its neighbours
 */
void CircuitRelay::leave() {
    printLog("leave");
    propagateRelayDisconnectCall(mix->getThisNode());
}


void CircuitRelay::handleTriggerExtensionCall(TriggerExtensionCall* msg) {
    printLog("handleTriggerExtensionCall");

    ExtendCircuitCall* extensionCall = new ExtendCircuitCall();
    extensionCall->setKeyExchange(msg->getKeyExchange());
    extensionCall->setPrevNodeCert(mix->getOwnCertificate());
    extensionCall->setPrevCircuitID(getCircuitID());
    extensionCall->setBitLength(EXTENDCIRCUITCALL_L(extensionCall));

    mix->numBuildCircuitSent++;
    mix->bytesBuildCircuitSent += extensionCall->getByteLength();

    mix->sendUdpRpcCall(msg->getNextNode(), extensionCall);
}


void CircuitRelay::handleDestroyCircuitCall(DestroyCircuitCall* msg) {
    printLog("handleDestroyCircuitCall");
    propagateDestroyCircuitCall();
}


void CircuitRelay::handleNotifyCircuitResponse(NotifyCircuitResponse* msg) {
    printLog("handleNotifyCircuitResponse");

    OnionMessage* onionMsg = msg->getMsg().dup();
    nextNode = msg->getSrcNode();
    nextCircuitID = msg->getNextCircuitID();

    EV << "    Next node: " << nextNode.getIp().str()
       << ":" << nextNode.getPort() << endl
       << "    Next CircuitID: " << nextCircuitID << endl;

    wrapAndRelay(onionMsg);
}


void CircuitRelay::handleOnionMessage(OnionMessage* msg) {
    printLog("handleOnionMessage");
    EV << "    Circuit ID: " << msg->getCircuitID() << endl;
    if(isExitNode()) {
        EV << "    This is an exit relay: "
           << nextNode.getIp().str() << ":" << nextNode.getPort() << endl;
        sendExitRequest(msg);
    } else if (msg->getDirection() == OUTWARD) {
        cPacket* payload = msg->peel(symmetricKey);
        OnionMessage* onionMsg = dynamic_cast<OnionMessage*>(payload);
        if(onionMsg) {
            relayMessage(onionMsg);
        }
    } else {
        // this is a returning packet
        wrapAndRelay(msg);
    }
}


void CircuitRelay::handleExitResponse(UDPResponse* msg) {
    printLog("handleExitResponse");
    wrapAndRelay(msg);
}


void CircuitRelay::handleFailure(NodeHandle failedNode) {
    printLog("handleFailure");
    propagateRelayDisconnectCall(failedNode);
}


/**
 * Sends KeepAliveCall to next node in the circuit
 */
void CircuitRelay::propagateKeepAliveCall() {
    printLog("propagateKeepAliveCall");
    KeepAliveCall* keepAliveMsg = new KeepAliveCall();
    keepAliveMsg->setCircuitID(nextCircuitID);
    mix->sendUdpRpcCall(nextNode, keepAliveMsg);
}


void CircuitRelay::propagateDestroyCircuitCall() {
    printLog("propagateDestroyCircuitCall");
    DestroyCircuitCall* destroyMsg = new DestroyCircuitCall();
    destroyMsg->setCircuitID(nextCircuitID);
    mix->sendUdpRpcCall(nextNode, destroyMsg);
}


void CircuitRelay::propagateRelayDisconnectCall(NodeHandle disconnectedNode) {
    printLog("propageteRelayDisconnectCall");
    if(nextNode == disconnectedNode) {
        nextNode = NodeHandle::UNSPECIFIED_NODE;
    }
    RelayDisconnectCall* relayDisconnectMsg = new RelayDisconnectCall();
    relayDisconnectMsg->setCircuitID(getCircuitID());
    relayDisconnectMsg->setDisconnectedNode(disconnectedNode);
    mix->sendUdpRpcCall(prevNode, relayDisconnectMsg);
}


void CircuitRelay::relayMessage(OnionMessage* msg) {
    printLog("relayMessage");
    mix->numOnionRelayed++;
    mix->bytesOnionRelayed += msg->getByteLength();
    mix->onionsRelayed.record(mix->numOnionRelayed);
    if(msg->getDirection() == OUTWARD) {
        EV << "    Relaying outgoing onion to "
           << nextNode.getIp().str() << ":" << nextNode.getPort() << endl;
        mix->sendUdpRpcCall(nextNode, msg);
    } else {
        EV << "    Relaying returning onion to "
           << prevNode.getIp().str() << ":" << prevNode.getPort() << endl;
        mix->sendUdpRpcCall(prevNode, msg);
    }
}


void CircuitRelay::wrapAndRelay(cPacket* msg) {
    printLog("wrapAndRelay");
    OnionMessage* onionMsg = getOnionMessage();
    EV << "    CircuitID: " << onionMsg->getCircuitID() << endl
       << "    Session Key: " << onionMsg->getEncryptionFlag().getSymmetricKey() << endl;
    onionMsg->encapsulate(msg);
    relayMessage(onionMsg);
}


void CircuitRelay::sendExitRequest(OnionMessage* msg) {
    printLog("sendExitRequest");
    cPacket* payload = msg->peel(symmetricKey);
    if(!payload) {
        EV << "    Symmetric key [" << symmetricKey << "] is incorrect;"
           << " aborting..." << endl;
        return;
    }
    UDPCall* udpCall = dynamic_cast<UDPCall*>(payload);
    if(udpCall) {
        EV << "    Sending exit request to target server..." << endl;
        TransportAddress sender = mix->getThisNode();
        sender.setPort(TargetServer::port);
        udpCall->setSrcNode(sender);
        mix->sendExitRequest(getCircuitID(), udpCall);
        mix->numExitRequests++;
        mix->bytesExitRequests += udpCall->getByteLength();
        return;
    }

    TriggerExtensionCall* extensionCall =
            dynamic_cast<TriggerExtensionCall*>(payload);
    if(extensionCall) {
        handleTriggerExtensionCall(extensionCall);
        return;
    }
    EV << "    Unrecognised payload " << payload
       << "; aborting..." << endl;
}


OnionMessage* CircuitRelay::getOnionMessage() {
    printLog("getOnionMessage");
    OnionMessage* msg = new OnionMessage(INWARD, getCircuitID(), symmetricKey);
    msg->setSrcNode(mix->getThisNode());
    msg->setBitLength(ONIONMESSAGE_L(msg));
    return msg;
}


void CircuitRelay::printLog(string function) {
    string message = "\n    " + prevNode.getIp().str()
                   + "-->" + nextNode.getIp().str()
                   + " @ " + getCircuitID().toString();
    mix->printLog("CircuitRelay::" + function, message);
}

