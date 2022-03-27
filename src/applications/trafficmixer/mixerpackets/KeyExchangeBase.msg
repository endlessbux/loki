cplusplus {{
#include "AsymmetricEncryptionFlag.h"
}}

class noncobject AsymmetricEncryptionFlag;


packet KeyExchange {
    @customize(true);
    string symmetricKey;
    AsymmetricEncryptionFlag encryptionFlag;
};