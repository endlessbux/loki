cplusplus {{
#include "common/BinaryValue.h"

typedef std::vector<char> CharVector;
}}

class noncobject CharVector;
// class BinaryValue extends CharVector, cObject;
class BinaryValue extends cObject;


packet OnionKey {
    //@customize(true);
    BinaryValue publicKey;
    BinaryValue privateKey;
    simtime_t expiration;
}