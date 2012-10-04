//
// plugin_manager.h: Plugin manager interface. It's used both by plugins
// and the main application.
// The main application will create a PluginManager and pass a pointer to
// plugins, which use the PluginManager_register_* functions to register
// hooks on the PluginManager.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "sds.h"

//
// Types of hook callbacks registered by plugins
//

// String Manipulate hook. Will be called with: the input String
//
typedef sds (*PluginStrManipulateHook)(sds);

typedef struct PluginManager_t PluginManager;

// Create a new plugin manager.
//
PluginManager* PluginManager_new();

// Free the memory of a plugin manager.
//
void PluginManager_free(PluginManager* pm);


// Register a hook for input String.
//
void PluginManager_register_strmanipulatehook(PluginManager* pm,
                                          PluginStrManipulateHook hook);

// Apply the registered str manipulate hooks to the given input string, returning
// the transformed string.
// All registered hooks are composed:
//
//  while (has_plugins)
//      inputstr = apply_next_plugin(inputstr)
//
// If no str manipulate plugin exists, NULL is returned.
//
sds PluginManager_apply_strmanipulatehooks(PluginManager* pm, sds inputstr);

#endif /* PLUGIN_MANAGER_H */

