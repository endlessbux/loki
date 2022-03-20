#ifndef __ONIONKEY_H_
#define __ONIONKEY_H_

#include "common/NodeHandle.h"


class OnionKey {
    public:
        NodeHandle* owner;

        OnionKey(NodeHandle* keyHolder, void* identity);
        virtual ~OnionKey();

        bool compareIdentity(void* identity);

        // Method to check if this OnionKey is still valid
        // TODO: bool isExpired();

        // TODO: encrypt()
        // TODO: decrypt()

    private:
        void* identitySignature;
        void* privateKey;
        // TODO: expiration
};

#endif
