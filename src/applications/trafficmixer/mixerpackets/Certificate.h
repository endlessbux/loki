#ifndef __CERTIFICATE_H_
#define __CERTIFICATE_H_

#include "CertificateBase_m.h"


class Conductor;

class Certificate : public Certificate_Base {
    friend class Conductor;

    /**
     * Class for simulating a cryptographic certificate.
     */
    public:
        Certificate(const char *name=nullptr) : Certificate_Base(name) {
            exchangeKey.setExpiration(SIMTIME_ZERO);
        }
        Certificate(const Certificate& other) : Certificate_Base(other) {}
        Certificate& operator=(const Certificate& other)
            {Certificate_Base::operator=(other); return *this;}
        virtual Certificate *dup() const {return new Certificate(*this);}

        bool isValid() {
            return !isExpired() && isAuthenticated();
        }

    private:
        bool isExpired() {
            return exchangeKey.getExpiration() < simTime();
        }

        bool isAuthenticated() {
            return getIsSigned();
        }

        void sign() {
            setIsSigned(true);
        }
};

#endif
