cplusplus {{
#include "common/CommonMessages_m.h"
#include "common/NodeHandle.h"
#include "common/OverlayKey.h"
#include "applications/trafficmixer/mixerpackets/Certificate.h"
#include "applications/trafficmixer/mixerpackets/CircuitEvidence.h"
}}


class BaseCallMessage;
class BaseResponseMessage;
class Certificate;
class CircuitEvidence;
class noncobject NodeHandle;
class noncobject OverlayKey;


packet MediatorPutResponse extends BaseResponseMessage {
    bool isSuccess = false;
}


packet GetCertificateCall extends BaseCallMessage {
    NodeHandle node;	// node for which the certificate is being queried
}


packet GetCertificateResponse extends BaseResponseMessage {
    Certificate cert;	// the queried node's certificate if it exists, an
    					// undefined certificate otherwise
}


packet PutCertificateCall extends BaseCallMessage {
    NodeHandle handle;	// the node corresponding the the certificate
    Certificate cert;	// the certificate to be stored on the DHT
}


packet PutCertificateResponse extends MediatorPutResponse { }


packet GetEvidenceCall extends BaseCallMessage {
    OverlayKey circuitID;	// the circuit ID corresponding to the queried
    						// evidence
}


packet GetEvidenceResponse extends BaseResponseMessage {
    CircuitEvidence evidence;	// the retrieved evidence, or an undefined
    							// evidence packet otherwise
}


packet PutEvidenceCall extends BaseCallMessage {
    CircuitEvidence evidence;	// the evidence to be stored on the DHT
}


packet PutEvidenceResponse extends MediatorPutResponse { }
