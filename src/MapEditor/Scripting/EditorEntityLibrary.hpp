// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYLIBRARY_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYLIBRARY_HPP

#include <ClientLib/Scripting/ClientEntityLibrary.hpp>

namespace bw
{
	class AssetStore;
	class Logger;

	class EditorEntityLibrary : public ClientEntityLibrary
	{
		public:
			using ClientEntityLibrary::ClientEntityLibrary;
			~EditorEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			void InitRigidBody(const Ndk::EntityHandle& entity, float mass, float friction, bool canRotate) override;

		private:
			void RegisterEditorLibrary(sol::table& elementMetatable);
	};
}

#include <MapEditor/Scripting/EditorEntityLibrary.inl>

#endif
