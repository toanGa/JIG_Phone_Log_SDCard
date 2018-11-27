/*******************************************************************************
** @file:		uart_console.c
** @author:		HUY DAI
** @version:	V1.0.0
** @date:		11:00 AM Wednesday, 31 January, 2018
** @brief:		debug with uart
** @revision:
**             	- version 1.0.1: <date> <name>
**             	<discribe the change>
**             	- version 1.0.1: <date> <name>
**             	<discribe the change>
*******************************************************************************/

/******************************************************************************
**                               INCLUDES
*******************************************************************************/
#include "uart_console.h"
#include "string.h"
#include "stdarg.h"

/*******************************************************************************
**                       INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define HUART_CONSOLE     huart1

/*******************************************************************************
**                       VARIABLE
*******************************************************************************/
static const char * const g_pcHex = "0123456789abcdef";
#warning config uart to handler


extern UART_HandleTypeDef HUART_CONSOLE;

/*******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
void UARTwrite( char *pcBuf, unsigned int len)
{
	HAL_UART_Transmit(&HUART_CONSOLE,(uint8_t*) pcBuf, len, 100);
}


/*******************************************************************************
** @brief      debug
** @param      None
** @return     None
** @time       11:03 AM Wednesday, 31 January, 2018
** @revision
*******************************************************************************/
/**
 * A simple UART based printf function supporting \%c, \%d, \%p, \%s, \%u,
 * \%x, and \%X.
 *
 * \param pcString is the format string.
 * \param ... are the optional arguments, which depend on the contents of the
 * format string.
 *
 * This function is very similar to the C library <tt>fprintf()</tt> function.
 * All of its output will be sent to the UART.  Only the following formatting
 * characters are supported:
 *
 * - \%c to print a character
 * - \%d to print a decimal value
 * - \%s to print a string
 * - \%u to print an unsigned decimal value
 * - \%x to print a hexadecimal value using lower case letters
 * - \%X to print a hexadecimal value using lower case letters (not upper case
 * letters as would typically be used)
 * - \%p to print a pointer as a hexadecimal value
 * - \%\% to print out a \% character
 *
 * For \%s, \%d, \%u, \%p, \%x, and \%X, an optional number may reside
 * between the \% and the format character, which specifies the minimum number
 * of characters to use for that value; if preceded by a 0 then the extra
 * characters will be filled with zeros instead of spaces.  For example,
 * ``\%8d'' will use eight characters to print the decimal value with spaces
 * added to reach eight; ``\%08d'' will use eight characters as well but will
 * add zeroes instead of spaces.
 *
 * The type of the arguments after \e pcString must match the requirements of
 * the format string.  For example, if an integer was passed where a string
 * was expected, an error of some kind will most likely occur.
 *
 * \return None.
 */
void UARTprintf( char *pcString, ...)
{
	
    unsigned int idx, pos, count, base, neg;
    char *pcStr, pcBuf[16], cFill;
    va_list vaArgP;
    int value;

    /* Start the varargs processing. */
    va_start(vaArgP, pcString);

    /* Loop while there are more characters in the string. */
    while(*pcString)
    {
        /* Find the first non-% character, or the end of the string. */
        for(idx = 0; (pcString[idx] != '%') && (pcString[idx] != '\0'); idx++)
        {
        }

        /* Write this portion of the string. */
        UARTwrite(pcString, idx);

        /* Skip the portion of the string that was written. */
        pcString += idx;

        /* See if the next character is a %. */
        if(*pcString == '%')
        {
            /* Skip the %. */
            pcString++;

            /* Set the digit count to zero, and the fill character to space
             * (i.e. to the defaults). */
            count = 0;
            cFill = ' ';

            /* It may be necessary to get back here to process more characters.
             * Goto's aren't pretty, but effective.  I feel extremely dirty for
             * using not one but two of the beasts. */
again:

            /* Determine how to handle the next character. */
            switch(*pcString++)
            {
                /* Handle the digit characters. */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    /* If this is a zero, and it is the first digit, then the
                     * fill character is a zero instead of a space. */
                    if((pcString[-1] == '0') && (count == 0))
                    {
                        cFill = '0';
                    }

                    /* Update the digit count. */
                    count *= 10;
                    count += pcString[-1] - '0';

                    /* Get the next character. */
                    goto again;
                }

                /* Handle the %c command. */
                case 'c':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Print out the character. */
                    UARTwrite((char *)&value, 1);

                    /* This command has been handled. */
                    break;
                }

                /* Handle the %d command. */
                case 'd':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* If the value is negative, make it positive and indicate
                     * that a minus sign is needed. */
                    if((int)value < 0)
                    {
                        /* Make the value positive. */
                        value = -(int)value;

                        /* Indicate that the value is negative. */
                        neg = 1;
                    }
                    else
                    {
                        /* Indicate that the value is positive so that a minus
                         * sign isn't inserted. */
                        neg = 0;
                    }

                    /* Set the base to 10. */
                    base = 10;

                    /* Convert the value to ASCII. */
                    goto convert;
                }

                /* Handle the %s command. */
                case 's':
                {
                    /* Get the string pointer from the varargs. */
                    pcStr = va_arg(vaArgP, char *);

                    /* Determine the length of the string. */
                    for(idx = 0; pcStr[idx] != '\0'; idx++)
                    {
                    }

                    /* Write the string. */
                    UARTwrite(pcStr, idx);

                    /* Write any required padding spaces */
                    if(count > idx)
                    {
                        count -= idx;
                        while(count--)
                        {
                            UARTwrite(( char *)" ", 1);
                        }
                    }
                    /* This command has been handled. */
                    break;
                }

                /* Handle the %u command. */
                case 'u':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* Set the base to 10. */
                    base = 10;

                    /* Indicate that the value is positive so that a minus sign
                     * isn't inserted. */
                    neg = 0;

                    /* Convert the value to ASCII. */
                    goto convert;
                }

                /* Handle the %x and %X commands.  Note that they are treated
                 * identically; i.e. %X will use lower case letters for a-f
                 * instead of the upper case letters is should use.  We also
                 * alias %p to %x. */
                case 'x':
                case 'X':
                case 'p':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* Set the base to 16. */
                    base = 16;

                    /* Indicate that the value is positive so that a minus sign
                     * isn't inserted. */
                    neg = 0;

                    /* Determine the number of digits in the string version of
                     * the value. */
convert:
                    for(idx = 1;
                        (((idx * base) <= value) &&
                         (((idx * base) / base) == idx));
                        idx *= base, count--)
                    {
                    }

                    /* If the value is negative, reduce the count of padding
                     * characters needed. */
                    if(neg)
                    {
                        count--;
                    }

                    /* If the value is negative and the value is padded with
                     * zeros, then place the minus sign before the padding. */
                    if(neg && (cFill == '0'))
                    {
                        /* Place the minus sign in the output buffer. */
                        pcBuf[pos++] = '-';

                        /* The minus sign has been placed, so turn off the
                         * negative flag. */
                        neg = 0;
                    }

                    /* Provide additional padding at the beginning of the
                     * string conversion if needed. */
                    if((count > 1) && (count < 16))
                    {
                        for(count--; count; count--)
                        {
                            pcBuf[pos++] = cFill;
                        }
                    }

                    /* If the value is negative, then place the minus sign
                     * before the number. */
                    if(neg)
                    {
                        /* Place the minus sign in the output buffer. */
                        pcBuf[pos++] = '-';
                    }

                    /* Convert the value into a string. */
                    for(; idx; idx /= base)
                    {
                        pcBuf[pos++] = g_pcHex[(value / idx) % base];
                    }

                    /* Write the string. */
                     UARTwrite(pcBuf, pos);

                    /* This command has been handled. */
                    break;
                }

                /* Handle the %% command. */
                case '%':
                {
                    /* Simply write a single %. */
                     UARTwrite(pcString - 1, 1);

                    /* This command has been handled. */
                    break;
                }

                /* Handle all other commands. */
                default:
                {
                    /* Indicate an error. */
                     UARTwrite(( char *)"ERROR", 5);

                    /* This command has been handled. */
                    break;
                }
            }
        }
    }

    /* End the varargs processing. */
    va_end(vaArgP); 
}

#if 0
void debug_printf( char *pcString, ...)
{
	
    unsigned int idx, pos, count, base, neg;
    char *pcStr, pcBuf[16], cFill;
    va_list vaArgP;
    int value;

    /* Start the varargs processing. */
    va_start(vaArgP, pcString);

    /* Loop while there are more characters in the string. */
    while(*pcString)
    {
        /* Find the first non-% character, or the end of the string. */
        for(idx = 0; (pcString[idx] != '%') && (pcString[idx] != '\0'); idx++)
        {
        }

        /* Write this portion of the string. */
        UARTwrite(pcString, idx);

        /* Skip the portion of the string that was written. */
        pcString += idx;

        /* See if the next character is a %. */
        if(*pcString == '%')
        {
            /* Skip the %. */
            pcString++;

            /* Set the digit count to zero, and the fill character to space
             * (i.e. to the defaults). */
            count = 0;
            cFill = ' ';

            /* It may be necessary to get back here to process more characters.
             * Goto's aren't pretty, but effective.  I feel extremely dirty for
             * using not one but two of the beasts. */
again:

            /* Determine how to handle the next character. */
            switch(*pcString++)
            {
                /* Handle the digit characters. */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    /* If this is a zero, and it is the first digit, then the
                     * fill character is a zero instead of a space. */
                    if((pcString[-1] == '0') && (count == 0))
                    {
                        cFill = '0';
                    }

                    /* Update the digit count. */
                    count *= 10;
                    count += pcString[-1] - '0';

                    /* Get the next character. */
                    goto again;
                }

                /* Handle the %c command. */
                case 'c':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Print out the character. */
                    UARTwrite((char *)&value, 1);

                    /* This command has been handled. */
                    break;
                }

                /* Handle the %d command. */
                case 'd':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* If the value is negative, make it positive and indicate
                     * that a minus sign is needed. */
                    if((int)value < 0)
                    {
                        /* Make the value positive. */
                        value = -(int)value;

                        /* Indicate that the value is negative. */
                        neg = 1;
                    }
                    else
                    {
                        /* Indicate that the value is positive so that a minus
                         * sign isn't inserted. */
                        neg = 0;
                    }

                    /* Set the base to 10. */
                    base = 10;

                    /* Convert the value to ASCII. */
                    goto convert;
                }

                /* Handle the %s command. */
                case 's':
                {
                    /* Get the string pointer from the varargs. */
                    pcStr = va_arg(vaArgP, char *);

                    /* Determine the length of the string. */
                    for(idx = 0; pcStr[idx] != '\0'; idx++)
                    {
                    }

                    /* Write the string. */
                    UARTwrite(pcStr, idx);

                    /* Write any required padding spaces */
                    if(count > idx)
                    {
                        count -= idx;
                        while(count--)
                        {
                            UARTwrite(( char *)" ", 1);
                        }
                    }
                    /* This command has been handled. */
                    break;
                }

                /* Handle the %u command. */
                case 'u':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* Set the base to 10. */
                    base = 10;

                    /* Indicate that the value is positive so that a minus sign
                     * isn't inserted. */
                    neg = 0;

                    /* Convert the value to ASCII. */
                    goto convert;
                }

                /* Handle the %x and %X commands.  Note that they are treated
                 * identically; i.e. %X will use lower case letters for a-f
                 * instead of the upper case letters is should use.  We also
                 * alias %p to %x. */
                case 'x':
                case 'X':
                case 'p':
                {
                    /* Get the value from the varargs. */
                    value = va_arg(vaArgP, unsigned int);

                    /* Reset the buffer position. */
                    pos = 0;

                    /* Set the base to 16. */
                    base = 16;

                    /* Indicate that the value is positive so that a minus sign
                     * isn't inserted. */
                    neg = 0;

                    /* Determine the number of digits in the string version of
                     * the value. */
convert:
                    for(idx = 1;
                        (((idx * base) <= value) &&
                         (((idx * base) / base) == idx));
                        idx *= base, count--)
                    {
                    }

                    /* If the value is negative, reduce the count of padding
                     * characters needed. */
                    if(neg)
                    {
                        count--;
                    }

                    /* If the value is negative and the value is padded with
                     * zeros, then place the minus sign before the padding. */
                    if(neg && (cFill == '0'))
                    {
                        /* Place the minus sign in the output buffer. */
                        pcBuf[pos++] = '-';

                        /* The minus sign has been placed, so turn off the
                         * negative flag. */
                        neg = 0;
                    }

                    /* Provide additional padding at the beginning of the
                     * string conversion if needed. */
                    if((count > 1) && (count < 16))
                    {
                        for(count--; count; count--)
                        {
                            pcBuf[pos++] = cFill;
                        }
                    }

                    /* If the value is negative, then place the minus sign
                     * before the number. */
                    if(neg)
                    {
                        /* Place the minus sign in the output buffer. */
                        pcBuf[pos++] = '-';
                    }

                    /* Convert the value into a string. */
                    for(; idx; idx /= base)
                    {
                        pcBuf[pos++] = g_pcHex[(value / idx) % base];
                    }

                    /* Write the string. */
                     UARTwrite(pcBuf, pos);

                    /* This command has been handled. */
                    break;
                }

                /* Handle the %% command. */
                case '%':
                {
                    /* Simply write a single %. */
                     UARTwrite(pcString - 1, 1);

                    /* This command has been handled. */
                    break;
                }

                /* Handle all other commands. */
                default:
                {
                    /* Indicate an error. */
                     UARTwrite(( char *)"ERROR", 5);

                    /* This command has been handled. */
                    break;
                }
            }
        }
    }

    /* End the varargs processing. */
    va_end(vaArgP); 
}
#endif

