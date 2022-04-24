#ifndef __KEYEXCHANGE_H_
#define __KEYEXCHANGE_H_


#include "CommonEncryptionPackets_m.h"
#include "KeyExchangeBase_m.h"
#include "jsoncpp/json/json.h"
#include "Certificate.h"

using std::string;

class KeyExchange : public KeyExchange_Base {
  private:
    void copy(const KeyExchange& other) {
        this->encryptionFlag = other.encryptionFlag;
        this->symmetricKey = other.symmetricKey;
    }

  public:
    KeyExchange() : KeyExchange_Base() {}
    KeyExchange(const KeyExchange& other) : KeyExchange_Base(other) {copy(other);}
    KeyExchange& operator=(const KeyExchange& other) {if (this==&other) return *this; KeyExchange_Base::operator=(other); copy(other); return *this;}
    virtual KeyExchange *dup() const override {return new KeyExchange(*this);}


    KeyExchange(Certificate cert, string key) : KeyExchange() {
        this->symmetricKey = key;
        this->encryptionFlag = AsymmetricEncryptionFlag(cert.getKeySet());
    }

    string getSymmetricKey() {
        if(encryptionFlag.getState() == ENCRYPTED) {
            return symmetricKey.str();
        }
        return "";
    }

    string getSymmetricKey(string decryptionKey) {
        encryptionFlag.decrypt(decryptionKey);
        return getSymmetricKey();
    }

    void encrypt() {
        encryptionFlag.encrypt();
    }

    string toJsonString() {
        Json::Value obj;
        obj["symmetricKey"] = symmetricKey.c_str();
        obj["encryptionFlag"] = encryptionFlag.toJsonString();

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, obj);
    }
};


inline KeyExchange getKeyExchangeFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    string symmetricKey = obj["symmetricKey"].asString();

    string jsonEncryptionFlag = obj["encryptionFlag"].asString();
    AsymmetricEncryptionFlag encryptionFlag =
            getAsymmetricEncryptionFlagFromJSON(jsonEncryptionFlag);

    KeyExchange exchange = KeyExchange();
    exchange.setSymmetricKey(symmetricKey.c_str());
    exchange.setEncryptionFlag(encryptionFlag);

    return exchange;
}

#endif
