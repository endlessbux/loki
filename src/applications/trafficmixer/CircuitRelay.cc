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
    // create and propagate evidence
    Certificate ownCert = mix->getOwnCertificate();
    AsymmetricEncryptionFlag signature = AsymmetricEncryptionFlag(ownCert.getKeySet());
    evidence = CircuitEvidence(*call, ownCert, signature);
    mix->putEvidence(evidence);

    // send response to prevNode
    BuildCircuitResponse* response = new BuildCircuitResponse();
    response->setCircuitID(getCircuitID());
    response->setSrcNode(mix->getThisNode());
    wrapAndRelay(response);
}


CircuitRelay::CircuitRelay(TrafficMixer* mix, ExtendCircuitCall* call, string privateKey) :
CircuitRelay(mix, call->getSrcNode(), call->getKeyExchange(), privateKey) {
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
    returnMsg->encapsulate(response);

    NotifyCircuitResponse* notifyMsg = new NotifyCircuitResponse();
    notifyMsg->setPrevCircuitID(call->getPrevCircuitID());
    notifyMsg->setNextCircuitID(getCircuitID());
    notifyMsg->setSrcNode(mix->getThisNode());
    notifyMsg->setMsg(*returnMsg);

    delete returnMsg;
    mix->sendRpcResponse(call, notifyMsg);
}


/**
 * Makes the node leave the circuit by sending a RelayDisconnectCall to its neighbours
 */
void CircuitRelay::leave() {
    propagateRelayDisconnectCall(mix->getThisNode());
}


void CircuitRelay::handleTriggerExtensionCall(TriggerExtensionCall* msg) {
    printLog("handleTriggerExtensionCall");

    ExtendCircuitCall* extensionCall = new ExtendCircuitCall();
    extensionCall->setKeyExchange(msg->getKeyExchange());
    extensionCall->setPrevNodeCert(mix->getOwnCertificate());
    extensionCall->setPrevCircuitID(getCircuitID());

    mix->sendUdpRpcCall(msg->getNextNode(), extensionCall);
}


void CircuitRelay::handleDestroyCircuitCall(DestroyCircuitCall* msg) {
    propagateDestroyCircuitCall();
}


void CircuitRelay::handleNotifyCircuitResponse(NotifyCircuitResponse* msg) {
    printLog("handleNotifyCircuitResponse");
    EV << "    Next node: " << msg->getSrcNode().getIp().str()
       << ":" << msg->getSrcNode().getPort() << endl
       << "    Next CircuitID: " << msg->getNextCircuitID() << endl;


    OnionMessage* onionMsg = msg->getMsg().dup();
    nextNode = msg->getSrcNode();
    nextCircuitID = msg->getNextCircuitID();
    wrapAndRelay(onionMsg);
}


void CircuitRelay::handleOnionMessage(OnionMessage* msg) {
    printLog("handleOnionMessage");
    if(isExitNode()) {
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


void CircuitRelay::handleExitResponse(TCPResponse* msg) {
    mix->closeTcpConnection((TransportAddress)msg->getSrcNode());
    wrapAndRelay(msg);
}


void CircuitRelay::handleFailure(NodeHandle failedNode) {
    propagateRelayDisconnectCall(failedNode);
}


/**
 * Sends KeepAliveCall to next node in the circuit
 */
void CircuitRelay::propagateKeepAliveCall() {
    KeepAliveCall* keepAliveMsg = new KeepAliveCall();
    keepAliveMsg->setCircuitID(nextCircuitID);
    mix->sendUdpRpcCall(nextNode, keepAliveMsg);
}


void CircuitRelay::propagateDestroyCircuitCall() {
    DestroyCircuitCall* destroyMsg = new DestroyCircuitCall();
    destroyMsg->setCircuitID(nextCircuitID);
    mix->sendUdpRpcCall(nextNode, destroyMsg);
}


void CircuitRelay::propagateRelayDisconnectCall(NodeHandle disconnectedNode) {
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
    TCPCall* tcpCall = dynamic_cast<TCPCall*>(payload);
    if(tcpCall) {
        EV << "   Sending exit request to target server..." << endl;
        TransportAddress sender = mix->getThisNode();
        TransportAddress target = tcpCall->getTargetAddress();
        tcpCall->setSenderAddress(sender);

        mix->establishTcpConnection(target);
        mix->sendTcpData(tcpCall, target);
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
    OnionMessage* msg = new OnionMessage(INWARD, getCircuitID(), symmetricKey);
    msg->setSrcNode(mix->getThisNode());
    return msg;
}


void CircuitRelay::printLog(string function) {
    mix->printLog("CircuitRelay::" + function);
}

