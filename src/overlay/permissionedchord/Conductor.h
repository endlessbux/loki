#ifndef __CONDUCTOR_H_
#define __CONDUCTOR_H_

#include "common/certificate.h"
#include "common/BaseOverlay.h"
#include "inet/networklayer/common/L3Address.h"
#include "common/CommonMessages_m.h"
#include "overlay/permissionedchord/ChordMessage_m.h"

using loki::RegistrationCall;

class Conductor : public BaseOverlay {
    /**
     * Handles registration to the overlay by storing peer metadata and
     * releasing certifications to peers' digital signatures
     */

    public:
        char* publicKey;
        static NodeHandle networkController;

        virtual ~Conductor() {};
        virtual void initializeOverlay(int stage);
        virtual void finishOverlay();
        virtual void handleUDPMessage(BaseOverlayMessage* msg);
        virtual bool handleRpcCall(BaseCallMessage* msg);
        virtual void handleTimerEvent(cMessage* msg);

    protected:
        /**
         * changes node state
         *
         * @param toState state to change to
         */
        virtual void changeState(int toState);


        //virtual void handleJoin();
        virtual void handleRegistration(RegistrationCall* registrationMsg);
};


#endif
