cplusplus {{
#include "AsymmetricKeySet.h"
}}

class noncobject AsymmetricKeySet;


packet Certificate {
    @customize(true);
    AsymmetricKeySet keySet;
    simtime_t expiration;
    bool isSigned = false;
}