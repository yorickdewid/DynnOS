#ifndef __STRING_H
#define __STRING_H

#define NULL	0
#define true	TRUE
#define false	FALSE
#define FALSE	0
#define TRUE	1


extern unsigned int stridx(const char *in, char value, unsigned int skip);
extern unsigned int strnidx(const char *in, char value, unsigned int skip);
extern unsigned int strsplt(char *out[], char *in, char value);
extern int strcmp(char *str1, char *str2);
extern int strncmp(char s1, char s2, unsigned long n);
extern char *strcpy(char *dest, const char *src);
extern char *strcat(char *dest, const char *src);
extern int strlen(const char *src);
extern char *strchr(const char* s,int c);
extern void itoa(char *buf, int base, int d);

#endif

