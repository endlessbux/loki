cplusplus {{
#include "common/TransportAddress.h"
#include "common/CommonMessages_m.h"

// constants in bits 
static const int HTTPGET_L = 800 * 8;
static const int HTTPRESPONSE_L = 1500 * 8;

#define UDPCALL_L(msg) (BASECALL_L(msg) + HTTPGET_L)
#define UDPRESPONSE_L(msg) (BASERESPONSE_L(msg) + HTTPRESPONSE_L)
}}

class noncobject TransportAddress;
class BaseCallMessage;
class BaseResponseMessage;


packet UDPCall extends BaseCallMessage
{
    TransportAddress senderAddress;  // address of the node that sent the message
    TransportAddress targetAddress;
    int internalID;
}

packet UDPResponse extends BaseResponseMessage
{
    int internalID;
}
