#include <dnet/dnet.hpp>
#include <dnet/packet.hpp>

#include <time.h>
#include <sys/time.h>

///  The namespace in which all of dnet resides in
namespace dnet { 

/**
 * Initializes dnet networking library. Must be called before ANY other
 * dnet library call
 */
bool init() {
	bool ok = (enet_initialize() == 0);
	if( ok ) 
		atexit( enet_deinitialize );
	PacketParser::Initialize();
	(void)dnettime();		// Make sure it initializes
	return ok;
}

/**
 * Finalize dnet - must be called to release resources.
 */
void shutdown() {
}

/**
 * This anonmous union is purely a sanity check - force the compiler to report an error if 
 * length of the types isnt as expected
 */
/* Gnu++ complains about this - since it is not used
static union {
    char    int8_t_incorrect[sizeof(  int8_t) == 1];
    char   uint8_t_incorrect[sizeof( uint8_t) == 1];
    
    char   int16_t_incorrect[sizeof( int16_t) == 2];
    char  uint16_t_incorrect[sizeof(uint16_t) == 2];
    
    char   int32_t_incorrect[sizeof( int32_t) == 4];
    char  uint32_t_incorrect[sizeof(uint32_t) == 4];
    
    char   int64_t_incorrect[sizeof( int64_t) == 8];
    char  uint64_t_incorrect[sizeof(uint64_t) == 8];

    char   float_incorrect[sizeof( float ) == 4];
};
*/

} // namespace dnet
