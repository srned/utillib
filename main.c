#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sds.h"

int main(int argc, char **argv) {
    sds arg = sdsnew(argv[0]);
    printf("program name:%s,#args:%d\n", arg, argc);
    sdsfree(arg);
}


