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

    poolSize = par("poolSize");
    numCircuits = par("numCircuits");

    changeState(INIT);
}


void TrafficMixer::finishApp() {

}


void TrafficMixer::handleTimerEvent(cMessage* msg) {

}


void TrafficMixer::changeState(int state) {
    switch(state) {
    case INIT:
        // share keys with the rest of the network through the DHT
        //dht.dataStorage->addData(key, kind, id, value, ttlMessage, is_modifiable, sourceNode, responsible)

        // make sure there are enough peers to populate the relay pool
        if(globalNodeList->getNumNodes() <= poolSize) {
            changeState(AWAIT);
            return;
        }

        // populate relay pool
        populatePool();
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


void TrafficMixer::keepCircuitsAlive() {

}


void TrafficMixer::populatePool() {
    while(relayPool.size() < poolSize) {
        // get random node
        TransportAddress* randomAddress = globalNodeList->getRandomAliveNode();
    }
}
