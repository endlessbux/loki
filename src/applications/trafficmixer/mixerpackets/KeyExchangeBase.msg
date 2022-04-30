cplusplus {{
#include "AsymmetricEncryptionFlag.h"

// key length in bits
static const int SIMMETRICKEY_L = 128;
static const int KEYEXCHANGE_L = SIMMETRICKEY_L;
}}

class noncobject AsymmetricEncryptionFlag;


packet KeyExchange {
    @customize(true);
    string symmetricKey;
    AsymmetricEncryptionFlag encryptionFlag;
};