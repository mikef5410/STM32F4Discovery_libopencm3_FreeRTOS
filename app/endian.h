/******************************************************************************
*
* NOT Copyright ... most of this was shamelessly
* stolen from NetBSD sources
* 
* 
* Filename:    endian.h  (named this to avoid confusion with toolkit
*                            supplied endian.h's )
*
* Description:  big-little-endian conversion stuff
*
* $Author$
*
* $DateTime$
*
* $Id$
*******************************************************************************/

#ifndef ENDIAN_H
#define ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif 


// #define	LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
// #define	BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
// #define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */

#if __MICROBLAZE__
#define BYTE_ORDER BIG_ENDIAN
#if 0
#define uint8_t  unsigned char
#define uint32_t unsigned long
#define uint16_t unsigned short
#define uint64_t unsigned long long
#endif
#include <stdint.h>
#endif

#ifdef __STM32__
#define BYTE_ORDER LITTLE_ENDIAN
#define uint8_t  unsigned short
#define uint16_t unsigned int
#define uint32_t unsigned long
#define uint64_t unsigned long long
#endif

#ifdef __NXP__
#define BYTE_ORDER LITTLE_ENDIAN
#define uint8_t  unsigned short
#define uint16_t unsigned int
#define uint32_t unsigned long
#define uint64_t unsigned long long
#endif

#if ( __i386__ || __x86_64__ )
#define BYTE_ORDER LITTLE_ENDIAN
#define uint8_t  unsigned char
#define uint32_t unsigned long
#define uint16_t unsigned short
#define uint64_t unsigned long long
#endif

//run-time check
//const int i = 1;
//#define is_bigendian() ( (*(char*)&i) == 0 )


static inline uint32_t bswap32(uint32_t x)
{
    return ((x << 24) & 0xff000000) |
	((x << 8) & 0x00ff0000) |
	((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
}

static inline uint16_t bswap16(uint16_t x)
{
    return ((x << 8) & 0xff00) | ((x >> 8) & 0x00ff);
}

static inline uint64_t bswap64(uint64_t x)
{
#ifdef _LP64
    /*
     * Assume we have wide enough registers to do it without touching
     * memory.
     */
    return ((x << 56) & 0xff00000000000000UL) |
	((x << 40) & 0x00ff000000000000UL) |
	((x << 24) & 0x0000ff0000000000UL) |
	((x << 8) & 0x000000ff00000000UL) |
	((x >> 8) & 0x00000000ff000000UL) |
	((x >> 24) & 0x0000000000ff0000UL) |
	((x >> 40) & 0x000000000000ff00UL) |
	((x >> 56) & 0x00000000000000ffUL);
#else
    /*
     * Split the operation in two 32bit steps.
     */
    uint32_t tl, th;

    th = bswap32((uint32_t) (x & 0x00000000ffffffffULL));
    tl = bswap32((uint32_t) ((x >> 32) & 0x00000000ffffffffULL));
    return ((uint64_t) th << 32) | tl;
#endif
}



#if BYTE_ORDER == BIG_ENDIAN

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define htobe64(x)	(x)
#define htole16(x)	bswap16((uint16_t)(x))
#define htole32(x)	bswap32((uint32_t)(x))
#define htole64(x)	bswap64((uint64_t)(x))

#define HTOBE16(x)	(void) (x)
#define HTOBE32(x)	(void) (x)
#define HTOBE64(x)	(void) (x)
#define HTOLE16(x)	(x) = bswap16((uint16_t)(x))
#define HTOLE32(x)	(x) = bswap32((uint32_t)(x))
#define HTOLE64(x)	(x) = bswap64((uint64_t)(x))

#else				/* LITTLE_ENDIAN */

#define htobe16(x)	bswap16((uint16_t)(x))
#define htobe32(x)	bswap32((uint32_t)(x))
#define htobe64(x)	bswap64((uint64_t)(x))
#define htole16(x)	(x)
#define htole32(x)	(x)
#define htole64(x)	(x)

#define HTOBE16(x)	(x) = bswap16((uint16_t)(x))
#define HTOBE32(x)	(x) = bswap32((uint32_t)(x))
#define HTOBE64(x)	(x) = bswap64((uint64_t)(x))
#define HTOLE16(x)	(void) (x)
#define HTOLE32(x)	(void) (x)
#define HTOLE64(x)	(void) (x)

#endif				/* LITTLE_ENDIAN */

#define be16toh(x)	htobe16(x)
#define be32toh(x)	htobe32(x)
#define be64toh(x)	htobe64(x)
#define le16toh(x)	htole16(x)
#define le32toh(x)	htole32(x)
#define le64toh(x)	htole64(x)

#define BE16TOH(x)	HTOBE16(x)
#define BE32TOH(x)	HTOBE32(x)
#define BE64TOH(x)	HTOBE64(x)
#define LE16TOH(x)	HTOLE16(x)
#define LE32TOH(x)	HTOLE32(x)
#define LE64TOH(x)	HTOLE64(x)

/*
 * Routines to encode/decode big- and little-endian multi-octet values
 * to/from an octet stream.
 */
#ifdef DEFINE_ENDIAN_INLINES

extern inline void be16enc(void *buf, uint16_t u)
{
    uint8_t *p = (uint8_t *) buf;

    p[0] = ((unsigned) u >> 8) & 0xff;
    p[1] = u & 0xff;
}

extern inline void le16enc(void *buf, uint16_t u)
{
    uint8_t *p = (uint8_t *) buf;

    p[0] = u & 0xff;
    p[1] = ((unsigned) u >> 8) & 0xff;
}

extern inline uint16_t be16dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return ((p[0] << 8) | p[1]);
}

extern inline uint16_t le16dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return ((p[1] << 8) | p[0]);
}

extern inline void be32enc(void *buf, uint32_t u)
{
    uint8_t *p = (uint8_t *) buf;

    p[0] = (u >> 24) & 0xff;
    p[1] = (u >> 16) & 0xff;
    p[2] = (u >> 8) & 0xff;
    p[3] = u & 0xff;
}

extern inline void le32enc(void *buf, uint32_t u)
{
    uint8_t *p = (uint8_t *) buf;

    p[0] = u & 0xff;
    p[1] = (u >> 8) & 0xff;
    p[2] = (u >> 16) & 0xff;
    p[3] = (u >> 24) & 0xff;
}

extern inline uint32_t be32dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

extern inline uint32_t le32dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

extern inline void be64enc(void *buf, uint64_t u)
{
    uint8_t *p = (uint8_t *) buf;

    be32enc(p, (uint32_t) (u >> 32));
    be32enc(p + 4, (uint32_t) (u & 0xffffffffULL));
}

extern inline void le64enc(void *buf, uint64_t u)
{
    uint8_t *p = (uint8_t *) buf;

    le32enc(p, (uint32_t) (u & 0xffffffffULL));
    le32enc(p + 4, (uint32_t) (u >> 32));
}

extern inline uint64_t be64dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return (((uint64_t) be32dec(p) << 32) | be32dec(p + 4));
}

extern  inline uint64_t le64dec(const void *buf)
{
    const uint8_t *p = (const uint8_t *) buf;

    return (le32dec(p) | ((uint64_t) le32dec(p + 4) << 32));
}
#endif

#ifdef __cplusplus
}
#endif

#endif				/* ENDIAN_H */
