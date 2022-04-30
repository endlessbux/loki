cplusplus {{
#include "common/CommonMessages_m.h"  
#include "common/OverlayKey.h"
#include "common/NodeHandle.h"
#include "Certificate.h"
#include "KeyExchange.h"
#include "OnionMessage.h"


#define CREATECIRCUITCALL_L(msg) (BASECALL_L(msg) + CERTIFICATE_L +\
								  KEYEXCHANGE_L)
#define BUILDCIRCUITRESPONSE_L(msg) (BASERESPONSE_L(msg) + KEY_L)
#define NOTIFYCIRCUITRESPONSE_L(msg) (BASECALL_L(msg) +\
									  KEY_L * 2 +\
									  ONIONMESSAGE_L(msg))
#define TRIGGEREXTENSIONCALL_L(msg) (BASECALL_L(msg) + IPADDRESS_L + KEYEXCHANGE_L)
#define EXTENDCIRCUITCALL_L(msg) (BASECALL_L(msg) + KEYEXCHANGE_L +\
								  CERTIFICATE_L + KEY_L)

}}

class BaseCallMessage;
class BaseResponseMessage;
class Certificate;
class KeyExchange;
class OnionMessage;
class noncobject OverlayKey;
class noncobject NodeHandle;


packet CreateCircuitCall extends BaseCallMessage {
    @customize(true);
    Certificate creatorCert;
    KeyExchange keyExchange;
}


packet BuildCircuitResponse extends BaseResponseMessage {
    OverlayKey circuitID;
}

packet NotifyCircuitResponse extends BaseResponseMessage {
    OverlayKey prevCircuitID;
    OverlayKey nextCircuitID;
    OnionMessage msg;
}


packet TriggerExtensionCall extends BaseCallMessage {
    NodeHandle nextNode;
    KeyExchange keyExchange;
}


packet ExtendCircuitCall extends BaseCallMessage {
    @customize(true);
    KeyExchange keyExchange;
    Certificate prevNodeCert;
    OverlayKey prevCircuitID;
}
