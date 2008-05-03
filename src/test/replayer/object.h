#include <dnet/dnet.hpp>
#include "history.h"

class ReplayTank : public dnet::NetObject {
public:

private:
	float x, y, angle, vx, vy;
	uint8_t active;
public:
	ReplayTank() : x(0), y(0), angle(0), vx(0), vy(0), active(false) {
	}

	ReplayTank& operator=(const Tank& t) {
		x = t.x();
		y = t.y();
		vx = t.vx();
		vy = t.vy();
		angle = t.angle();
		active = t.isActive();

		return *this;
	}

	DNET_DESC(ReplayTank,
		DNET_ATTRIBUTE(x,30);
		DNET_ATTRIBUTE(y,30);
		DNET_ATTRIBUTE(angle,30);
		DNET_ATTRIBUTE(vx,30);
		DNET_ATTRIBUTE(vy,30);
		DNET_ATTRIBUTE(active,10);
	);
};

DNET_REGISTER(ReplayTank,500,1000);
