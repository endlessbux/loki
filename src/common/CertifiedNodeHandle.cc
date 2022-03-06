#include "common/CertifiedNodeHandle.h"


CertifiedNodeHandle::CertifiedNodeHandle( ) {
    ownCertificate = new certificate;
}


CertifiedNodeHandle::CertifiedNodeHandle( CertifiedNodeHandle& source )
: NodeHandle( source.key, source.ip, source.port ) {
    ownCertificate = source.ownCertificate;
}


CertifiedNodeHandle::CertifiedNodeHandle( const OverlayKey& key, const L3Address& ip, int port, certificate& cert )
: NodeHandle( key, ip, port ) {
    ownCertificate = &cert;
}


CertifiedNodeHandle::CertifiedNodeHandle( const TransportAddress& ta )
: NodeHandle( ta ) {
    ownCertificate = new certificate;
}


CertifiedNodeHandle::CertifiedNodeHandle( const OverlayKey& key, const TransportAddress& ta, certificate& cert )
: NodeHandle( key, ta ) {
    ownCertificate = &cert;
}
