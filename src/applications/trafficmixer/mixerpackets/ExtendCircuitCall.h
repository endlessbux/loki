#ifndef __EXTENDCIRCUITCALL_H_
#define __EXTENDCIRCUITCALL_H_


#include "CircuitBuildingMessages_m.h"
#include "jsoncpp/json/json.h"
using namespace std;


class ExtendCircuitCall : public ExtendCircuitCall_Base {
private:
    void copy(const ExtendCircuitCall& other) {
        this->keyExchange = other.keyExchange;
        this->prevNodeCert = other.prevNodeCert;
        this->prevCircuitID = other.prevCircuitID;
    }

public:
    ExtendCircuitCall(const char *name=nullptr, short kind=0) : ExtendCircuitCall_Base(name,kind) {}
    ExtendCircuitCall(const ExtendCircuitCall& other) : ExtendCircuitCall_Base(other) {copy(other);}
    ExtendCircuitCall& operator=(const ExtendCircuitCall& other) {if (this==&other) return *this; ExtendCircuitCall_Base::operator=(other); copy(other); return *this;}
    virtual ExtendCircuitCall *dup() const override {return new ExtendCircuitCall(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from ExtendCircuitCall_Base

    string toJsonString() {
        Json::Value obj;
        obj["keyExchange"] = keyExchange.toJsonString();
        obj["prevNodeCert"] = prevNodeCert.toJsonString();
        obj["prevCircuitID"] = prevCircuitID.toString();

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, obj);
    }
};


inline ExtendCircuitCall getExtendCircuitCallFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    string jsonKeyExchange = obj["keyExchange"].asString();
    KeyExchange keyExchange = getKeyExchangeFromJSON(jsonKeyExchange);

    string jsonPrevNodeCert = obj["prevNodeCert"].asString();
    Certificate prevNodeCert = getCertificateFromJSON(jsonPrevNodeCert);

    string strPrevCircuitID = obj["prevCircuitID"].asString();
    OverlayKey prevCircuitID = OverlayKey(strPrevCircuitID);


    ExtendCircuitCall call = ExtendCircuitCall();
    call.setKeyExchange(keyExchange);
    call.setPrevNodeCert(prevNodeCert);
    call.setPrevCircuitID(prevCircuitID);

    return call;
}

#endif
