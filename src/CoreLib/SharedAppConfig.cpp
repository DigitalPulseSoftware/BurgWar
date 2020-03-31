// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedAppConfig.hpp>

namespace bw
{
	SharedAppConfig::SharedAppConfig(BurgApp& app) :
	ConfigFile(app)
	{
		RegisterStringOption("Assets.ResourceFolder");
		RegisterStringOption("Assets.ScriptFolder");
		RegisterBoolOption("Debug.SendServerState");
		RegisterFloatOption("GameSettings.TickRate");
	}
}
