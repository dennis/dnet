#ifndef INCLUDE_DNET_BUFFER_HPP
#define INCLUDE_DNET_BUFFER_HPP

#include <stdint.h>
#include <assert.h>
#include <dnet/pack.hpp>
#include <dnet/common.hpp>

namespace dnet {

class Buffer {
private:
	int			readIdx;
	int			writeIdx;
	uint8_t*	buffer;
	int			bufferLen;

public:
	Buffer(int=512);
	Buffer(const Buffer&);
	~Buffer();

	Buffer& operator=(const Buffer&);

	// Just look at the data, do not remove it from the buffer
	int peek(uint8_t*,int) const;

	// Remove data
	int discard(int);

	// Read data (same as peek, just with discard() of read data)
	int read(uint8_t*,int);
	int write(uint8_t*,int);
	uint8_t* getData();
	uint8_t* peekData();

	int capacity() const {
		return bufferLen-1;
	}
	int size() const;
	bool empty() const {
		return size() == 0;
	}
	int remaining() const {
		return capacity() - size();
	}

	void clear()  {
		readIdx = writeIdx = 0;
	}

	template<class T>
	Buffer& operator<<(T& v) {		// write to our buffer
		uint8_t	buf[512];
		int l = SizeOf(v);
		assert(l < 512 );

		//LOG_BUF("operator<<: remaining = %d, SizeOf(v) = %d, size()=%d\n", remaining(), l, size());
		
		if( remaining() < l ) {
			throw "buffer too short";	// FIXME
		}
		pack(buf,v);
		write(buf, l);
		return *this;
	}

	template<class T>
	Buffer& operator>>(T& v) {
		uint8_t* buf = peekData();
		
		//LOG_BUF("operator>>: remaining = %d, size()=%d\n", remaining(), size());

		assert( size() > 0 );
		
		uint8_t* ptr = static_cast<uint8_t*>(unpack(buf,v));
		discard(ptr - buf);
		return *this;
	}

	void moveFrom(Buffer&);

	void dump();
};


inline int SizeOf(const Buffer v) {
	return sizeof(uint16_t) + v.size();
}

inline void* pack(void* b, Buffer& v) {
	LOG_PACK("pack(%x,Buffer=%d)\n", (unsigned int)b, v.size());
	uint16_t l = SizeOf(v) - sizeof(uint16_t);
	b = pack(b,l);
	LOG_PACK(" Buffer is %d\n", l );
	v.read(static_cast<uint8_t*>(b),l);
	return ((void*)((char*)b + l + sizeof(uint16_t)));
}

inline void* unpack(void* b, Buffer& v) {
	LOG_PACK("unpack(%x, target(Buffer)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	uint16_t l;
	b = unpack(b,l);
	LOG_PACK(" Buffer is %d\n", l );
	v.write(static_cast<uint8_t*>(b),l);
	return ((void*)((char*)b + l + sizeof(uint16_t)));
}

} // namespace dnet

#endif
