// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORGAMEMODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORGAMEMODE_HPP

#include <CoreLib/Scripting/SharedGamemode.hpp>

namespace bw
{
	class MapCanvas;

	class EditorGamemode : public SharedGamemode
	{
		public:
			EditorGamemode(MapCanvas& clientMatch, std::shared_ptr<ScriptingContext> scriptingContext, PropertyValueMap propertyValues);
			~EditorGamemode() = default;

		private:
			void InitializeGamemode(const std::string& gamemodeName) override;
	};
}

#include <MapEditor/Scripting/EditorGamemode.inl>

#endif
