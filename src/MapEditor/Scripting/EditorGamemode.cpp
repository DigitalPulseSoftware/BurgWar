// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorGamemode.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <cassert>

namespace bw
{
	EditorGamemode::EditorGamemode(MapCanvas& mapCanvas, std::shared_ptr<ScriptingContext> scriptingContext, PropertyValueMap propertyValues) :
	SharedGamemode(mapCanvas, std::move(scriptingContext), "editor", std::move(propertyValues))
	{
		Reload();
	}

	void EditorGamemode::InitializeGamemode(const std::string& gamemodeName)
	{
		const std::filesystem::path gamemodeFolder = "gamemodes";

		std::filesystem::path gamemodePath = gamemodeFolder / gamemodeName;

		auto& context = GetScriptingContext();
		context->Load(gamemodePath / Nz::Utf8Path("shared.lua"));
		context->Load(gamemodePath / Nz::Utf8Path("cl_init.lua"));
	}
}
