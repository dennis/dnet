#include <stdio.h>
#include <dnet/dnet.hpp>

#define TEST(type, value)  { \
	type t = value; \
	ptr = dnet::pack(buffer, t); \
	dnet::unpack(buffer, t); \
	assert( t == value || #type " isnt working" ); \
	assert( ((char*)ptr - buffer) == sizeof(type) ); \
	puts(#type ".. works");\
	}

#define TESTCHARPTR(type, value) { \
	type t; t = (char*)malloc(255); strcpy(t,value); \
	memset(buffer, 0, 255); \
	ptr = dnet::pack(buffer, t); \
	dnet::unpack(buffer, t); \
	assert( strcmp(t,value) == 0  || #type " isnt working" ); \
	assert( ((char*)ptr - buffer) == sizeof(type) ); \
	puts(#type ".. works");\
	free(t); \
	}

void packunpack() {
	/*
	char buffer[1024];
	void* ptr;

	TEST(int8_t, 42 );
	TEST(uint8_t, 42 );

	TEST(int16_t, 511 );
	TEST(uint16_t, 255 );

	TEST(int32_t, 511 );
	TEST(uint32_t, 255 );

	TEST(float, 3.15 );
	TEST(float, 3.1415 );

	TEST(char, 'a' );
	TESTCHARPTR(char*, "testIng");

	TEST(std::string, "tessssssting");
	*/
}

int main() {
	dnet::Buffer aBuf;

	int8_t	a1, b1  = 1;
	int16_t a2, b2	= 2;
	int32_t a3, b3	= 3;
	uint8_t a4, b4	= 4;
	uint16_t a5, b5	= 5;
	uint32_t a6, b6	= 6;
	float a7, b7	= 42;
	//char a8, b8		= '8';
	float a8, b8 = 0;
	std::string a9("heeest");
	std::string b9("heeest");

	aBuf << a1 << a2 << a3;
	printf("aBuf size = %d, capacity = %d\n", aBuf.size(), aBuf.capacity() );
	aBuf <<	a4 << a5 << a6;
	printf("aBuf size = %d, capacity = %d\n", aBuf.size(), aBuf.capacity() );
	aBuf <<	a7 << a8 << a9;
	printf("aBuf size = %d, capacity = %d\n", aBuf.size(), aBuf.capacity() );
	aBuf.dump();

	aBuf >> b1 >> b2 >> b3 >>
		b4 >> b5 >> b6 >>
		b7 >> b8 >> b9;

	assert( a1 == b1 );	// int8_t
	assert( a2 == b2 );	// int16_t
	assert( a3 == b3 );	// int32_t
	assert( a4 == b4 ); // uint8_t
	assert( a5 == b5 ); // uint16_t
	assert( a6 == b6 ); // uint32_t
	assert( a7 == b7 ); // float
	//assert( a8 == b8 ); // char
	assert( a9 == b9 ); // std::string

	return 0;
}
