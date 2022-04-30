cplusplus {{
#include "CommonEncryptionPackets_m.h"
#include "AsymmetricKeySet.h"


static const int CERTIFICATE_L = ASYMMETRICKEYSET_L + TIMESTAMP_L;
}}

class noncobject AsymmetricKeySet;


packet Certificate {
    @customize(true);
    AsymmetricKeySet keySet;
    simtime_t expiration;
    bool isSigned = false;
}