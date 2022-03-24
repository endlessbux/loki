#ifndef __CIRCUITEVIDENCE_H_
#define __CIRCUITEVIDENCE_H_

#include "CircuitEvidenceBase_m.h"
#include "CircuitBuildingMessages_m.h"


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


        CircuitEvidence(EvidenceType type, BinaryValue requestPayload,
                        Certificate producerCert, BinaryValue producerSignature,
                        OverlayKey circuitID) {
            this->type = type;
            this->requestPayload = requestPayload;
            this->producerCert = producerCert;
            this->producerSignature = producerSignature;
            this->circuitID = circuitID;
        }

        CircuitEvidence(CreateCircuitRequest request, Certificate producerCert,
                        BinaryValue producerSignature) :
        CircuitEvidence(
                CIRCUIT_CREATION,
                BinaryValue(&request),
                producerCert,
                producerSignature,
                OverlayKey::sha1(BinaryValue(&request))) { }

        CircuitEvidence(ExtendCircuitRequest request, Certificate producerCert,
                        BinaryValue producerSignature) :
        CircuitEvidence(CIRCUIT_EXTENSION,
                        BinaryValue(&request),
                        producerCert,
                        producerSignature,
                        OverlayKey::sha1(BinaryValue(&request))) { }

        bool isAuthenticated() {
            return producerCert.getIsSigned() &&
                  (producerCert.getExchangeKey().getPrivateKey() == producerSignature);
        }

    private:
        void decrypt() {
            setIsEncrypted(false);
        }
};


#endif
