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
int flag[256];


void printline(char r, int n, int ret) {
    int i = 0;
    for (i = 0; i < n; i++) fwritef(1, "%c", r);
    if (ret) fwritef(1, "\n");
}


void
walk(char *path, int level, int rec) {
    int fd, n;
    struct File f;
    struct File *dir;
    char new[MAXPATHLEN] = {0};
    if (rec == 0)return;
    if ((fd = open(path, O_RDONLY)) < 0)user_panic("open %s: %e", path, fd);
    while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
        if (f.f_name[0]) {
            dir = &f;
            printline(' ', level * 4, 0);
            fwritef(1, "|-- ");
            if (dir->f_type == FTYPE_REG)
                fwritef(1, "%s", dir->f_name);
            else
                fwritef(1, LIGHT_BLUE(%s), dir->f_name);

            fwritef(1, "\n");

            if (dir->f_type == FTYPE_DIR) {
                strcpy(new, path);
                strcat(new, "/");
                strcat(new, f.f_name);
                walk(new, level + 1, rec - 1);
            }
        }
    }
}
void
tree_start(char *path, u_int recursive) {
      	fwritef(1, GREEN(.)" "PURPLE(%s\n), path);
    walk(path, 0, recursive);
    return;
}

void
tree(char *path, char *prefix) {
    int r, fd;
    struct Stat st;
    if ((r = stat(path, &st)) < 0)
        user_panic("stat %s: %e", path, r);
    if (st.st_isdir && !flag['d']) {
        tree_start(path, -1);
    } else {
        tree_start(path, -1);
        //ls1(0, st.st_isdir, st.st_size, path);
    }
}


void
usage(void) {
    fwritef(1, "usage: tree [-dFl] [file...]\n");
    exit();
}

void
umain(int argc, char **argv) {
    int i;
    char curpath[MAXPATHLEN];
    ARGBEGIN
    {
        default:
            usage();
        case 'd':
        case 'F':
        case 'l':
            flag[(u_char) ARGC()]++;
        break;
    }
    ARGEND
    //syscall_curpath(curpath, 0);
    if (argc == 0) {
        //tree(curpath, "");
        tree("/", "");
    } else {
        for (i = 0; i < argc; i++)
            tree(argv[i], argv[i]);
    }

}
/*


#include "lib.h"

void walk(char *path, int level, int rec) {

    int fd, n;
    struct File f, *dir;
    char new[MAXPATHLEN] = {0};

    if(rec == 0) return;
    if ((fd = open(path, O_RDONLY)) < 0) user_panic("open %s: %e", path, fd);

    while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
        if (f.f_name[0]) {
            dir = &f;
            printline(' ', level * 4, 0);
            fwritef(1, "|-- ");

            if(dir->f_type == FTYPE_REG) fwritef(1, "%s\n", dir->f_name);
            else fwritef(1, "%s\n", dir->f_name);

            if(dir->f_type == FTYPE_DIR) {
                strcpy(new, path);
                strcat(new, "/");
                strcat(new, f.f_name);
                walk(new, level + 1, rec - 1);
            }
        }
    }
}

void printline(char r, int n, int ret) { 
    int i = 0;
    for(i = 0; i < n;i++) fwritef(1, "%c", r);
    if(ret) fwritef(1, "\n");
}

umain(int argc, char **argv)
{
	if(argc==0)walk("/",0,-1);
	else walk(argv[0],0,-1);
}
*/
