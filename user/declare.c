#include "lib.h"

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


void
set(char *name, char *value) {
    int r;

    if ((r = syscall_env_var(name, value, 2)) < 0) {
        if (r == -13){ syscall_env_var(name, value, 6);writef("\033[0;36mdeclare global:\033[m %s = %s\n",name,value);}
        else if (r == -14) fwritef(1, "set: [%s] is readonly\n", name);
        return;
    }
    writef("\033[0;36mdeclare global:\033[m %s = %s\n",name,value);
}

void
set_tmp(char *name, char *value) {
    int r;

    if ((r = syscall_env_var(name, value, 2)) < 0) {
        if (r == -13){ syscall_env_var(name, value, 7);writef("\033[0;36mdeclare local:\033[m %s = %s\n",name,value);}
        else if (r == -14) fwritef(1, "declare: [%s] is readonly\n", name);
        return;
    }
    writef("\033[0;36mdeclare local:\033[m %s = %s\n",name,value);
}

void
set_readonly(char *name, char *value) {
    int r;

    if ((r = syscall_env_var(name, value, 2)) < 0) {
        if (r == -13){ syscall_env_var(name, value, 5);writef("\033[0;36mdeclare read-only:\033[m %s = %s\n",name,value);}
        else if (r == -14) fwritef(1, "set: [%s] is readonly\n", name);
        return;
    }
    writef("\033[0;36mdeclare read-only:\033[m %s = %s\n",name,value);
}

void
usage(void) {
    fwritef(1, "usage: set [var] [value]\n");
    exit();
}


char buf[8192];

void
export() {
    char *name_table[8196];
    char *value_table[8196];
    syscall_env_var(name_table, value_table, 4);
    int i = 0,j=0;
    for (i=0,j=0;name_table[i];i+=64,j+=256)
    {      
	writef("export in i %d\n",i);
        writef( "%s = %s\n", (name_table+i), (value_table+j));
    }
}

void
declare_search(char *name) {
    char value[256];
    syscall_env_var(name, value, 1);
    fwritef(1, GREEN(%s) " = %s\n", name, value);
}


void
umain(int argc, char **argv) {
    int i;
    ARGBEGIN
    {
        default:
            usage();
	case 'x':
        case 'r':
            flag[(u_char) ARGC()]++;
        break;
    }
    ARGEND

    if (argc == 0) export();
    else if (flag['x']) {
        if (argc == 1) set(argv[0], "");
        else if (argc == 2) set(argv[0], argv[1]);
    } else if (flag['r']){
        if (argc == 1) set_readonly(argv[0], "");
        else if (argc == 2) set_readonly(argv[0], argv[1]);
    } else {
    	if (argc == 1) declare_search(argv[0]);
        else if (argc == 2) set_tmp(argv[0], argv[1]);
	}
}

