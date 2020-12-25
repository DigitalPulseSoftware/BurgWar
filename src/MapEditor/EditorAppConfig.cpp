// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/EditorAppConfig.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>

namespace bw
{
	EditorAppConfig::EditorAppConfig(EditorWindow& app) :
	SharedAppConfig(app)
	{
		RegisterStringOption("Resources.EditorDirectory");
		// Cache folders shouldn't be needed, but just in case...
		RegisterStringOption("Resources.AssetCacheDirectory", ".resourceCache");
		RegisterStringOption("Resources.ScriptCacheDirectory", ".scriptCache");
	}
}
