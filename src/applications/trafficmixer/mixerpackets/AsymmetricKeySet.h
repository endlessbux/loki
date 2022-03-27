#ifndef __ASYMMETRICKEYSET_H_
#define __ASYMMETRICKEYSET_H_

#include <omnetpp.h>
#include "CommonEncryptionPackets_m.h"
#include "common/OverlayKey.h"
#include "common/BinaryValue.h"


using std::string;


class AsymmetricKeySet : public AsymmetricKeySet_Base {
    private:
        void copy(const AsymmetricKeySet& other) {
            this->privateKey = other.privateKey;
            this->publicKey = other.publicKey;
        }
    public:
        AsymmetricKeySet() : AsymmetricKeySet_Base() {
            BinaryValue seed = BinaryValue(simTime().inUnit(SIMTIME_MS));
            string publicKey = "KEY__" + OverlayKey::sha1(seed).toString();
            string privateKey = "KEY__" + OverlayKey::sha1(seed).toString();
        }
        AsymmetricKeySet(const AsymmetricKeySet& other) : AsymmetricKeySet_Base(other) {copy(other);}
        AsymmetricKeySet& operator=(const AsymmetricKeySet& other) {
            if (this==&other) return *this;
            AsymmetricKeySet_Base::operator=(other);
            copy(other);
            return *this;
        }
        virtual AsymmetricKeySet *dup() const override {
            return new AsymmetricKeySet(*this);
        }

        // My methods

        bool checkPublicKey(string publicKey) {
            return this->publicKey == publicKey;
        }

        bool checkPrivateKey(string privateKey) {
            return this->privateKey == privateKey;
        }
};

Register_Class(AsymmetricKeySet);




/**
 * Generates a new random keyset
 */
inline AsymmetricKeySet generateNewKeySet() {
    string publicKey = "AKEY__" + generateRandomKey();

    string privateKey = "AKEY__" + generateRandomKey(1);

    AsymmetricKeySet keySet = AsymmetricKeySet();
    keySet.setPublicKey(publicKey.c_str());
    keySet.setPrivateKey(privateKey.c_str());

    return keySet;
}

#endif
