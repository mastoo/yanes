#include "print_utils.h"
#include "syshead.h"

void print_hexdump(char *str, int len)
{
	int i;
    printf("Printing hexdump:\n");
    for ( i = 0; i < len; i ++) {
        if (i % 8 == 0) printf("\n");
        printf("%02x ", (unsigned char)str[i]);
    }

    printf("\n");
}

void print_error(char *str, ...)
{
    va_list ap;
    char buf[200];
    va_start(ap, str);
    vsnprintf(buf, 200, str, ap);

    va_end(ap);

    perror(buf);
}
