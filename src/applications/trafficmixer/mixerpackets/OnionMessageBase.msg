cplusplus {{
#include "common/OverlayKey.h"
#include "common/BinaryValue.h"
#include "SymmetricEncryptionFlag.h"
}}


class noncobject OverlayKey;
class noncobject SymmetricEncryptionFlag;
class BinaryValue extends cObject;


enum TrafficDirection {
    OUTWARD = 0;
    INWARD = 1;
};


packet OnionMessage {
    @customize(true);
    int direction @enum(TrafficDirection);
    OverlayKey circuitID;
    SymmetricEncryptionFlag encryptionFlag;
};
