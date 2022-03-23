cplusplus {{
#include "OnionKey_m.h"
}}


class OnionKey;


packet Certificate {
	@customize(true);
	OnionKey exchangeKey;
	bool isSigned = false;
}
