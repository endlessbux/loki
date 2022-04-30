cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"

// constants for message length in bits
static const int ASYMMETRICKEYSET_L = 2048;
static const int TIMESTAMP_L = 13 * 8;

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
    