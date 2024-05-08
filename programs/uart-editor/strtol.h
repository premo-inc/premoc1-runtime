#include <limits.h>
#define NUL '\0'

#define BOOL char
#define TRUE 1
#define FALSE 0

unsigned long ustrtol(const char *nPtr, char **endPtr, int base)
{
    // checking if the base value is correct
    if ((base < 2 || base > 36) && base != 0)
    {
        return 0;
    }

    unsigned long number = 0;
    const char *divider;
    int currentdigit;
    unsigned long cutoff;
    BOOL correctconversion = TRUE;

    divider = nPtr;

    // looking for a space if the beggining of the string is moved further
    while ((*divider) == ' ' || (*divider) == '\r' || (*divider) == '\n')
        divider++;

    if (*divider == NUL)
    {
        *endPtr = (char *)divider;
        return 0;
    }

    if (*divider < '0' || (*divider > '9' && *divider < 'A') || (*divider > 'z'))
        return 0;

    if ((base == 8) && (*divider == '0'))
    {
        divider++;
        if (*divider == 'o' || *divider == 'O') // if the input includes 'o', it's skipped
            divider++;
    }
    else if ((base == 16))
    {
        if (*divider == '0')
        {
            divider++;
            if (*divider == 'x' || *divider == 'X')
            {
                divider++;
                if (*divider > 'f' || *divider > 'F')
                {
                    divider--;
                    *endPtr = (char *)divider;
                    return 0;
                }
            }
            else
                divider--;
        }
        // basically the system-detecting algorithm
    }
    else if (base == 0)
    {
        if (*divider == '0')
        {
            divider++;
            if (*divider == 'o' || *divider == 'O')
            {
                base = 8;
                divider++;
            }
            else if (*divider == 'x' || *divider == 'X')
            {
                base = 16;
                divider++;
            }
            else
            {
                *endPtr = (char *)divider;
                return 0;
            }
        }
        else if (*divider >= '1' && *divider <= '9')
        {
            base = 10;
        }
    }

    // looping until the end of the input string
    // searching for convertable characters
    while (*divider != NUL)
    {
        if ((*divider) >= '0' && (*divider) <= '9')
            currentdigit = *divider - '0'; // converting to the actual integer
        else
        {
            char t;
            if ((*divider) >= 'a' && (*divider) <= 'z')
            {
                t = *divider - ('a' - 'A');
            }
            else
            {
                t = *divider;
            }

            if ((t - 'A') + 10 < base)
                currentdigit = (t - 'A') + 10;
            else
                break;
        }
        number = (number * base) + currentdigit;
        divider++;
    }
    
    if (endPtr != NUL)
    {
        if ((*divider) == ' ' || (*divider) == '\r' || (*divider) == '\n') // checking if the number is separated
            divider++;                                                     // from the rest of the string
        *endPtr = (char *)divider;
    }
    return number;
}
