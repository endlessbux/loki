cplusplus {{
#include "CommonEncryptionPackets_m.h"
#include "AsymmetricKeySet.h"
}}

class noncobject AsymmetricKeySet;
class noncobject EncryptionFlag;


class AsymmetricEncryptionFlag extends EncryptionFlag {
    @customize(true);
    AsymmetricKeySet keySet;
};


class SymmetricEncryptionFlag extends EncryptionFlag {
    @customize(true);
    string symmetricKey;
};
