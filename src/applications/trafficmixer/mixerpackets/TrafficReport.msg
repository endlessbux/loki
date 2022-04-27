cplusplus {{
#include "common/OverlayKey.h"
#include "common/TransportAddress.h"
#include "common/CommonMessages_m.h"
}}

class noncobject OverlayKey;
class noncobject TransportAddress;
class BaseCallMessage;
class BaseResponseMessage;


packet TrafficRecord {
    OverlayKey circuitID;
    TransportAddress target;
    simtime_t timestamp;
}

packet TrafficReport extends BaseCallMessage {
    TrafficRecord records[];
}

packet ReportResponse extends BaseResponseMessage {
    
}
