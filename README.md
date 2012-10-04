# utillib

This is a C library that consists of collection of generic tools that are used in systems dev efforts. This inturn uses baselib. 

# 1.Plugin InfraStructure
In this effort, we implement a template plugin infrastructure that can be leveraged to develop any realistic extensions. 
It consists of an application that takes a string as argument and the string is passed around to the plugins that are present in plugins directory. "plugin_main" target builds the application that uses plugins. A sample plugin that returns alternate characters from the input string is implemented in plugins/alternate.c

## To Build and Run
sh#make
sh$./plugin_main ReturnAlternaTeString
returning:alternate
Loaded plugin from: 'plugins/alternate.so'
Given:ReturnAlternaTeString,Manipulated:RtrAtraetig

## Related Files
plugin_main.c
plugin_discovery.c
plugin_manager.c
plugins/*

## Credits
http://eli.thegreenplace.net/2012/08/24/plugins-in-c/



