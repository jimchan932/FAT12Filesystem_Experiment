#include "ctype.h"
bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

char toupper(char chr)
{
    return islower(chr) ? chr -'a' + 'A' : chr;
}

bool isspace(char chr)
{
    return chr == ' ';
}

bool isupper(char chr)
{
    return chr >= 'A' && chr <= 'Z';
}

char tolower(char chr)
{
    return isupper(chr) ? chr - 'A' + 'a' : chr;
}

