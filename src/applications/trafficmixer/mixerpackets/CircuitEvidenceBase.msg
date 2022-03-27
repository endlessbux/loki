cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"
#include "Certificate.h"

#include "AsymmetricEncryptionFlag.h"
}}


class noncobject OverlayKey;
class noncobject AsymmetricEncryptionFlag;
class Certificate;


enum EvidenceType {
    CIRCUIT_CREATION = 0;
    CIRCUIT_EXTENSION = 1;
}


packet CircuitEvidence {
    @customize(true);
    int type @enum(EvidenceType);
    bool isEncrypted = true;
    Certificate producerCert;
    AsymmetricEncryptionFlag producerSignature;
    simtime_t timestamp;
    OverlayKey circuitID;
}
