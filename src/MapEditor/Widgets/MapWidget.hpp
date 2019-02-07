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
	class MapWidget : public NazaraCanvas
	{
		public:
			MapWidget(QWidget* parent = nullptr);
			~MapWidget() = default;

			void ClearEntities();
			Ndk::EntityId CreateEntity(const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

			void EditEntityPosition(Ndk::EntityId entityId);

			void UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation);

		private:
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);
			void OnUpdate(float elapsedTime) override;

			std::unique_ptr<PositionGizmo> m_positionGizmo;
			Ndk::EntityHandle m_camera;
			Ndk::EntityList m_mapEntities;
			Ndk::World m_world;
	};
}

#include <MapEditor/Widgets/MapWidget.inl>

#endif