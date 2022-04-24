#ifndef __ASYMMETRICENCRYPTIONFLAG_H_
#define __ASYMMETRICENCRYPTIONFLAG_H_

#include "EncryptionFlagsBase_m.h"
#include "jsoncpp/json/json.h"

using std::string;


class AsymmetricEncryptionFlag : public AsymmetricEncryptionFlag_Base {
    private:
        void copy(const AsymmetricEncryptionFlag& other) {
            this->state = other.state;
            this->keySet = other.keySet;
        }

    public:
        AsymmetricEncryptionFlag() : AsymmetricEncryptionFlag_Base() {
            encrypt();
        }
        AsymmetricEncryptionFlag(const AsymmetricEncryptionFlag& other) : AsymmetricEncryptionFlag_Base(other) {copy(other);}
        AsymmetricEncryptionFlag& operator=(const AsymmetricEncryptionFlag& other) {if (this==&other) return *this; AsymmetricEncryptionFlag_Base::operator=(other); copy(other); return *this;}
        virtual AsymmetricEncryptionFlag *dup() const override {return new AsymmetricEncryptionFlag(*this);}


        AsymmetricEncryptionFlag(AsymmetricKeySet keySet) : AsymmetricEncryptionFlag() {
            this->keySet = keySet;
        }

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

        string toJsonString() {
            Json::Value obj;
            obj["state"] = state;
            obj["keySet"] = keySet.toJSONString();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, obj);
        }
};


inline AsymmetricEncryptionFlag getAsymmetricEncryptionFlagFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    int state = obj["state"].asInt();

    string jsonKeySet = obj["keySet"].asString();
    AsymmetricKeySet keySet = getKeySetFromJSON(jsonKeySet);

    AsymmetricEncryptionFlag flag = AsymmetricEncryptionFlag();
    flag.setState(state);
    flag.setKeySet(keySet);

    return flag;
}

#endif
