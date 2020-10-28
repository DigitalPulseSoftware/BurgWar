// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_GIZMOS_POSITION_HPP
#define BURGWAR_MAPEDITOR_GIZMOS_POSITION_HPP

#include <MapEditor/Gizmos/EditorGizmo.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <array>
#include <vector>

namespace bw
{
	class Camera;

	class PositionGizmo : public EditorGizmo
	{
		public:
			PositionGizmo(Camera& camera, std::vector<Ndk::EntityHandle> entities, const Nz::Vector2f& positionAlignment);
			~PositionGizmo() = default;

			bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;

			inline void UpdatePositionAlignment(const Nz::Vector2f& positionAlignment);

			NazaraSignal(OnPositionUpdated, PositionGizmo* /*emitter*/, Nz::Vector2f /*offset*/);

		private:
			Nz::Vector2f ComputeNewPosition(int mouseX, int mouseY) const;

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
			std::vector<Nz::Vector2f> m_entitiesOffsets;
			Camera& m_camera;
			MovementType m_hoveredAction;
			MovementType m_movementType;
			Ndk::EntityOwner m_arrowEntity;
			Nz::Vector2f m_movementStartPos;
			Nz::Vector2f m_originalPosition;
			Nz::Vector2f m_positionAlignment;
	};
}

#include <MapEditor/Gizmos/PositionGizmo.inl>

#endif
