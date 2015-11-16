#include <kcommon/memory.h>
#include <kcommon/string.h>

unsigned int stridx(const char *in, char value, unsigned int skip)
{
	return memidx(in, value, strlen(in), skip);
}

unsigned int strnidx(const char *in, char value, unsigned int skip)
{
	return memnidx(in, value, strlen(in), skip);
}

unsigned int strsplt(char *out[], char *in, char value)
{
	if(in[0] == '\0')
	{
		return 0;
	}

	unsigned int count = 1;
	out[0] = in;

	unsigned int i;

	for(i = 1; in[i] != '\0'; i++)
	{
		if(in[i - 1] == value)
		{
			in[i - 1] = '\0';
			out[count] = in + i;
			count++;
		}
	}

	return count;
}

int strcmp(char *str1, char *str2)
{
	int i = 0;
	int failed = 0;

	while(str1[i] != '\0' && str2[i] != '\0')
	{
		if(str1[i] != str2[i])
		{
			failed = 1;
			break;
		}
		i++;
	}

	if((str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0'))
	{
		failed = 1;
	}
  
	return failed;
}


int strncmp(char s1, char s2, unsigned long n)
{
	unsigned char u1, u2;

	while(n-- > 0)
	{
		u1 = (unsigned char) s1++;
		u2 = (unsigned char) s2++;

		if(u1 != u2)
		{
			return u1 - u2;
		}

		if(u1 == '\0')
		{
			return 0;
		}
	}

	return 0;
}


char *strcpy(char *dest, const char *src)
{
	while(*src)
	{
		*(dest++) =  *(src++);
	}
	*dest='\0';

	return dest;
}

char *strcat(char *dest, const char *src)
{
	char *save = dest;

	for (; *dest; ++dest);
	while ((*dest++ = *src++) != '\0');
	return(save);
}

int strlen(const char *src)
{
	int i = 0;

	while(*src++)
	{
		i++;
	}

	return i;
}

char *strchr(const char* s,int c)
{
	do
	{
		if(*s == c)
		{
			return(char*)s;
		}
	}while(*s++);

	return (0);
}

void itoa(char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;

	if(base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if(base == 'x')
	{
		divisor = 16;
	}
     
	do
	{
		int remainder = ud % divisor;
     
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while(ud /= divisor);

	*p = 0;

	p1 = buf;
	p2 = p - 1;

	while(p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

