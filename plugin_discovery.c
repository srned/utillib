// The following is the Eli Bendersky's implementation of Plugins infrastructure modified 
// in order to comply with our needs
//
// The original copyright notice follows. */
// plugin_discovery.c: Plugin discovery implementation.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//
#include "plugin_discovery.h"
#include "zmalloc.h"

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


// PluginDiscoveryState manages the state of plugin discovery. Holds a list
// of handles returned by dlopen() on the plugin DSOs.
// An opaque pointer to this object can be held by the caller.
//
typedef struct PluginHandleList_t {
    void* handle;
    struct PluginHandleList_t* next;
} PluginHandleList;

typedef struct PluginDiscoveryState_t {
    PluginHandleList* handle_list;
} PluginDiscoveryState;


// This is the signature of a plugin initialization function that the plugins
// must export as init_<pluginname>. This function accepts a pointer to a plugin
// manager. Its return value means an error if < 0, success otherwise.
//
typedef int (*PluginInitFunc)(PluginManager*);


// Given a filename, return the name of the plugin (the filename
// without .so extension) if it appears like a valid plugin path, or
// NULL.
//
static sds get_plugin_name(char* filename) {
    // Find where the file name starts, and where it ends before
    // the extension starts
    char* last_slash = strrchr(filename, '/');
    char* name_start = last_slash ? last_slash + 1 : filename;
    char* last_dot = strrchr(filename, '.');

    // We only care about file names that have a .so extension
    if (!last_dot || strcmp(last_dot, ".so"))
        return NULL;

    sds ret = sdsrange(sdsnew(filename), name_start - filename, last_dot - filename - 1);
    printf("returning:%s\n", ret);

    return ret;
}


// Attempt to load the plugin specified by a name and a full path, by
// dynamically loading the DSO and calling its initialization function.
// If everything goes well, the plugin initialization function will be
// called and passed the plugin manager. In this case, the DSO handle
// (the result of dlopen) will be returned.
// If the loading fails for some reason or the DSO is not a valid plugin
// (e.g. has no init function, or the init function returned an error code),
// NULL is returned.
//
static void* load_plugin(sds name, sds fullpath, PluginManager* pm) {
    // Make sure the path to dlopen has a slash, for it to consider it 
    // an actual filesystem path and not just a lookup name.
    sds slashedpath = sdscatprintf(sdsempty(),"./%s", fullpath);

    // Attempt to open the plugin DSO
    void* libhandle = dlopen(slashedpath, RTLD_NOW);
    sdsfree(slashedpath);
    if (!libhandle) {
        printf("Error loading DSO: %s\n", dlerror());
        return NULL;
    }

    // Attempt to find the init function and then call it
    sds initfunc_name = sdscatprintf(sdsempty(),"init_%s", name);
    // dlsym returns void*, but we obviously need to cast it to a function
    // pointer to be able to call it. Since void* and function pointers are
    // mutually inconvertible in the eyes of C99, and -pedantic complains about
    // a plain cast, we cast through a pointer-sized integer.
    PluginInitFunc initfunc = (PluginInitFunc)
        (intptr_t) dlsym(libhandle, initfunc_name);
    sdsfree(initfunc_name);
    if (!initfunc) {
        printf("Error loading init function: %s\n", dlerror());
        dlclose(libhandle);
        return NULL;
    }

    int rc = initfunc(pm);
    if (rc < 0) {
        printf("Error: Plugin init function returned %d\n", rc);
        dlclose(libhandle);
        return NULL;
    }

    printf("Loaded plugin from: '%s'\n", fullpath);
    return libhandle;
}


void* discover_plugins(sds dirname, PluginManager* pm) {
    //const char* dirname_s = dirname;
    DIR* dir = opendir(dirname);
    if (!dir) {
        sds err = sdscatprintf(sdsempty(), "Unable to open path '%s'", dirname);
        perror(err);
        sdsfree(err);
        return NULL;
    }

    PluginDiscoveryState* plugins_state = zmalloc(sizeof(*plugins_state));
    plugins_state->handle_list = NULL;

    // Look at all DSOs in the plugin directory and attempt to load them.
    struct dirent* direntry;
    while ((direntry = readdir(dir))) {
        sds name = get_plugin_name(direntry->d_name);
        if (!name)
            continue;
        sds fullpath = sdscatprintf(sdsempty(),"%s/%s",
                dirname, direntry->d_name);
        // Load the plugin, get the DSO handle and add it to the list
        void* handle = load_plugin(name, fullpath, pm);
        if (handle) {
            PluginHandleList* handle_node = zmalloc(sizeof(*handle_node));
            handle_node->handle = handle;
            handle_node->next = plugins_state->handle_list;
            plugins_state->handle_list = handle_node;
        }

        sdsfree(name);
        sdsfree(fullpath);
    }

    closedir(dir);
    // Return a state if plugins were found.
    if (plugins_state->handle_list)
        return (void*)plugins_state;
    else {
        zfree(plugins_state);
        return NULL;
    }
}


void cleanup_plugins(void* vpds) {
    PluginDiscoveryState* pds = (PluginDiscoveryState*)vpds;
    if (!pds) 
        return;
    PluginHandleList* node = pds->handle_list;
    while (node) {
        PluginHandleList* next = node->next;
        dlclose(node->handle);
        zfree(node);
        node = next;
    }
    zfree(pds);
}


