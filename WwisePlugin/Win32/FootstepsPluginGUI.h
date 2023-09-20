#pragma once

#include "../FootstepsPlugin.h"

class FootstepsPluginGUI final
	: public AK::Wwise::Plugin::PluginMFCWindows<>
	, public AK::Wwise::Plugin::GUIWindows
{
public:
	FootstepsPluginGUI();

};
