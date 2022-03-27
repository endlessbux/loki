cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"

using std::string;
    
    
inline string generateRandomKey(int offset = 0) {
    BinaryValue seed = BinaryValue(simTime().inUnit(SIMTIME_MS) + offset);
    return OverlayKey::sha1(seed).toString();
}
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
    