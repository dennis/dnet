#include <dnet/time.hpp>
#include <enet/enet.h>

namespace dnet {
uint32_t  dnettime() {
	return enet_time_get();
}
}
