#include <kcommon/memory.h>

unsigned int memidx(const void *in, char value, unsigned int count, unsigned int skip)
{

    const char *ip = in;

    for (; count; ip++, count--)
    {

        if (*ip == value)
        {

            if (skip)
                skip--;
            else
                break;

        }

    }

    return ip - (char *)in;

}

unsigned int memnidx(const void *in, char value, unsigned int count, unsigned int skip)
{

    const char *ip = (const char *) in + count - 1;

    for (; count > 0; ip--, count--)
    {

        if (*ip == value)
        {

            if (skip)
                skip--;
            else
                break;

        }

    }

    return count - 1;

}

void *memrplc(void *out, char value1, char value2, unsigned int len)
{
	char *op = out;

	for(; len; op++, len--)
	{
		if(*op == value1)
		{
			*op = value2;
		}
	}

	return out;
}

void memcpy(void *dest, const void *src, unsigned int len)
{
	const char *sp = (const void *)src;
	char *dp = (void *)dest;

	for(; len != 0; len--)
	{
		*dp++ = *sp++;
	}
}

void memset(void *dest, char val, unsigned int len)
{
	char *temp = (char *)dest;

	for(; len != 0; len--)
	{
		*temp++ = val;
	}
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, unsigned int len)
{
	unsigned short *temp = (unsigned short *)dest;

	for(; len != 0; len--)
	{
		*temp++ = val;
	}

	return dest;
}


int memcmp(const void *s1, const void *s2, unsigned int n)
{
	if(n)
	{
		const unsigned char *p1 = s1, *p2 = s2;
 
		do
		{
			if(*p1++ != *p2++)
			{
				return(*--p1 - *--p2);
			}
		}while(--n);
	}

	return 0;
}

