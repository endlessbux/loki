#ifndef __ONIONMESSAGE_H_
#define __ONIONMESSAGE_H_

#include "OnionMessageBase_m.h"
#include "common/OverlayKey.h"


class OnionMessage : public OnionMessage_Base {

    /**
     * Class to showcase the functioning of Onion Routing.
     */
    public:
        OnionMessage(const char *name=nullptr) : OnionMessage_Base(name) {}
        OnionMessage(const OnionMessage& other) : OnionMessage_Base(other) {}
        OnionMessage& operator=(const OnionMessage& other)
            {OnionMessage_Base::operator=(other); return *this;}
        virtual OnionMessage *dup() const {return new OnionMessage(*this);}


        OnionMessage(const TrafficDirection direction, const OverlayKey circuitID,
                     const BinaryValue payload, const BinaryValue encryptionKey) {
            this->direction = direction;
            this->circuitID = circuitID;
            this->payload = payload;
            this->encryptionKey = encryptionKey;
        }

        BinaryValue peel(BinaryValue decryptionKey) {
            if(encryptionKey == decryptionKey) {
                return payload;
            }
            return BinaryValue();
        }

};

#endif
