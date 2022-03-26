#ifndef __TRAFFICMIXER_H_
#define __TRAFFICMIXER_H_

#include <omnetpp.h>
#include <vector>
#include <memory>
#include "tier2/dhtmediator/DHTMediator.h"
#include "tier2/dhtmediator/DHTOperationMessage_m.h"
#include "common/BaseApp.h"
#include "common/NodeHandle.h"
#include "common/OverlayAccess.h"
#include "overlay/permissionedchord/PermissionedChord.h"
#include "overlay/permissionedchord/ChordMessage_m.h"
#include "mixerpackets/Certificate.h"


class DHTDataStorage;
using loki::PermissionedChord;
using loki::JoinCall;


class TrafficMixer : public BaseApp {
    public:
        enum States {
            INIT        = 0,
            AWAIT       = 1,
            BOOTSTRAP   = 2,
            READY       = 3,
            SHUTDOWN    = 4
        };

        // statistics
        int numSent;
        int numRelayed;


        TrafficMixer();
        virtual ~TrafficMixer() {};
        void initializeApp(int stage);
        void finishApp();
        void handleTimerEvent(cMessage* msg);

        Certificate getOwnCertificate() {
            return ((PermissionedChord*)OverlayAccess().get(this))->cert;
        }

    protected:

        virtual void changeState(int state);

        //void internalHandleRpcMessage(BaseRpcMessage* msg) override;

        virtual bool handleRpcCall(BaseCallMessage* msg) override;

    private:
        DHTMediator dht;
        int certStorageNonce = 0;

        void rpcJoin(JoinCall* joinCall);

        void rpcPut(PutCall* putCall);

        void rpcGet(GetCall* getCall);
};


#endif
