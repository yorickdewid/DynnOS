#ifndef __MEMORY_H
#define __MEMORY_H

extern unsigned int memidx(const void *in, char value, unsigned int count, unsigned int skip);
extern unsigned int memnidx(const void *in, char value, unsigned int count, unsigned int skip);
extern void *memrplc(void *out, char value1, char value2, unsigned int len);
extern void memcpy(void *dest, const void *src, unsigned int len);
extern void memset(void *dest, char val, unsigned int len);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, unsigned int len);
extern int memcmp(const void *s1, const void *s2, unsigned int n);

#endif

