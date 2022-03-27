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


        CircuitEvidence(EvidenceType type, cPacket* request,
                        Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature,
                        OverlayKey circuitID) {
            this->type = type;
            this->producerCert = producerCert;
            this->producerSignature = producerSignature;
            this->circuitID = circuitID;
            this->encapsulate(request);
        }

        CircuitEvidence(CreateCircuitRequest request, Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature) :
        CircuitEvidence(
                CIRCUIT_CREATION,
                &request,
                producerCert,
                producerSignature,
                OverlayKey::sha1(BinaryValue(&request))) { }

        CircuitEvidence(ExtendCircuitRequest request, Certificate producerCert,
                        AsymmetricEncryptionFlag producerSignature) :
        CircuitEvidence(
                CIRCUIT_EXTENSION,
                &request,
                producerCert,
                producerSignature,
                OverlayKey::sha1(BinaryValue(&request))) { }

        bool isAuthenticated() {
            return producerCert.getIsSigned() &&
                   producerSignature.getState() == SIGNED &&
                   producerSignature.verify(
                       producerCert.getKeySet().getPublicKey()
                   );
        }

    private:
        void decrypt() {
            setIsEncrypted(false);
        }
};


#endif
