// Copyright (C) 2020 Jérôme Leclercq
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

			const Ndk::EntityHandle& InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties, const Ndk::EntityHandle& parent = Ndk::EntityHandle::InvalidHandle) const override;

		private:
			void BindCallbacks(const ScriptedEntity& entityClass, const Ndk::EntityHandle& entity) const override;

			std::shared_ptr<ScriptedEntity> CreateElement() const override;
			void InitializeElement(sol::table& elementTable, ScriptedEntity& element) override;
	};
}

#include <MapEditor/Scripting/EditorEntityStore.inl>

#endif
