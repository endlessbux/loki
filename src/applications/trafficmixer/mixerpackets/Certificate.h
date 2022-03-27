#ifndef __CERTIFICATE_H_
#define __CERTIFICATE_H_

#include "CertificateBase_m.h"

using omnetpp::simTime;


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
        Certificate(const Certificate& other) : Certificate_Base(other) {copy(other);}
        Certificate& operator=(const Certificate& other) {if (this==&other) return *this; Certificate_Base::operator=(other); copy(other); return *this;}
        virtual Certificate *dup() const override {return new Certificate(*this);}


        bool isValid() {
            return isSigned && !isExpired();
        }
};

inline void doParsimPacking(cCommBuffer *b, Certificate& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(cCommBuffer *b, Certificate& obj) {obj.parsimUnpack(b);}

Register_Class(Certificate);

#endif
