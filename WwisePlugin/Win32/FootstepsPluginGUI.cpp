
#include "FootstepsPluginGUI.h"

FootstepsPluginGUI::FootstepsPluginGUI()
{
}

ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
    Footsteps,            // Name of the plug-in container for this shared library
    FootstepsPluginGUI,   // Authoring plug-in class to add to the plug-in container
    FootstepsSource       // Corresponding Sound Engine plug-in class
);
