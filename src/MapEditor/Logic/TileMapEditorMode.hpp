// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP

#include <MapEditor/Logic/EntityEditorMode.hpp>
#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Platform/Cursor.hpp>
#include <NDK/EntityOwner.hpp>
#include <optional>

namespace bw
{
	class TileMapEditorMode : public EntityEditorMode
	{
		public:
			struct TileData;

			TileMapEditorMode(const Ndk::EntityHandle& targetEntity, const Nz::Vector2f& origin, const Nz::DegreeAnglef& rotation,
			                  const Nz::Vector2ui& mapSize, const Nz::Vector2f& tileSize, std::vector<Nz::UInt32> content, 
			                  const std::vector<TileData>& tiles, EditorWindow& editor);
			~TileMapEditorMode() = default;

			void EnableClearMode(bool clearMode);

			void OnEnter() override;
			void OnLeave() override;

			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseEntered() override;
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;

			struct TileData
			{
				Nz::MaterialRef material;
				Nz::Rectf texCoords;
			};

		private:
			void ApplyTile(std::optional<Nz::Vector2ui> tilePosition);
			std::optional<Nz::Vector2ui> GetTilePositionFromMouse(int mouseX, int mouseY) const;
			void OnTileSelected(std::size_t tileIndex);

			enum class EditionMode
			{
				DisableTile,
				EnableTile,
				None
			};

			std::size_t m_selectedTile;
			std::vector<Nz::MaterialRef> m_materials;
			std::vector<Nz::UInt32> m_tilemapContent;
			std::vector<TileSelectionWidget::TileData> m_tileData;
			Ndk::EntityOwner m_tileSelectionEntity;
			Ndk::EntityOwner m_tilemapEntity;
			Nz::CursorRef m_eraserCursor;
			Nz::DegreeAnglef m_rotation;
			Nz::SpriteRef m_hoveringTileSprite;
			Nz::TileMapRef m_tileMap;
			Nz::Vector2f m_origin;
			Nz::Vector2f m_tileSize;
			Nz::Vector2ui m_mapSize;
			EditionMode m_editionMode;
			bool m_clearMode;
	};
}

#include <MapEditor/Logic/TileMapEditorMode.inl>

#endif
