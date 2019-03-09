// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP

#include <MapEditor/Logic/EntityEditorMode.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <NDK/EntityOwner.hpp>
#include <optional>

namespace bw
{
	class TileMapEditorMode : public EntityEditorMode
	{
		public:
			TileMapEditorMode(const Ndk::EntityHandle& targetEntity, const Nz::Vector2f& origin, const Nz::DegreeAnglef& rotation, 
			                  const Nz::Vector2ui& mapSize, const Nz::Vector2f& tileSize, std::vector<Nz::UInt8> content);
			~TileMapEditorMode() = default;

			void OnEnter(EditorWindow& editor) override;
			void OnLeave(EditorWindow& editor) override;

			void OnMouseButtonPressed(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseMoved(EditorWindow& editor, const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;

		private:
			std::optional<Nz::Vector2ui> GetTilePositionFromMouse(EditorWindow& editor, int mouseX, int mouseY) const;

			enum class EditionMode
			{
				DisableTile,
				EnableTile,
				None
			};

			std::vector<Nz::UInt8> m_tilemapContent;
			Ndk::EntityOwner m_tilePreviewEntity;
			Ndk::EntityOwner m_tilemapEntity;
			Nz::DegreeAnglef m_rotation;
			Nz::SpriteRef m_tileSprite;
			Nz::TileMapRef m_tileMap;
			Nz::Vector2f m_origin;
			Nz::Vector2f m_tileSize;
			Nz::Vector2ui m_mapSize;
			EditionMode m_editionMode;
	};
}

#include <MapEditor/Logic/TileMapEditorMode.inl>

#endif
