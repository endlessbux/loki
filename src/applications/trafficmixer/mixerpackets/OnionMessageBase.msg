cplusplus {{
#include "common/OverlayKey.h"
#include "common/BinaryValue.h"
}}


class noncobject OverlayKey;
class noncobject BinaryValue;


enum TrafficDirection {
    OUTWARD = 0;
    INWARD = 1;
};


packet OnionMessage {
    @customize(true);
    int direction @enum(TrafficDirection);
    OverlayKey circuitID;
    BinaryValue payload;
    BinaryValue encryptionKey;
};
