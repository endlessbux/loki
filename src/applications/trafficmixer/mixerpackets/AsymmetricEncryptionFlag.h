#ifndef __ASYMMETRICENCRYPTIONFLAG_H_
#define __ASYMMETRICENCRYPTIONGLAG_H_

#include "CommonEncryptionPackets_m.h"
#include "EncryptionFlagsBase_m.h"

using std::string;


class AsymmetricEncryptionFlag : public AsymmetricEncryptionFlag_Base {
    private:
        void copy(const AsymmetricEncryptionFlag& other) {
            this->state = other.state;
            this->keySet = other.keySet;
        }

    public:
        AsymmetricEncryptionFlag() : AsymmetricEncryptionFlag_Base() {}
        AsymmetricEncryptionFlag(const AsymmetricEncryptionFlag& other) : AsymmetricEncryptionFlag_Base(other) {copy(other);}
        AsymmetricEncryptionFlag& operator=(const AsymmetricEncryptionFlag& other) {if (this==&other) return *this; AsymmetricEncryptionFlag_Base::operator=(other); copy(other); return *this;}
        virtual AsymmetricEncryptionFlag *dup() const override {return new AsymmetricEncryptionFlag(*this);}


        void decrypt(string decryptionKey) {
            if(keySet.checkPrivateKey(decryptionKey))
                state = PLAINTEXT;
        }

        void encrypt() {
            if(state == PLAINTEXT)
                state = ENCRYPTED;
        }

        void sign() {
            if(state == PLAINTEXT)
                state = SIGNED;
        }

        bool verify(string publicKey) {
            return keySet.checkPublicKey(publicKey);
        }
};

Register_Class(AsymmetricEncryptionFlag);

#endif
