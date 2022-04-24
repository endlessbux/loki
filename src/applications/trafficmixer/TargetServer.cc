#include "TargetServer.h"
#include "mixerpackets/UDPMessage_m.h"


Define_Module(TargetServer);


TransportAddress TargetServer::address = TransportAddress::UNSPECIFIED_NODE;
int TargetServer::port = 24000;

TargetServer::TargetServer() {

}


TargetServer::~TargetServer() {

}


void TargetServer::initializeApp(int stage) {
    if(stage != MIN_STAGE_APP) {
        return;
    }

    thisNode.setPort(TargetServer::port);
    TargetServer::address = thisNode;

    bindToPort(TargetServer::port);
}


void TargetServer::finishApp() {

}


void TargetServer::handleUDPMessage(cMessage* msg) {
    printLog("handleUDPMessage");
}


bool TargetServer::internalHandleRpcCall(BaseCallMessage* msg) {
    printLog("internalHandleRpcCall");
    UDPCall* callMsg = dynamic_cast<UDPCall*>(msg);
    if(!callMsg) {
        EV << "    Received an unknown message; aborting..." << endl;
        return false;
    }
    NodeHandle srcNode = callMsg->getSrcNode();
    EV << "    Received a message from: " << srcNode.getIp().str()
       << ";\n    sending a response..." << endl;
    UDPResponse* responseMsg = new UDPResponse();
    responseMsg->setNonce(callMsg->getNonce());
    sendRpcResponse(callMsg, responseMsg);

    return true;
}
