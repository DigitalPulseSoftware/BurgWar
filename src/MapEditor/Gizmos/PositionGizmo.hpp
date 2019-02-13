// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_GIZMOS_POSITION_HPP
#define BURGWAR_MAPEDITOR_GIZMOS_POSITION_HPP

#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class PositionGizmo
	{
		friend class MapCanvas;

		public:
			PositionGizmo(Ndk::Entity* camera, Ndk::Entity* entity);
			~PositionGizmo() = default;

			NazaraSignal(OnPositionUpdated, PositionGizmo* /*emitter*/, Nz::Vector2f /*newPosition*/);

		private:
			bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);

			enum MovementType
			{
				None = 0xFF,
				
				XYAxis = 0,
				XAxis = 1,
				YAxis = 2
			};

			std::array<Nz::Color, 3> m_spriteDefaultColors;
			std::array<Nz::SpriteRef, 3> m_sprites;
			std::array<Nz::Vector2f, 3> m_allowedMovements;
			MovementType m_hoveredAction;
			MovementType m_movementType;
			Ndk::EntityHandle m_cameraEntity;
			Ndk::EntityHandle m_movedEntity;
			Ndk::EntityOwner m_arrowEntity;
			Nz::Vector2f m_originalPosition;
			Nz::Vector2f m_movementStartPos;
	};
}

#include <MapEditor/Widgets/MapCanvas.inl>

#endif