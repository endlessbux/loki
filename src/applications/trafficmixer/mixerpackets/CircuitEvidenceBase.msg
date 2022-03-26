cplusplus {{
#include "common/BinaryValue.h"
#include "common/OverlayKey.h"
#include "Certificate.h"
}}


class BinaryValue extends cObject;
class noncobject OverlayKey;
class Certificate;


enum EvidenceType {
    CIRCUIT_CREATION = 0;
    CIRCUIT_EXTENSION = 1;
}


packet CircuitEvidence {
    @customize(true);
    int type @enum(EvidenceType);
    BinaryValue requestPayload;
    bool isEncrypted = true;
    Certificate producerCert;
    BinaryValue producerSignature;
    simtime_t timestamp;
    OverlayKey circuitID;
}
