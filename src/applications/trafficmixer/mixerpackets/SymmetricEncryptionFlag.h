#ifndef __SYMMETRICENCRYPTIONFLAG_H_
#define __SYMMETRICENCRYPTIONFLAG_H_

#include "CommonEncryptionPackets_m.h"
#include "EncryptionFlagsBase_m.h"

using std::string;


class SymmetricEncryptionFlag : public SymmetricEncryptionFlag_Base {
    private:
        void copy(const SymmetricEncryptionFlag& other) {
            this->symmetricKey = other.symmetricKey;
            this->state = other.state;
        }

  public:
    SymmetricEncryptionFlag() : SymmetricEncryptionFlag_Base() {}
    SymmetricEncryptionFlag(const SymmetricEncryptionFlag& other) : SymmetricEncryptionFlag_Base(other) {copy(other);}
    SymmetricEncryptionFlag& operator=(const SymmetricEncryptionFlag& other) {if (this==&other) return *this; SymmetricEncryptionFlag_Base::operator=(other); copy(other); return *this;}
    virtual SymmetricEncryptionFlag *dup() const override {return new SymmetricEncryptionFlag(*this);}

    // own methods

    void decrypt(string decryptionKey) {
        if(decryptionKey == symmetricKey)
            state = PLAINTEXT;
    }

    void encrypt() {
        state = ENCRYPTED;
    }
};
Register_Class(SymmetricEncryptionFlag);


inline std::string generateNewSymmetricKey() {
    int seed = simTime().inUnit(SIMTIME_MS);
    std::string key = "SKEY__" + OverlayKey::sha1(seed).toString();

    return key;
}

#endif
