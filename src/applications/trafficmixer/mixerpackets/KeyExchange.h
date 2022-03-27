#ifndef __KEYEXCHANGE_H_
#define __KEYEXCHANGE_H_

#include "EncryptionPackets_m.h"
using std::string;

class KeyExchange : public KeyExchange_Base {
  private:
    void copy(const KeyExchange& other) {
        this->encryptionFlag = other->encryptionFlag;
        this->symmetricKey = other->symmetricKey;
    }

  public:
    KeyExchange() : KeyExchange_Base() {}
    KeyExchange(const KeyExchange& other) : KeyExchange_Base(other) {copy(other);}
    KeyExchange& operator=(const KeyExchange& other) {if (this==&other) return *this; KeyExchange_Base::operator=(other); copy(other); return *this;}
    virtual KeyExchange *dup() const override {return new KeyExchange(*this);}

    string getSymmetricKey() override {
        if(encryptionFlag.getState() == ENCRYPTED) {
            return symmetricKey;
        }
        return "";
    }

    string getSymmetricKey(string decryptionKey) {
        encryptionFlag.decrypt(decryptionKey);
        return getSymmetricKey;
    }

    void encrypt() {
        encryptionFlag.encrypt();
    }
};

Register_Class(KeyExchange);

#endif
