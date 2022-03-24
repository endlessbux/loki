#ifndef __DHTMEDIATOR_H_
#define __DHTMEDIATOR_H_

#include "tier2/dhttestapp/DHTTestApp.h"
#include "applications/trafficmixer/mixerpackets/CircuitEvidence.h"


class DHTMediator : public DHTTestApp {
    public:
        DHTMediator();
        virtual ~DHTMediator();

        int storeEvidence(const CircuitEvidence evidence, const int ttl);

    private:

        virtual void handleTimerEvent(cMessage* msg) override;

        /**
         * Requests a key-value data item through TIER1.
         *
         * @param key The key of the requested data item
         */
        void sendGetRequest(const OverlayKey& key);

        /**
         * Distributes a key-value data item through TIER1.
         *
         * @param key The overlay key assigned to the data item
         * @param value The value of the distributed data item
         * @param ttl The data item's Time-To-Live parameter in seconds
         * @param isModifiable Determines whether the data item can be
         *        modified after being distributed; defaults to true
         * @return nonce of the DHT put request
         */
        int sendPutRequest(
            const OverlayKey& key,
            const BinaryValue value,
            const int ttl,
            const bool isModifiable = true
        );

        /**
         * see DHTTestApp.h
         */
        void handleGetResponse(DHTgetCAPIResponse* msg,
                               DHTStatsContext* context) override;

        /**
         * see DHTTestApp.h
         */
        void handlePutResponse(DHTputCAPIResponse* msg,
                               DHTStatsContext* context) override;
};

#endif
