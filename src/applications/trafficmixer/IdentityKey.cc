#include "IdentityKey.h"


IdentityKey::IdentityKey(NodeHandle* keyHolder) {
    owner = keyHolder;
    privateKey = this;
}


IdentityKey::~IdentityKey() {

}


OnionKey* IdentityKey::generateNewOnionKey() {
    return new OnionKey(owner, privateKey);
}


bool IdentityKey::isOnionKeyAuthentic(OnionKey* key) {
    return key->compareIdentity(privateKey);
}
