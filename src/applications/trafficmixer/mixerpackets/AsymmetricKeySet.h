#ifndef __ASYMMETRICKEYSET_H_
#define __ASYMMETRICKEYSET_H_

#include <omnetpp.h>
#include "CommonEncryptionPackets_m.h"
#include "common/OverlayKey.h"
#include "common/BinaryValue.h"
#include "jsoncpp/json/json.h"
#include <bits/stdc++.h>

using namespace std;


class AsymmetricKeySet : public AsymmetricKeySet_Base {
    private:
        void copy(const AsymmetricKeySet& other) {
            this->privateKey = other.privateKey;
            this->publicKey = other.publicKey;
        }
    public:
        AsymmetricKeySet() : AsymmetricKeySet_Base() {}
        AsymmetricKeySet(const AsymmetricKeySet& other) : AsymmetricKeySet_Base(other) {copy(other);}
        AsymmetricKeySet& operator=(const AsymmetricKeySet& other) {
            if (this==&other) return *this;
            AsymmetricKeySet_Base::operator=(other);
            copy(other);
            return *this;
        }
        AsymmetricKeySet(string publicKey, string privateKey) {
            this->publicKey = publicKey;
            this->privateKey = privateKey;
        }
        virtual AsymmetricKeySet *dup() const override {
            return new AsymmetricKeySet(*this);
        }

        // My methods
        bool operator==(const AsymmetricKeySet& other) {
            return this->publicKey == other.publicKey &&
                   this->privateKey == other.privateKey;
        }

        bool checkPublicKey(string publicKey) {
            return this->publicKey == publicKey;
        }

        bool checkPrivateKey(string privateKey) {
            return this->privateKey == privateKey;
        }

        string toJSONString() {
            Json::Value obj;
            obj["privateKey"] = privateKey.c_str();
            obj["publicKey"] = publicKey.c_str();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, obj);
        }
};

Register_Class(AsymmetricKeySet);


inline void doParsimPacking(omnetpp::cCommBuffer *b, const AsymmetricKeySet& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, AsymmetricKeySet& obj) {obj.parsimUnpack(b);}


inline string generateRandomKey(double seed) {
    union doubleIntConvert {
        double dSeed;
        uint64_t iSeed;
    };
    doubleIntConvert keySeed;
    keySeed.dSeed = seed;
    string seedStr = std::bitset<64>(keySeed.iSeed).to_string();


    BinaryValue seedBin = BinaryValue(seedStr);
    return OverlayKey::sha1(seedBin).toString();
}

/**
 * Generates a new random keyset
 */
inline AsymmetricKeySet generateNewKeySet(double seed) {
    string publicKey = "AKEY__" + generateRandomKey(seed);
    string privateKey = "AKEY__" + generateRandomKey(seed + 1);

    AsymmetricKeySet keySet = AsymmetricKeySet();
    keySet.setPublicKey(publicKey.c_str());
    keySet.setPrivateKey(privateKey.c_str());

    return keySet;
}


/**
 * Generates an AsymmetricKeySet from a JSON encoded string
 *
 * @param jsonStr string the JSON encoded AsymmetricKeySet
 */
inline AsymmetricKeySet getKeySetFromJSON(string jsonStr) {
    Json::Value obj;
    Json::Reader reader;
    reader.parse(jsonStr, obj);


    string pub = obj["publicKey"].asString();
    string priv = obj["privateKey"].asString();
    return AsymmetricKeySet(pub, priv);
}

#endif
