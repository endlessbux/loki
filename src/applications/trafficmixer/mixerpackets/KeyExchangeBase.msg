cplusplus {{
#include "common/BinaryValue.h"
#include "OnionKey.h"
}}


class noncobject BinaryValue;
class noncobject OnionKey;


packet KeyExchange {
    @customize(true);
    BinaryValue symmetricKey;
    OnionKey exchangeKey;
}