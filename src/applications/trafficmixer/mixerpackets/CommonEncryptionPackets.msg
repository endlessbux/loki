cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"

}}


class AsymmetricKeySet {
    @customize(true);
    string publicKey;
    string privateKey;
};


enum EncryptionState {
    PLAINTEXT = 0;
    ENCRYPTED = 1;
    SIGNED = 2;
};


class EncryptionFlag {
    int state @enum(EncryptionState);
};
    