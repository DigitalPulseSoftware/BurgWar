// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/VisualLayer.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvasLayer.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <NDK/World.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class EditorGamemode;

	class MapCanvas : public SharedMatch, public WorldCanvas
	{
		friend MapCanvasLayer;

		public:
			MapCanvas(EditorWindow& editor, QWidget* parent = nullptr);
			~MapCanvas();

			void Clear();
			void ClearEntitySelection();

			const Ndk::EntityHandle& CreateEntity(LayerIndex layerIndex, EntityId uniqueId, const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, PropertyValueMap properties);
			void DeleteEntity(LayerIndex layerIndex, EntityId uniqueId);

			void EditEntitiesPosition(const std::vector<EntityId>& entityIds);

			void ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func) override;
			template<typename F> void ForEachEntity(F&& func);
			template<typename F> void ForEachMapEntity(F&& func);

			inline const std::shared_ptr<VirtualDirectory>& GetAssetDirectory();
			EditorEntityStore& GetEntityStore() override;
			const EditorEntityStore& GetEntityStore() const override;
			MapCanvasLayer& GetLayer(LayerIndex layerIndex) override;
			const MapCanvasLayer& GetLayer(LayerIndex layerIndex) const override;
			LayerIndex GetLayerCount() const override;
			const NetworkStringStore& GetNetworkStringStore() const override;
			inline ScriptingContext& GetScriptingContext();
			inline const ScriptingContext& GetScriptingContext() const;
			inline const std::shared_ptr<VirtualDirectory>& GetScriptDirectory();
			std::shared_ptr<const SharedGamemode> GetSharedGamemode() const override;
			ClientWeaponStore& GetWeaponStore() override;
			const ClientWeaponStore& GetWeaponStore() const override;

			void ReloadScripts();
			void ResetLayers(std::size_t layerCount);

			const Ndk::EntityHandle& RetrieveEntityByUniqueId(EntityId uniqueId) const override;
			EntityId RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const override;

			void ShowGrid(bool show);

			void UpdateActiveLayer(std::optional<LayerIndex> layerIndex);
			void UpdateEntityPositionAndRotation(EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			NazaraSignal(OnCameraZoomFactorUpdated, MapCanvas* /*emitter*/, float /*zoomFactor*/);
			NazaraSignal(OnCanvasMouseButtonPressed, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseButtonReleased, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseEntered, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseLeft, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseMoved, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseMoveEvent& /*mouseButton*/);
			NazaraSignal(OnDeleteEntities, MapCanvas* /*emitter*/, const EntityId* /*entityId*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesPositionUpdated, MapCanvas* /*emitter*/, const EntityId* /*entityId*/, std::size_t /*entityCount*/, const Nz::Vector2f& /*position*/);
			NazaraSignal(OnMultiSelectionStateUpdated, MapCanvas* /*emitter*/, bool /*newState*/);

		private:
			inline void RegisterEntity(EntityId uniqueId, LayerVisualEntityHandle handle);
			void OnKeyPressed(const Nz::WindowEvent::KeyEvent& key) override;
			void OnKeyReleased(const Nz::WindowEvent::KeyEvent& key) override;
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseEntered() override;
			void OnMouseLeft() override;
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;
			void OnTick(bool /*lastTick*/) override;
			inline void UnregisterEntity(EntityId uniqueId);
			void UpdateGrid();

			static std::vector<EntityId> BuildEntityIds(const std::vector<Ndk::EntityHandle>& entities);

			void resizeEvent(QResizeEvent* event) override;

			NazaraSlot(EditorWindow, OnLayerAlignmentUpdate, m_onLayerAlignmentUpdate);
			NazaraSlot(Ndk::Entity, OnEntityDestruction, m_onGizmoEntityDestroyed);

			std::optional<ClientAssetStore> m_assetStore;
			std::optional<ClientWeaponStore> m_weaponStore;
			std::optional<EditorEntityStore> m_entityStore;
			std::optional<LayerIndex> m_currentLayer;
			std::shared_ptr<EditorGamemode> m_gamemode;
			std::shared_ptr<ScriptingContext> m_scriptingContext;
			std::shared_ptr<VirtualDirectory> m_assetDirectory;
			std::shared_ptr<VirtualDirectory> m_scriptDirectory;
			tsl::hopscotch_map<EntityId, LayerVisualEntityHandle> m_entitiesByUniqueId;
			std::vector<MapCanvasLayer> m_layers;
			std::unique_ptr<EditorGizmo> m_entityGizmo;
			EditorWindow& m_editor;
			Ndk::EntityOwner m_currentLayerEntity;
			Ndk::EntityOwner m_gridEntity;
	};
}

#include <MapEditor/Widgets/MapCanvas.inl>

#endif
