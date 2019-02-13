// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPWIDGET_HPP

#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/World.hpp>
#include <memory>

namespace bw
{
	class MapCanvas : public NazaraCanvas
	{
		public:
			MapCanvas(QWidget* parent = nullptr);
			~MapCanvas() = default;

			void ClearEntities();
			void ClearEntitySelection();

			Ndk::EntityId CreateEntity(const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			void EditEntityPosition(Ndk::EntityId entityId);

			void UpdateBackgroundColor(Nz::Color color);
			void UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			NazaraSignal(OnEntityPositionUpdated, MapCanvas* /*emitter*/, Ndk::EntityId /*entityId*/, const Nz::Vector2f& /*position*/);
			NazaraSignal(OnEntitySelected, MapCanvas* /*emitter*/, Ndk::EntityId /*entityId*/);

		private:
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);
			void OnUpdate(float elapsedTime) override;

			std::unique_ptr<PositionGizmo> m_positionGizmo;
			Ndk::EntityHandle m_cameraEntity;
			Ndk::EntityList m_mapEntities;
			Ndk::World m_world;
	};
}

#include <MapEditor/Widgets/MapCanvas.inl>

#endif