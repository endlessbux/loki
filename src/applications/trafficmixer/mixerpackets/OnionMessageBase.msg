cplusplus {{
#include "common/OverlayKey.h"
#include "common/BinaryValue.h"
#include "SymmetricEncryptionFlag.h"
#include "common/CommonMessages_m.h"
}}


class noncobject OverlayKey;
class noncobject SymmetricEncryptionFlag;
class BaseCallMessage;
class BaseResponseMessage;


enum TrafficDirection {
    OUTWARD = 0;
    INWARD = 1;
};


packet OnionMessage extends BaseCallMessage {
    @customize(true);
    int direction @enum(TrafficDirection);
    OverlayKey circuitID;
    SymmetricEncryptionFlag encryptionFlag;
};

packet OnionCheck extends BaseResponseMessage {

}

