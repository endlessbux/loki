cplusplus {{
#include "common/OverlayKey.h"
#include "common/TransportAddress.h"
#include "common/CommonMessages_m.h"
#include "CommonEncryptionPackets_m.h"

// constants in bits
static const int IPADDRESS_L = 8 * 4;
static const int TRAFFICRECORD_L = TIMESTAMP_L + 
								   IPADDRESS_L + KEY_L;

#define TRAFFICREPORT_L(msg) (BASECALL_L(msg) +\
							  msg->getRecordsArraySize() * TRAFFICRECORD_L)
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
