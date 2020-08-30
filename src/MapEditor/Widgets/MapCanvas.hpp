// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP

#include <CoreLib/PropertyValues.hpp>
#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <NDK/World.hpp>
#include <memory>

namespace bw
{
	class MapCanvas : public WorldCanvas
	{
		public:
			MapCanvas(EditorWindow& editor, QWidget* parent = nullptr);
			~MapCanvas() = default;

			void ClearEntities();
			void ClearEntitySelection();

			const Ndk::EntityHandle& CreateEntity(const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties);
			void DeleteEntity(Ndk::EntityId entityId);

			template<typename F> void ForEachEntity(F&& func);

			inline const Ndk::EntityList& GetMapEntities() const;

			void EditEntityPosition(Ndk::EntityId entityId);

			void ShowGrid(bool show);

			void UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			NazaraSignal(OnCameraZoomFactorUpdated, MapCanvas* /*emitter*/, float /*zoomFactor*/);
			NazaraSignal(OnCanvasMouseButtonPressed, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseButtonReleased, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/);
			NazaraSignal(OnCanvasMouseEntered, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseLeft, MapCanvas* /*emitter*/);
			NazaraSignal(OnCanvasMouseMoved, MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseMoveEvent& /*mouseButton*/);
			NazaraSignal(OnDeleteEntity, MapCanvas* /*emitter*/, Ndk::EntityId /*entityId*/);
			NazaraSignal(OnEntityPositionUpdated, MapCanvas* /*emitter*/, Ndk::EntityId /*entityId*/, const Nz::Vector2f& /*position*/);

		private:
			void OnKeyPressed(const Nz::WindowEvent::KeyEvent& key) override;
			void OnKeyReleased(const Nz::WindowEvent::KeyEvent& key) override;
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseEntered() override;
			void OnMouseLeft() override;
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;
			void UpdateGrid();

			void resizeEvent(QResizeEvent* event) override;

			NazaraSlot(EditorWindow, OnLayerAlignmentUpdate, m_onLayerAlignmentUpdate);
			NazaraSlot(Ndk::Entity, OnEntityDestruction, m_onGizmoEntityDestroyed);

			std::unique_ptr<EditorGizmo> m_entityGizmo;
			EditorWindow& m_editor;
			Ndk::EntityOwner m_gridEntity;
			Ndk::EntityList m_mapEntities;
	};
}

#include <MapEditor/Widgets/MapCanvas.inl>

#endif
