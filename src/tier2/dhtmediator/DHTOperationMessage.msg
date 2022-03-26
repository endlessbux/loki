cplusplus {{
#include "common/CommonMessages_m.h"
#include "common/NodeHandle.h"
}}


class BaseCallMessage;
class noncobject DhtDumpEntry;
class noncobject NodeHandle;


packet PutCall extends BaseCallMessage {
	int confirmedNonce;
}


packet GetCall extends BaseCallMessage {
    DhtDumpEntry dhtEntry;
}
