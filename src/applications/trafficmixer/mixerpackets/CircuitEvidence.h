#ifndef __CIRCUITEVIDENCE_H_
#define __CIRCUITEVIDENCE_H_

#include "CircuitEvidenceBase_m.h"
#include "CircuitBuildingMessages_m.h"
#include "ExtendCircuitCall.h"
#include "CreateCircuitCall.h"
#include "jsoncpp/json/json.h"


class Controller;

class CircuitEvidence : public CircuitEvidence_Base {
    friend class Controller;

    /**
     * Class to trace the bulding (creation and extension) of onion circuits back to the
     * origin.
     */
    public:

        // overridden
        CircuitEvidence(const char *name=nullptr) : CircuitEvidence_Base(name) {}
        CircuitEvidence(const CircuitEvidence& other) : CircuitEvidence_Base(other) {}
        CircuitEvidence& operator=(const CircuitEvidence& other)
            {CircuitEvidence_Base::operator=(other); return *this;}
        virtual CircuitEvidence *dup() const {return new CircuitEvidence(*this);}


        CircuitEvidence(EvidenceType type, string jsonCall,
                        Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature,
                        OverlayKey circuitID,
                        simtime_t timestamp) {
            this->type = type;
            this->producerCert = producerCert;
            this->producerSignature = producerSignature;
            this->circuitID = circuitID;
            this->jsonCall = jsonCall.c_str();
            this->timestamp = timestamp;
        }

        CircuitEvidence(CreateCircuitCall request, Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature) :
        CircuitEvidence(
                CIRCUIT_CREATION,
                request.toJsonString(),
                producerCert,
                producerSignature,
                OverlayKey::random(),
                simTime()) { }

        CircuitEvidence(ExtendCircuitCall request, Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature) :
        CircuitEvidence(
                CIRCUIT_EXTENSION,
                request.toJsonString(),
                producerCert,
                producerSignature,
                OverlayKey::random(),
                simTime()) { }

        bool isAuthenticated() {
            return producerCert.getIsSigned() &&
                   producerSignature.getState() == SIGNED &&
                   producerSignature.verify(
                       producerCert.getKeySet().getPublicKey()
                   );
        }

        BaseCallMessage* getExtendCircuitCallEvidence() {
            if(type == CIRCUIT_EXTENSION) {
                return getExtendCircuitCallFromJSON(jsonCall.c_str()).dup();
            }
            return getCreateCircuitCallFromJSON(jsonCall.c_str()).dup();
        }

        string toJsonString() {
            Json::Value obj;
            obj["type"] = type;
            obj["producerCert"] = producerCert.toJsonString();
            obj["producerSignature"] = producerSignature.toJsonString();
            obj["circuitID"] = circuitID.toString();
            obj["jsonCall"] = jsonCall.c_str();
            obj["timestampRaw"] = (int)timestamp.raw();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, obj);
        }

    private:
        void decrypt() {
            setIsEncrypted(false);
        }
};


inline CircuitEvidence getCircuitEvidenceFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    EvidenceType type = static_cast<EvidenceType>(obj["type"].asInt());

    string jsonProducerCert = obj["producerCert"].asString();
    Certificate producerCert = getCertificateFromJSON(jsonProducerCert);

    string jsonProducerSignature = obj["producerSignature"].asString();
    AsymmetricEncryptionFlag producerSignature =
            getAsymmetricEncryptionFlagFromJSON(jsonProducerSignature);

    string strCircuitID = obj["circuitID"].asString();
    OverlayKey circuitID = OverlayKey(strCircuitID);

    string jsonCall = obj["jsonCall"].asString();

    int64_t rawTimestamp = obj["timestampRaw"].asInt64();
    simtime_t timestamp = SimTime::fromRaw(rawTimestamp);


    CircuitEvidence evidence = CircuitEvidence(type, jsonCall, producerCert,
                                               producerSignature, circuitID,
                                               timestamp);
    return evidence;
}

#endif
