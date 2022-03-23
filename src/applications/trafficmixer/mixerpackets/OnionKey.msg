cplusplus {{
#include "common/BinaryValue.h"  
}}


class noncobject BinaryValue;


packet OnionKey {
    BinaryValue publicKey;
    BinaryValue privateKey;
    simtime_t expiration;
}