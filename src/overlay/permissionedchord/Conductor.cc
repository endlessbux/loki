#include "Conductor.h"

#include <stdio.h>
#include <iostream>
#include "common/GlobalStatistics.h"
#include "common/GlobalNodeListAccess.h"
#include "common/BaseRpc.h"
#include "applications/trafficmixer/mixerpackets/Certificate.h"
#include "applications/trafficmixer/mixerpackets/AsymmetricKeySet.h"

using namespace std;

Define_Module(Conductor);


NodeHandle Conductor::networkController = NodeHandle::UNSPECIFIED_NODE;


void Conductor::initializeOverlay(int stage) {
    /**
     * Called upon object creation. Assigns defined values to corresponding variables.
     * Generates a simulated RSA key pair for the Controller.
     */
    if(stage != MIN_STAGE_OVERLAY) return;

    Conductor::networkController = thisNode;

    changeState(READY);
}


void Conductor::finishOverlay() {

}


void Conductor::handleUDPMessage(BaseOverlayMessage* msg) {
    EV << "Received UDP Message: " << msg;

    delete msg;
}


bool Conductor::handleRpcCall(BaseCallMessage* msg) {
    if (state != READY) {
        EV << "[Conductor::handleRpcCall() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Received RPC call and state != READY"
           << endl;
        return false;
    }

    if(loki::RegistrationCall* registrationMsg = dynamic_cast<loki::RegistrationCall*>(msg)) {
        // TODO: introduce registration delay with a timer
        handleRegistration(registrationMsg);
        return true;
    }

    return false;
}


void Conductor::handleTimerEvent(cMessage* msg) {

}


void Conductor::changeState(int toState) {
    switch(toState) {
        case READY:
            state = READY;

            // debug message
            if (debugOutput) {
                EV << "[Conductor::changeState() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "    Entered READY stage"
                << endl;
            }
            getParentModule()->getParentModule()->bubble("Enter READY state.");

            break;
    }
}


void Conductor::handleRegistration(loki::RegistrationCall* registrationMsg) {
    Certificate receivedCert = registrationMsg->getCert();
    NodeHandle sourceNode = registrationMsg->getSrcNode();
    AsymmetricKeySet receivedKey = receivedCert.getKeySet();

    // OnionKey expires after a year
    receivedCert.setExpiration(simTime() + 3600 * 24 * 365);
    receivedCert.sign();

    // Store node information locally
    loki::RegistrationCall* msgToTier3 = registrationMsg->dup();
    msgToTier3->setCert(receivedCert);
    msgToTier3->setNode(registrationMsg->getSrcNode());
    sendInternalRpcCall(TIER3_COMP, msgToTier3);

    // Send response
    loki::RegistrationResponse* responseMsg = new loki::RegistrationResponse("RegistrationResponse");
    responseMsg->setCert(receivedCert);
    sendRpcResponse(registrationMsg, responseMsg);
}


// TODO:
//bool Conductor::registerUser() {
//
//    return false;
//}

