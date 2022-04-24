#ifndef __CREATECIRCUITCALL_H_
#define __CREATECIRCUITCALL_H_

#include "CircuitBuildingMessages_m.h"
#include "jsoncpp/json/json.h"
using namespace std;


class CreateCircuitCall : public CreateCircuitCall_Base {
private:
    void copy(const CreateCircuitCall& other) {
        this->creatorCert = other.creatorCert;
        this->keyExchange = other.keyExchange;
    }

public:
    CreateCircuitCall(const char *name=nullptr, short kind=0) : CreateCircuitCall_Base(name,kind) {}
    CreateCircuitCall(const CreateCircuitCall& other) : CreateCircuitCall_Base(other) {copy(other);}
    CreateCircuitCall& operator=(const CreateCircuitCall& other) {if (this==&other) return *this; CreateCircuitCall_Base::operator=(other); copy(other); return *this;}
    virtual CreateCircuitCall *dup() const override {return new CreateCircuitCall(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from CreateCircuitCall_Base

    string toJsonString() {
        Json::Value obj;
        obj["creatorCert"] = creatorCert.toJsonString();
        obj["keyExchange"] = keyExchange.toJsonString();

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, obj);
    }
};


inline CreateCircuitCall getCreateCircuitCallFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    string jsonCert = obj["creatorCert"].asString();
    Certificate cert = getCertificateFromJSON(jsonCert);

    string jsonKeyExchange = obj["keyExchange"].asString();
    KeyExchange keyExchange = getKeyExchangeFromJSON(jsonKeyExchange);


    CreateCircuitCall call = CreateCircuitCall();
    call.setCreatorCert(cert);
    call.setKeyExchange(keyExchange);

    return call;
}

#endif
