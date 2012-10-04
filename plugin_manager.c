#include <stddef.h>
#include "plugin_manager.h"
#include "zmalloc.h"


// The hooks are contained in simple singly-linked lists
//

typedef struct PluginStrManipulateHookList_t {
    PluginStrManipulateHook hook;
    struct PluginStrManipulateHookList_t* next;
} PluginStrManipulateHookList;


struct PluginManager_t {
    PluginStrManipulateHookList* strmanipulate_hook_list;
};


PluginManager* PluginManager_new() {
    PluginManager* pm = zmalloc(sizeof(*pm));
    pm->strmanipulate_hook_list = NULL;
    return pm;
}


void PluginManager_free(PluginManager* pm) {
    PluginStrManipulateHookList* strmanipulateplugin = pm->strmanipulate_hook_list;
    while (strmanipulateplugin) {
        PluginStrManipulateHookList* next = strmanipulateplugin->next;
        zfree(strmanipulateplugin);
        strmanipulateplugin = next;
    }
    zfree(pm);
}


void PluginManager_register_strmanipulatehook(PluginManager* pm,
                                          PluginStrManipulateHook hook) {
    PluginStrManipulateHookList* node = zmalloc(sizeof(*node));
    node->hook = hook;
    node->next = pm->strmanipulate_hook_list;
    pm->strmanipulate_hook_list = node;
}



sds PluginManager_apply_strmanipulatehooks(PluginManager* pm, sds strmanipulate) {
    PluginStrManipulateHookList* strmanipulate_plugin = pm->strmanipulate_hook_list;
    if (!strmanipulate_plugin)
        return NULL;

    strmanipulate = sdsdup(strmanipulate);
    while (strmanipulate_plugin) {
        sds new_strmanipulate = strmanipulate_plugin->hook(strmanipulate);
        sdsfree(strmanipulate);
        strmanipulate = new_strmanipulate;

        strmanipulate_plugin = strmanipulate_plugin->next;
    }

    return strmanipulate;
}

