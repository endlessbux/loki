cplusplus {{
#include "common/CommonMessages_m.h"  
#include "common/OverlayKey.h"
#include "common/NodeHandle.h"
#include "Certificate.h"
#include "KeyExchange.h"
}}

class BaseCallMessage;
class BaseResponseMessage;
class Certificate;
class KeyExchange;
class noncobject OverlayKey;
class noncobject NodeHandle;


packet CreateCircuitRequest extends BaseCallMessage {
    Certificate creatorCert;
    KeyExchange keyExchange;
}


packet BuildCircuitResponse extends BaseResponseMessage {
    OverlayKey circuitID;
}


packet ExtendCircuitRpc extends BaseCallMessage {
    NodeHandle nextNode;
    KeyExchange keyExchange;
}


packet ExtendCircuitRequest extends BaseCallMessage {
    KeyExchange keyExchange;
    Certificate prevNodeCert;
    OverlayKey prevCircuitID;
}
