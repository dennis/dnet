
#include <dnet/buffer.hpp>
#include <string.h>		// for memcpy etc
#include <assert.h>

#define MIN(x,y) (x < y ? x : y)

namespace dnet {

Buffer::Buffer(int len) {
	bufferLen	= len+1;
	buffer		= new uint8_t[bufferLen];

	readIdx		= 0;
	writeIdx	= 0;

	assert( size() == 0 );
}

Buffer::Buffer(const Buffer& src) {
	bufferLen	= src.bufferLen;
	buffer		= new uint8_t[bufferLen];

	readIdx		= src.readIdx;
	writeIdx	= src.writeIdx;

	memcpy(buffer, src.buffer, bufferLen);

	assert( size() == src.size());

	LOG_BUF("Buffer(Copy) size=%d\n", size());
}

Buffer::~Buffer() {
	delete[] buffer;
}

Buffer& Buffer::operator=(const Buffer& src) {
	if( bufferLen ) {
		delete[] buffer;
	}
	bufferLen	= src.bufferLen;
	buffer		= new uint8_t[bufferLen];

	readIdx		= src.readIdx;
	writeIdx	= src.writeIdx;

	memcpy(buffer, src.buffer, bufferLen);

	assert( size() == src.size());

	LOG_BUF("Buffer::operator= size=%d\n", size());

	return *this;
}

int Buffer::peek(uint8_t* dst, int len) const {
	if( size() < len )
		len = size();

	// No data
	if( len == 0 )
		return 0;

	// No wrap
	if( readIdx < writeIdx  ) {
		assert(len>0);
		memcpy(dst, &buffer[readIdx], len );
		//readIdx += len;
	}

	// Wrap
	else {
		int endReadSize 	= bufferLen-readIdx;
		int beginReadSize 	= writeIdx;

		endReadSize			= MIN(endReadSize,len);
		beginReadSize		= MIN(beginReadSize, (len - endReadSize));

		if( endReadSize > 0 ) {
			memcpy(dst,&buffer[readIdx],endReadSize);
			//readIdx += endReadSize;
		}
		if( beginReadSize > 0 ) {
			memcpy(&dst[endReadSize],buffer,beginReadSize);
			//readIdx += beginReadSize;
		}
	}
	//readIdx = readIdx % bufferLen;

	return len;
}

int Buffer::discard(int len)  {
	if( size() < len )
		len = size();

	// No data
	if( len == 0 )
		return 0;

	// No wrap
	if( readIdx < writeIdx  ) {
		readIdx += len;
	}

	// Wrap
	else {
		int endReadSize 	= bufferLen-readIdx;
		int beginReadSize 	= writeIdx;

		endReadSize			= MIN(endReadSize,len);
		beginReadSize		= MIN(beginReadSize, (len - endReadSize));
		if( endReadSize > 0 ) {
			readIdx += endReadSize;
		}
		if( beginReadSize > 0 ) {
			readIdx += beginReadSize;
		}
	}
	readIdx = readIdx % bufferLen;

	return len;
}

int Buffer::write(uint8_t* src, int len) {
	assert( capacity() - size() >= 0 );
	if( capacity() - size() < len )
		len = capacity() - size();

	// No data
	if( len == 0 )
		return 0;
		
	assert(len > 0);
	
	// No wrap
	if( writeIdx < readIdx )  {
		memcpy(&buffer[writeIdx],src,len);
		writeIdx += len;
	}

	// Wrap
	else {
		int endWriteSize 	= bufferLen-writeIdx;
		int beginWriteSize 	= readIdx;

		endWriteSize		= MIN(endWriteSize,len);
		beginWriteSize		= MIN(beginWriteSize, (len - endWriteSize));

		if( endWriteSize > 0 ) { 
			memcpy(&buffer[writeIdx],src,endWriteSize);
			writeIdx += endWriteSize;
		}
		if( beginWriteSize > 0 ) {
			memcpy(buffer,&src[endWriteSize],beginWriteSize);
			writeIdx += beginWriteSize;
		}
	}
	writeIdx = writeIdx % bufferLen;

	return len;
}

int Buffer::read(uint8_t* dst, int len) {
	len  = peek(dst,len);
	return discard(len);
}

int Buffer::size() const {
	// No data
	if( readIdx == writeIdx ) return 0;

	// No Wrap
	if( readIdx < writeIdx  ) {
		return writeIdx - readIdx;
	}

	// Wrap
	else{
		return writeIdx + bufferLen - readIdx;
	}
}

uint8_t* Buffer::peekData()  {
	uint8_t *ptr = getData();

	readIdx = 0;

	return ptr;
}

uint8_t* Buffer::getData()  {
	// TODO Make this smarter
	// 1) allocate a new buffer
	// 2) copy data to it (thereby making it sequential)
	// 3) mark it all as read
	// 4) return pointer to buffer

	// Only bother if readIdx isnt at the beginning of buffer
	if( readIdx != 0 ) {
		uint8_t*	newBuf = new uint8_t[bufferLen];
		int s = size();
		read(newBuf,s);
		clear();
		write(newBuf,s);
		readIdx = 0;
		writeIdx = s;
		delete[] newBuf;
	}
	return buffer;
}

void Buffer::dump() {
	int bufSize = size();
	uint8_t *ptr = getData();
	for(int i = 0; i < bufSize; i++) {
		printf("  %x: 0x%02x %-3d [%c] ", ((unsigned int)ptr+i), ptr[i], ptr[i], ptr[i] < 32 ? '?' :  (char)ptr[i]);
		if( (i % 3) == 2  ) 
			puts("");
	}
	puts("");

	readIdx = 0;
}

void Buffer::moveFrom( Buffer& src) {
	write( src.getData(), src.size() );
}
/*
#include <stdio.h>
#include <assert.h>
int main() {
	Buffer	buf(10);
	char	buffer[1024];
	int 	size;
	// Buffer should be empty now
	assert( buf.size() == 0 && buf.empty() );
	assert( buf.peek(buffer,20) == 0 );
	assert( buf.capacity() == 10 );

	// Add a bute, check if we're still ok
	assert( buf.write("1",1) == 1 );
	assert( buf.size() == 1 && !buf.empty() );
	assert( buf.peek(buffer,20) == 1 );
	assert( buffer[0] == '1' );

	// Remove it again
	assert( buf.discard(20) == 1 );
	assert( buf.size() == 0 && buf.empty() );

	// Put too much data in (we got 10 bytes availble, lets try putting 14 in)
	assert( buf.write("1234567", 7) == 7 );
	assert( buf.write("1234567", 7) == 3 );
	assert( buf.size() == buf.capacity() && !buf.empty() );

	// Read data in chunks of 3 bytes
	assert( buf.read(buffer,3) == 3 );
	assert( buf.read(&buffer[3],3) == 3 );
	assert( buf.read(&buffer[6],3) == 3 );
	assert( buf.read(&buffer[9],3) == 1 );
	
	// Check contents of buffer
	assert( strncmp("1234567123", buffer, 10) == 0 );

	// Put 0-9 into buffer
	assert( buf.write("0123456789", 10 ) == 10 );

	// Read 2 bytes
	assert( buf.read(buffer,2) == 2 );

	// Add 3 new bytes
	assert( buf.write("abc", 3) == 2 );

	// Read all 10 bytes
	assert( buf.read(buffer,10) == 10 );
	assert( strncmp("23456789ab", buffer, 10) == 0 );

	buf.clear();

	assert( buf.size() == 0 );

	puts("Test is a success!");
}
*/

} // namespace dnet
