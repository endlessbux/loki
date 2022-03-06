#ifndef __SIMULATEDCERTIFICATION_H_
#define __SIMULATEDCERTIFICATION_H_

#include "common/OverlayKey.h"


struct certificate {
    bool isNodeCertified { false };
    OverlayKey* peerKey { new OverlayKey(OverlayKey::UNSPECIFIED_KEY) };
};

#endif
