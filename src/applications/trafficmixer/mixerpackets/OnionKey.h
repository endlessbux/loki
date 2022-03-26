#ifndef __ONIONKEY_H_
#define __ONIONKEY_H_

//#include "OnionKeyBase_m.h"
#include "OnionKey_m.h"


//class Conductor;
//
//class OnionKey : public OnionKey_Base {
//    friend class Condudctor;
//
//    /**
//     * Class to simulate asymmetric key exchange
//     */
//    public:
//        // overridden
//        OnionKey(const char *name=nullptr) : OnionKey_Base(name) {
//            expiration = SIMTIME_ZERO;
//        }
//        OnionKey(const OnionKey& other) : OnionKey_Base(other) {}
//        OnionKey& operator=(const OnionKey& other)
//            {OnionKey_Base::operator=(other); return *this;}
//        virtual OnionKey *dup() const {return new OnionKey(*this);}
//
//};

//inline void doParsimPacking(cCommBuffer *b, OnionKey& obj) {obj.parsimPack(b);}
//inline void doParsimUnpacking(cCommBuffer *b, OnionKey& obj) {obj.parsimUnpack(b);}

#endif
