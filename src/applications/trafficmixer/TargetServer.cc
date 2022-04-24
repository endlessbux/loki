#include "TargetServer.h"
#include "mixerpackets/TCPMessage_m.h"


Define_Module(TargetServer);


NodeHandle TargetServer::address = NodeHandle::UNSPECIFIED_NODE;
int TargetServer::tcpPort = 24000;

TargetServer::TargetServer() {

}


TargetServer::~TargetServer() {

}


void TargetServer::initializeApp(int stage) {
    if(stage != MIN_STAGE_APP) {
        return;
    }

    thisNode.setPort(TargetServer::tcpPort);
    TargetServer::address = thisNode;

    bindAndListenTcp(TargetServer::tcpPort);
}


void TargetServer::finishApp() {

}


void TargetServer::handleDataReceived(TransportAddress address, cPacket* msg, bool urgent) {
    printLog("handleDataReceived");
    TCPCall* callMsg = dynamic_cast<TCPCall*>(msg);
    if(!callMsg) {
        EV << "    Received an unknown message; aborting..." << endl;
        return;
    }
    EV << "    Received a message from: " << address.getIp().str()
       << ";\n    sending a response..." << endl;
    TCPResponse* responseMsg = new TCPResponse();
    responseMsg->setNonce(callMsg->getNonce());
    sendTcpData(responseMsg, address);
}
