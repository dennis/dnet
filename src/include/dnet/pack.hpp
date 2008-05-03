#ifndef INCLUDE_PACK_HPP
#define INCLUDE_PACK_HPP

#include <arpa/inet.h>	// htons..etc
#include <string>

#include <dnet/common.hpp>
#include <dnet/buffer.hpp>

namespace dnet {
// SizeOf

template<class T>
inline int SizeOf(const T) {
	return sizeof(T);
}

template<>
inline int SizeOf(const char* v) {
	return strlen(v);
}

template<>
inline int SizeOf(const std::string v) {
	return v.size()+1;
}

// Packers
template<class T> inline void* packRaw(void* b, const T& t, int size) {
	LOG_PACK("packRaw(target=%x, source=%x, size=%d)\n", (unsigned int)b, (unsigned int)&t, size );
	memcpy(b,&t,size);
	return ((void*)((char*)b + size));
}

template<class T> inline void* packRaw(void* b, const T& t) {
	LOG_PACK("packRaw(target=%x,source=%x)\n",(unsigned int)b, (unsigned int)&t );
	return packRaw(b,t,SizeOf(t));
}

inline void* pack(void* b, int8_t v) {
	LOG_PACK("pack(%x,int8_t=%d);\n", (unsigned int)b, v);
	return packRaw(b,v);
}

inline void* pack(void* b, uint8_t v) {
	LOG_PACK("pack(%x,uint8_t=%d);\n", (unsigned int)b, v);
	return packRaw(b,v);
}

inline void* pack(void* b, int16_t v) {
	LOG_PACK("pack(%x,int16_t=%d); (htons=%d)\n", (unsigned int)b, v, htons(v));
	//v = htons(v);
	return packRaw(b,v);
}

inline void* pack(void* b, uint16_t v) {
	LOG_PACK("pack(%x,uint16_t=%d); (htons=%d)\n", (unsigned int)b, v, htons(v));
	//v = htons(v);
	return packRaw(b,v);
}

inline void* pack(void* b, int32_t v) {
	LOG_PACK("pack(%x,int32_t=%d); (htons=%d)\n", (unsigned int)b, v, htons(v));
	//v = htonl(v);
	return packRaw(b,v);
}

inline void* pack(void* b, uint32_t v) {
	LOG_PACK("pack(%x,uint32_t=%d); (htons=%d)\n", (unsigned int)b, v, htons(v));
	//v = htonl(v)
	return packRaw(b,v);
}

inline void* pack(void* b, float v) {
	LOG_PACK("pack(%x,float=%f);\n", (unsigned int)b, v);
	return packRaw(b,v);
}

inline void* pack(void* b, std::string v) {
	LOG_PACK("pack(%x,std::string=%s)\n", (unsigned int)b, v.c_str());
	memcpy(b,v.c_str(),v.size()+1);
	return ((void*)((char*)b + SizeOf(v)));
}


// Unpackers
template<class T> inline void* unpackRaw(void* b, T& t) {
	LOG_PACK("unpackRaw(%x,target=%x,size=%d)\n", (unsigned int)b, (unsigned int)&t, sizeof(T));
	memcpy(&t,b,sizeof(T));
	return ((void*)((char*)b + SizeOf(t)));
}

inline void* unpack(void* b, int8_t& v) {
  LOG_PACK("unpack(%x, target(int8)=%x)\n",  (unsigned int)b, (unsigned int)&v);
  void* ptr = unpackRaw(b,v);
  LOG_PACK(".. value = %d\n", v);
  return ptr;
}

inline void* unpack(void* b, uint8_t& v) {
	LOG_PACK("unpack(%x, target(uint8)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b,v);
	LOG_PACK(".. value = %d\n", v);
  return ptr;
}

inline void* unpack(void* b, uint16_t& v) {
	LOG_PACK("unpack(%x, target(uint16)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b, v);
	LOG_PACK(".. value = %d ntohs = %d\n", v , (uint16_t)ntohs(v));
	//v = ntohs(v);
	return ptr;
}

inline void* unpack(void* b, int16_t& v) {
	LOG_PACK("unpack(%x, target(int16)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b, v);
	LOG_PACK(".. value = %d ntohs = %d\n", v , (int16_t)ntohs(v));
	//v = ntohs(v);
	return ptr;
}

inline void* unpack(void* b, uint32_t& v) {
	LOG_PACK("unpack(%x, target(uint32)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b, v);
	LOG_PACK(".. value = %d ntohs = %d\n", v , (uint32_t)ntohs(v));
	//v = ntohs(v);
	return ptr;
}

inline void* unpack(void* b, int32_t& v) {
	LOG_PACK("unpack(%x, target(int32)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b, v);
	LOG_PACK(".. value = %d ntohs = %d\n", v , (int32_t)ntohs(v));
	//v = ntohs(v);
	return ptr;
}

inline void* unpack(void* b, float& v) {
	LOG_PACK("unpack(%x, target(int32)=%x)\n",  (unsigned int)b, (unsigned int)&v);
	void* ptr = unpackRaw(b, v);
	LOG_PACK(".. value = %f\n", v);
	return ptr;
}

inline void* unpack(void* b, std::string& v) {
	char* ptr = static_cast<char*>(b);
	LOG_PACK("Unpacking string: %x %s (%d)\n", (unsigned int)b, ptr, strlen(ptr));	
	v = std::string(ptr);
	return ((void*)(ptr + SizeOf(v)));
}

} // namespace dnet

#endif
