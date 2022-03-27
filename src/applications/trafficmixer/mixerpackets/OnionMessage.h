#ifndef __ONIONMESSAGE_H_
#define __ONIONMESSAGE_H_

#include "OnionMessageBase_m.h"
#include "common/OverlayKey.h"

using std::string;


class OnionMessage : public OnionMessage_Base {

    /**
     * Class to demonstrate the functioning of Onion Routing.
     */
    public:
        OnionMessage(const char *name=nullptr) : OnionMessage_Base(name) {}
        OnionMessage(const OnionMessage& other) : OnionMessage_Base(other) {}
        OnionMessage& operator=(const OnionMessage& other)
            {OnionMessage_Base::operator=(other); return *this;}
        virtual OnionMessage *dup() const {return new OnionMessage(*this);}


        OnionMessage(TrafficDirection direction, OverlayKey circuitID,
                     SymmetricEncryptionFlag encryptionFlag) {
            this->direction = direction;
            this->circuitID = circuitID;
            this->encryptionFlag = encryptionFlag;
        }

        cPacket* peel(string decryptionKey) {
            encryptionFlag.decrypt(decryptionKey);
            if(encryptionFlag.getState() == PLAINTEXT) {
                return this->decapsulate();
            }
            return nullptr;
        }

};

#endif
