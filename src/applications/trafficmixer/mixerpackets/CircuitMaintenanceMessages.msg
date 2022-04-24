cplusplus {{
#include "common/CommonMessages_m.h" 
#include "common/OverlayKey.h" 
#include "common/NodeHandle.h"
}}


class BaseCallMessage;
class BaseResponseMessage;
class noncobject OverlayKey;
class noncobject NodeHandle;


packet DestroyCircuitCall extends BaseCallMessage {
    OverlayKey circuitID;
}

packet RelayDisconnectCall extends BaseCallMessage {
    OverlayKey circuitID;
    NodeHandle disconnectedNode;
}

packet KeepAliveCall extends BaseCallMessage {
	OverlayKey circuitID;
}

packet KeepAliveResponse extends BaseResponseMessage {
    OverlayKey circuitID;
}
