#include "string.h"
#include "stdint.h"

const char * strchr(const char * str, char  chr)
{
    if(str == NULL)
    {
		return NULL;
    }

    while(*str)
    {
		if(*str == chr)
		{
			return str;
		}
		++str;
    }
    return NULL;
}
char *strcpy(char *dst, const char *src)
{
    char * origDst = dst;
    if(dst == NULL)
    {
		return NULL;	
    }
    if(src == NULL)
    {
		*dst = '\0';
		return dst;
    }

    while(*src)
    {
		*dst = *src;
		++src;
		++dst;
    }

    *dst = '\0';
    return origDst;
}

unsigned strlen(const char * str)
{
    unsigned len = 0;
    while(*str)
    {
		len++;
		str++;
    }
    return len;
}
char *strncpy(char *dst, const char *src, int n)
{
    char * origDst = dst;
    if(dst == NULL)
    {
		return NULL;	
    }
    if(src == NULL)
    {
		*dst = '\0';
		return dst;
    }
    int i = 0; 
    while(*src && ++i <= n)
    {
		*dst = *src;
		++src;
		++dst;
    }

    *dst = '\0';
    return origDst;
}

char *strcat(char *dst, const char *src)
{
    // make 'ptr point to the end of the destination string
    char *ptr = dst + strlen(dst);
    while(*src != '\0')
		*ptr++ = *src++;

    *ptr = '\0';

    return dst;
}
char *strncat(char *dst, const char *src, int n)
{
    int i = 0;
    // make 'ptr point to the end of the destination string
    char *ptr = dst + strlen(dst);
    while(*src != '\0' && ++i <= n)
		*ptr++ = *src++;

    *ptr = '\0';

    return dst;   
}
int strcmp(const char *p1, const char *p2)
{
    register const unsigned char *s1 = (const unsigned char*)p1;
    register const unsigned char *s2 = (const unsigned char*)p2;
    unsigned register char c1, c2;
    do
    {
		c1 = (unsigned char)*s1++;
		c2 = (unsigned char)*s2++;
		if(c1 == '\0')
			return c1 - c2;
    } while(c1 == c2);
    return c1 - c2;
}
