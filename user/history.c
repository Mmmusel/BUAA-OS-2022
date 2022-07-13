#include "lib.h"

/* Define color modes */
#define BOLD_GREEN(str) "\033[0;32;32m\033[1m" # str "\033[m"
#define RED(str) "\033[0;32;31m" # str "\033[m"
#define LIGHT_RED(str) "\033[1;31m" # str "\033[m"
#define GREEN(str) "\033[0;32;32m" # str "\033[m"
#define LIGHT_GREEN(str) "\033[1;32m" # str "\033[m"
#define BLUE(str) "\033[0;32;34m" # str "\033[m"
#define LIGHT_BLUE(str) "\033[1;34m" # str "\033[m"
#define DARK_GRAY(str) "\033[1;30m" # str "\033[m"
#define CYAN(str) "\033[0;36m" # str "\033[m"
#define LIGHT_CYAN(str) "\033[1;36m" # str "\033[m"
#define PURPLE(str) "\033[0;35m" # str "\033[m"
#define LIGHT_PURPLE(str) "\033[1;35m" # str "\033[m"
#define BROWN(str) "\033[0;33m" # str "\033[m"
#define YELLOW(str) "\033[1;33m" # str "\033[m"
#define LIGHT_GRAY(str) "\033[0;37m" # str "\033[m"
#define WHITE(str) "\033[1;37m" # str "\033[m"
char buf[8192];

void
history(int f, char *s) {
    long n;
    int r;

    while ((n = read(f, buf, (long) sizeof buf)) > 0)
        if ((r = write(1, buf, n)) != n)
            user_panic("write error copying %s: %e", s, r);
    if (n < 0)
        user_panic("error reading %s: %e", s, n);
}

void
usage(void) {
    fwritef(1, "usage: history\n");
    exit();
}

void
umain(int argc, char **argv) {
    int f, i;

    writef(LIGHT_CYAN(	[History Commands]	\n));
    f = open("/.history", O_RDONLY);
    if (f < 0)
        user_panic("can't open %s: %e", argv[i], f);
    else {
        history(f, argv[i]);
        close(f);
    }
}


