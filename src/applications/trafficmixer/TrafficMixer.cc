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
