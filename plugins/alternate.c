// 
// Plugin that returns string with alternate chars
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "sds.h"
#include "plugin_manager.h"

static sds alternate_hook(sds str) {
    int i,j;
    sds alternate = sdsnewlen(NULL,sdslen(str)/2);
    for(i=0,j=0; j < sdslen(str); i++, j=j+2) 
        alternate[i] = str[j];
    return alternate;
}


int init_alternate(PluginManager* pm) {
    PluginManager_register_strmanipulatehook(pm, alternate_hook);
    return 1;
}

