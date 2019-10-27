// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYSTORE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYSTORE_HPP

#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>

namespace bw
{
	class EditorEntityStore : public ClientEditorEntityStore
	{
		public:
			using ClientEditorEntityStore::ClientEditorEntityStore;
			~EditorEntityStore() = default;

		private:
			std::shared_ptr<ScriptedEntity> CreateElement() const override;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override;
	};
}

#include <MapEditor/Scripting/EditorEntityStore.inl>

#endif
