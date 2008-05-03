#ifndef INCLUDE_DNET_HPP
#define INCLUDE_DNET_HPP

#include <dnet/client.hpp>
#include <dnet/server.hpp>
#include <dnet/dist.hpp>
#include <dnet/buffer.hpp>
#include <dnet/packet.hpp>

namespace dnet {

bool init();
void shutdown();

};
#endif // INCLUDE_DNET_HPP
