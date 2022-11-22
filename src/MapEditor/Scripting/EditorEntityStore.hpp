// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYSTORE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITORENTITYSTORE_HPP

#include <ClientLib/LayerVisualEntity.hpp>
#include <ClientLib/Scripting/ClientEditorEntityStore.hpp>

namespace bw
{
	class MapCanvas;

	class EditorEntityStore : public ClientEditorEntityStore
	{
		public:
			inline EditorEntityStore(MapCanvas& mapCanvas, ClientAssetStore& assetStore, const Logger& logger, std::shared_ptr<ScriptingContext> context);
			~EditorEntityStore() = default;

			std::optional<LayerVisualEntity> Instantiate(LayerIndex layerIndex, entt::registry& world, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, float scale, PropertyValueMap properties, entt::entity parent = entt::null) const;

		private:
			void BindCallbacks(const ScriptedEntity& entityClass, entt::entity entity) const override;

			std::shared_ptr<ScriptedEntity> CreateElement() const override;
			void InitializeElement(sol::main_table& elementTable, ScriptedEntity& element) override;

			MapCanvas& m_mapCanvas;
	};
}

#include <MapEditor/Scripting/EditorEntityStore.inl>

#endif
