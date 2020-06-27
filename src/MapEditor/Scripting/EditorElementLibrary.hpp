// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORELEMENTLIBRARY_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORELEMENTLIBRARY_HPP

#include <ClientLib/Scripting/ClientElementLibrary.hpp>

namespace bw
{
	class AssetStore;
	class Logger;

	class EditorElementLibrary : public ClientElementLibrary
	{
		public:
			using ClientElementLibrary::ClientElementLibrary;
			~EditorElementLibrary() = default;

		protected:
			void RegisterClientLibrary(sol::table& elementTable) override;
	};
}

#include <MapEditor/Scripting/EditorElementLibrary.inl>

#endif
