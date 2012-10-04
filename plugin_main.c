//
// plugin_main.c: The main program
//
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "zmalloc.h"
#include "sds.h"
#include "plugin_discovery.h"
#include "plugin_manager.h"

static sds apply_strmanipulate_hooks(sds inputstr,
        PluginManager* pm) {
    sds strmanipulated = PluginManager_apply_strmanipulatehooks(pm, inputstr);
    return strmanipulated ? strmanipulated : sdsdup(inputstr);
}


int main(int argc, const char* argv[]) {
    if (argc  != 2) {
        printf("Usage:./plugin_main ThisIsInputString\n");
        exit(1);
    }
    sds inputstr = sdsnew(argv[1]);

    // Perform plugin discovery in the "plugins" directory relative to the
    // working directory
    PluginManager* pm = PluginManager_new();
    sds dirname = sdsnew("plugins");
    void* pdstate = discover_plugins(dirname, pm);
    sdsfree(dirname);

    sds strmanipulate = apply_strmanipulate_hooks(inputstr, pm);

    printf("Given:%s,Manipulated:%s\n", inputstr, strmanipulate);

    sdsfree(inputstr);
    sdsfree(strmanipulate);
    PluginManager_free(pm);
    cleanup_plugins(pdstate);

    return 0;
}


