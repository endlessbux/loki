cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"
#include "Certificate.h"

#include "AsymmetricEncryptionFlag.h"


#define CIRCUITEVIDENCE_L(msg) (CERTIFICATE_L + TIMESTAMP_L +\
								KEY_L +\
								sizeof(msg->getJsonCall()) * 8) 
}}


class noncobject OverlayKey;
class noncobject AsymmetricEncryptionFlag;
class Certificate;


enum EvidenceType {
    CIRCUIT_CREATION = 1;
    CIRCUIT_EXTENSION = 2;
}


packet CircuitEvidence {
    @customize(true);
    int type @enum(EvidenceType);
    bool isEncrypted = true;
    Certificate producerCert;
    AsymmetricEncryptionFlag producerSignature;
    simtime_t timestamp;
    OverlayKey circuitID;
    string jsonCall;
}
