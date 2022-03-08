#ifndef __CERTIFIEDNODEHANDLE_H_
#define __CERTIFIEDNODEHANDLE_H_

#include "common/certificate.h"
#include "common/NodeHandle.h"
#include "common/TransportAddress.h"


class CertifiedNodeHandle : public NodeHandle {
    protected:
        certificate* cert;     // certificate belonging to this node

    public:
        /**
         * Initialises an unspecified CertifiedNodeHandle.
         */
        CertifiedNodeHandle( );

        /**
         * Copy constructor.
         *
         * @param source the instance to be copied.
         */
        CertifiedNodeHandle( CertifiedNodeHandle& source );

        /**
         * Complete constructor.
         *
         * @param key The OverlayKey.
         * @param ip The L3Address.
         * @param port The UDP-Port.
         * @param cert The certificate assigned to this node.
         */
        CertifiedNodeHandle( const OverlayKey& key, const L3Address& ip, int port, certificate& cert );

        /**
         * Constructor to generate a CertifiedNodeHandle from a TransportAddress.
         * The key and certificate will be unspecified.
         *
         * @param ta The transport address.
         */
        CertifiedNodeHandle( const TransportAddress& ta );

        /**
         * Constructor to generate a CertifiedNodeHandle from an existing
         * OverlayKey, TransportAddress, and certificate.
         *
         * @param key The overlay key.
         * @param ta The transport address.
         * @param cert The certificate assigned to this node.
         */
        CertifiedNodeHandle( const OverlayKey& key, const TransportAddress& ta, certificate& cert );
};

#endif
