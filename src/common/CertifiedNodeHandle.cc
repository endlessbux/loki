#include "common/CertifiedNodeHandle.h"


CertifiedNodeHandle::CertifiedNodeHandle( ) {
    cert = new certificate;
}


CertifiedNodeHandle::CertifiedNodeHandle( CertifiedNodeHandle& source )
: NodeHandle( source.key, source.ip, source.port ) {
    cert = source.cert;
}


CertifiedNodeHandle::CertifiedNodeHandle( const OverlayKey& key, const L3Address& ip, int port, certificate& cert )
: NodeHandle( key, ip, port ) {
    this->cert = &cert;
}


CertifiedNodeHandle::CertifiedNodeHandle( const TransportAddress& ta )
: NodeHandle( ta ) {
    cert = new certificate;
}


CertifiedNodeHandle::CertifiedNodeHandle( const OverlayKey& key, const TransportAddress& ta, certificate& cert )
: NodeHandle( key, ta ) {
    this->cert = &cert;
}
