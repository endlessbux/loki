#ifndef __CERTIFICATE_H_
#define __CERTIFICATE_H_

#include "CertificateBase_m.h"
#include "jsoncpp/json/json.h"

using omnetpp::simTime;
using namespace std;


class Conductor;


class Certificate : public Certificate_Base {
    friend class Conductor;

    private:
        void copy(const Certificate& other) {
            this->isSigned = other.isSigned;
            this->expiration = other.expiration;
            this->keySet = other.keySet;
        }

        bool isExpired() {
            return simTime() > expiration;
        }

        void sign() {
            isSigned = true;
        }

    public:

        Certificate() : Certificate_Base() {}
        Certificate(AsymmetricKeySet key) : Certificate_Base() {
            keySet = key;
        }
        Certificate(const Certificate& other) : Certificate_Base(other) {copy(other);}
        Certificate& operator=(const Certificate& other) {if (this==&other) return *this; Certificate_Base::operator=(other); copy(other); return *this;}
        bool operator==(const Certificate& other) {
            return this->isSigned == other.isSigned &&
                   this->expiration == other.expiration &&
                   this->keySet == other.keySet;
        }
        bool operator!=(const Certificate& other) {
            return !(*this == other);
        }
        virtual Certificate *dup() const override {return new Certificate(*this);}


        bool isValid() {
            return isSigned && !isExpired();
        }

        string toJsonString() {
            Json::Value obj;
            obj["isSigned"] = isSigned;
            obj["rawExpiration"] = (int)expiration.raw();
            obj["keySet"] = keySet.toJSONString();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, obj);
        }
};

inline void doParsimPacking(cCommBuffer *b, Certificate& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(cCommBuffer *b, Certificate& obj) {obj.parsimUnpack(b);}


inline Certificate getCertificateFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    bool isSigned = obj["isSigned"].asBool();

    int rawExpiration = obj["rawExpiration"].asInt();
    simtime_t expiration = SimTime::fromRaw(rawExpiration);

    string jsonKeySet = obj["keySet"].asString();
    AsymmetricKeySet keySet = getKeySetFromJSON(jsonKeySet);


    Certificate cert = Certificate();
    cert.setIsSigned(isSigned);
    cert.setExpiration(expiration);
    cert.setKeySet(keySet);

    return cert;
}


inline Certificate generateNewCertificate(double seed) {
    AsymmetricKeySet keySet = generateNewKeySet(seed);
    Certificate cert = Certificate();
    cert.setKeySet(keySet);

    return cert;
}

#endif
