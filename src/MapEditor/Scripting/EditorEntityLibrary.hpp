// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYLIBRARY_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYLIBRARY_HPP

#include <ClientLib/Scripting/ClientEntityLibrary.hpp>

namespace bw
{
	class AssetStore;
	class EditorWindow;
	class Logger;

	class EditorEntityLibrary : public ClientEntityLibrary
	{
		public:
			inline EditorEntityLibrary(EditorWindow& editorWindow, const Logger& logger, ClientAssetStore& assetStore);
			~EditorEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			void InitRigidBody(lua_State* L, entt::entity entity, float mass) override;

		private:
			void RegisterEditorLibrary(sol::table& elementMetatable);

			EditorWindow& m_editorWindow;
	};
}

#include <MapEditor/Scripting/EditorEntityLibrary.inl>

#endif
