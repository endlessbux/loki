#ifndef __IDENTITYKEY_H_
#define __IDENTITYKEY_H_

#include "common/NodeHandle.h"
#include "OnionKey.h"


class IdentityKey {
    public:
        NodeHandle* owner;

        IdentityKey(NodeHandle* keyHolder);
        virtual ~IdentityKey();

        OnionKey* generateNewOnionKey();

        bool isOnionKeyAuthentic(OnionKey* key);

    private:
        void* privateKey;

};

#endif
