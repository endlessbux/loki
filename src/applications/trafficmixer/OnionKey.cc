#include "OnionKey.h"

OnionKey::OnionKey(NodeHandle* keyHolder, void* identity) {
    identitySignature = identity;
    owner = keyHolder;
    privateKey = this;
}


OnionKey::~OnionKey() {
}


bool OnionKey::compareIdentity(void* identity) {
    return identitySignature == identity;
}
