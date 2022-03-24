cplusplus {{
#include "common/CommonMessages_m.h"
}}


class BaseCallMessage;
class noncobject DhtDumpEntry;


packet StorageSuccessMessage extends BaseCallMessage {
    int confirmedNonce;
}


packet StorageGetResultMessage extends BaseCallMessage {
    DhtDumpEntry dhtEntry;
}
