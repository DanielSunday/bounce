#include <stdio.h>

#include "level.h"

static void flush_stdin(void);

int main()
{
    int level;

    puts("Enter a negative integer to exit.");
    for (;;)
    {
        printf("Enter level: ");
        if (scanf("%d", &level) != 1)
        {
            puts("Please try again.");
            flush_stdin();
        }
        else if (level < 0)
        {
            puts("Goodbye!");
            break;
        }
        else
        {
            run_level(level);
            flush_stdin();
        }
    }

    return 0;
}

static void flush_stdin(void)
{
    while (getchar() != '\n');
}
