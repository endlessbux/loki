#ifndef __KEYEXCHANGE_H_
#define __KEYEXCHANGE_H_

#include "KeyExchangeBase_m.h"


class KeyExchange : public KeyExchange_Base {
    /**
     * Class for simulating a key exchange through asymmetric encryption.
     */
    public:
        KeyExchange(const char *name=nullptr) : KeyExchange_Base(name) {}
        KeyExchange(const KeyExchange& other) : KeyExchange_Base(other) {}
        KeyExchange& operator=(const KeyExchange& other)
            {KeyExchange_Base::operator=(other); return *this;}
        virtual KeyExchange *dup() const {return new KeyExchange(*this);}

        /**
         * Simulate decryption of the exchanged symmetric key.
         *
         * @param key OnionKey The key-pair to be used for decryption
         * @return the un-encrypted symmetric key
         */
        BinaryValue getSymmetricKey(OnionKey key) {
            if(key.getPrivateKey() == getExchangeKey().getPrivateKey()) {
                return symmetricKey;
            }
            return new BinaryValue();
        }

        /**
         * Simulate encryption of a symmetric key.
         *
         * @param key OnionKey the key-pair used for encryption.
         */
        void encrypt(OnionKey key) {
            setExchangeKey(*key.dup());
        }

};

#endif
