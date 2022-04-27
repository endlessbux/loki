cplusplus {{
#include "common/OverlayKey.h"
#include "common/TransportAddress.h"
#include "Certificate.h"
#include "CircuitEvidence.h"
}}

class noncobject TransportAddress;
class noncobject OverlayKey;
class Certificate;
class CircuitEvidence;


message RegisteredUser {
    TransportAddress address;
    Certificate cert;
}


message CircuitData {
    RegisteredUser user;
    OverlayKey circuitTrace[];
    CircuitEvidence evidenceTrace[];
} 
