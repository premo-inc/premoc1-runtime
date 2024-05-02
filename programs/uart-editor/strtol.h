#include <limits.h>
#define NUL '\0'

#define BOOL char
#define TRUE 1
#define FALSE 0

long strtol(const char *nPtr, char **endPtr, int base)
{
    // checking if the base value is correct
    if ((base < 2 || base > 36) && base != 0)
    {
        return 0;
    }

    long number = 0;
    const char *divider;
    int currentdigit,
        sign,
        cutlim;
    enum sign
    {
        NEGATIVE,
        POSITIVE
    };
    unsigned long cutoff;
    BOOL correctconversion = TRUE;

    divider = nPtr;

    // looking for a space if the beggining of the string is moved further
    while ((*divider) == ' ' || (*divider) == '\r' || (*divider) == '\n')
        divider++;

    // detecting the sign, positive by default
    if (*divider == '+')
    {
        sign = POSITIVE;
        divider++;
    }
    else if (*divider == '-')
    {
        sign = NEGATIVE;
        divider++;
    }
    else
        sign = POSITIVE;

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

    // two conditions just for clarity --> |LONG_MIN| = LONG_MAX + 1
    if (sign)
        cutoff = LONG_MAX / (unsigned long)base;
    else
        cutoff = (unsigned long)LONG_MIN / (unsigned long)base;
    cutlim = cutoff % (unsigned long)base;

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
        if (!correctconversion ||
            number > cutoff ||
            (number == cutoff && (int)currentdigit > cutlim))
        {
            correctconversion = FALSE;
            divider++;
        }
        else
        { // the actual conversion to decimal
            correctconversion = TRUE;
            number = (number * base) + currentdigit;
            divider++;
        }
    }
    if (!correctconversion)
    {
        if (sign)
            number = LONG_MAX;
        else
            number = LONG_MIN;
    }
    if (sign == NEGATIVE)
        number *= -1;
    if (endPtr != NUL)
    {
        if ((*divider) == ' ' || (*divider) == '\r' || (*divider) == '\n') // checking if the number is separated
            divider++;                                                     // from the rest of the string
        *endPtr = (char *)divider;
    }
    return number;
}
