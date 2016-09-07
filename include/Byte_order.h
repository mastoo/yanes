/**
 * \brief Functions and macros to convert between host and network byte order.
 *
 * This files provides
 * - Macros HTONS(), HTONL(), NTOHS() and NTOHL()
 * - Inline functions htons(), htonl(), ntohs() and ntohl()
 *
 * If some of these already exist on a platform, we include the existing ones.
 * Otherwise we implement them ourselves.
 *
 * You are invited to add mechanisms to detect additional
 * platforms, compilers, operating systems, frameworks etc.
 *
 * Currently supported:
 * - Linux include detection
 * - MS Windows include detection
 * - GCC endianness detection
 *
 * File available at: http://www.mikrocontroller.net/articles/Datei:Byte_order.h
 *
 * \date 2013-02-28
 */

#ifndef BYTE_ORDER_H_
#define BYTE_ORDER_H_

#include <stdint.h>

// Linux?
#if defined(__linux__) || defined(__linux) || defined(_linux) || defined(linux)
	#include <netinet/in.h>
	#define BYTE_ORDER_FUNCTIONS_FOUND_
#endif

// Windows?
#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
	#include <winsock2.h>
	#define BYTE_ORDER_FUNCTIONS_FOUND_
	#define BYTE_ORDER_IS_LITTLE_ENDIAN_
#endif

// GCC byte order defined?
#if defined(__BYTE_ORDER__)
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define BYTE_ORDER_IS_LITTLE_ENDIAN_
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define BYTE_ORDER_IS_BIG_ENDIAN_
	#else
		#error Byte order not supported!
	#endif
#endif

// Define HTONS() and HTONL()
#if defined(BYTE_ORDER_IS_LITTLE_ENDIAN_)
	#ifndef HTONS
		#define HTONS(x) ((uint16_t) (((uint16_t) (x) << 8) | ((uint16_t) (x) >> 8)))
	#endif
	#ifndef HTONL
		#define HTONL(x) ((uint32_t) (((uint32_t) HTONS(x) << 16) | HTONS((uint32_t) (x) >> 16)))
	#endif
#elif defined(BYTE_ORDER_IS_BIG_ENDIAN_)
	#ifndef HTONS
		#define HTONS(x) ((uint16_t) (x))
	#endif
	#ifndef HTONL
		#define HTONL(x) ((uint32_t) (x))
	#endif
#else
	#error No byte order defined!
#endif

// Define NTOHS() and NTOHL()
#ifndef NTOHS
	#define NTOHS(x) HTONS(x)
#endif
#ifndef NTOHL
	#define NTOHL(x) HTONL(x)
#endif

// Define htons(), htonl(), ntohs() and ntohl()
#ifndef BYTE_ORDER_FUNCTIONS_FOUND_
static inline uint16_t htons(uint16_t x) {
	return HTONS(x);
}
static inline uint32_t htonl(uint32_t x) {
	return HTONL(x);
}
static inline uint16_t ntohs(uint16_t x) {
	return htons(x);
}
static inline uint32_t ntohl(uint32_t x) {
	return htonl(x);
}
#endif /* BYTE_ORDER_FUNCTIONS_FOUND_ */

#endif /* BYTE_ORDER_H_ */
