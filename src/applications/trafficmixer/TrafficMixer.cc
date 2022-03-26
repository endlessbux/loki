#include "TrafficMixer.h"
#include "common/GlobalNodeList.h"

using namespace std;

Define_Module(TrafficMixer);

TrafficMixer::TrafficMixer() {

}


void TrafficMixer::initializeApp(int stage) {
    if (stage != MIN_STAGE_APP) {
        return;
    }
}


void TrafficMixer::finishApp() {

}


void TrafficMixer::handleTimerEvent(cMessage* msg) {

}


void TrafficMixer::changeState(int state) {
    switch(state) {
    case INIT:
        // share certificate information with the rest of the network through the DHT
        //certStorageNonce = dht.storeCertificate(getOwnCertificate(), thisNode);
        // make sure there are enough peers to populate the relay pool


        // populate relay pool

        break;
    case AWAIT:

        break;
    case BOOTSTRAP:
        // build circuits

        break;
    case READY:
        // maintain circuits

        break;
    case SHUTDOWN:
        // close up existing circuits

        break;
    }
}


bool TrafficMixer::handleRpcCall(BaseCallMessage* msg) {
    RPC_SWITCH_START( msg )
    RPC_DELEGATE( Join, rpcJoin )
    RPC_DELEGATE( Put, rpcPut )
    RPC_DELEGATE( Get, rpcGet )
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}

// called when a new node joined the overlay
void TrafficMixer::rpcJoin(JoinCall* joinCall) {
    NodeHandle requestorHandle = joinCall->getSrcNode();
    Certificate requestorCert = joinCall->getCert();
    certStorageNonce = dht.storeCertificate(requestorCert, requestorHandle);
}


// called when something was successfully stored on the DHT
void TrafficMixer::rpcPut(PutCall* putCall) {

}


// called when something is retrieved from the DHT
void TrafficMixer::rpcGet(GetCall* getCall) {

}
