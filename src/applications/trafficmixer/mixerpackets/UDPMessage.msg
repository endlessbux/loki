cplusplus {{
#include "common/TransportAddress.h"
#include "common/CommonMessages_m.h"
}}

class noncobject TransportAddress;
class BaseCallMessage;
class BaseResponseMessage;


packet UDPCall extends BaseCallMessage
{
    TransportAddress senderAddress;  // address of the node that sent the message
    TransportAddress targetAddress;
}

packet UDPResponse extends BaseResponseMessage
{
    
}
