// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_TILEMAP_EDITOR_MODE_HPP

#include <MapEditor/Logic/EntityEditorMode.hpp>
#include <ClientLib/Utility/TileMapData.hpp>
#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Platform/Cursor.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <optional>

class QDockWidget;

namespace bw
{
	class TileMapEditorMode : public EntityEditorMode
	{
		public:
			TileMapEditorMode(EditorWindow& editor, entt::entity targetEntity, TileMapData tilemapData, const std::vector<TileMaterialData>& materials, const std::vector<TileData>& tiles);
			~TileMapEditorMode() = default;

			void EnableClearMode(bool clearMode);

			inline const TileMapData& GetTileMapData() const;

			void OnEnter() override;
			void OnLeave() override;

			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseEntered() override;
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) override;

			NazaraSignal(OnEditionCancelled, TileMapEditorMode* /*editorMode*/);
			NazaraSignal(OnEditionFinished, TileMapEditorMode* /*editorMode*/, const TileMapData& /*tileMapData*/);

		private:
			struct SelectedTiles;

			void ApplyTile(std::optional<Nz::Vector2ui> tilePosition);
			std::optional<Nz::Vector2ui> GetTilePositionFromMouse(int mouseX, int mouseY) const;
			void UpdateSelection(std::size_t width, std::size_t height, const std::vector<TileSelectionWidget::TileSelection>& selectedTiles);

			enum class EditionMode
			{
				DisableTile,
				EnableTile,
				None
			};

			struct SelectedTiles
			{
				std::size_t width = 0;
				std::size_t height = 0;
				std::vector<std::size_t> tiles;
			};

			struct Tiles
			{
				std::size_t materialIndex;
				Nz::Rectf texCoords;
			};

			SelectedTiles m_selection;
			std::vector<TileSelectionWidget::TilesetGroup> m_tilesetGroups;
			std::vector<Tiles> m_tiles;
			EntityOwner m_tileSelectionEntity;
			EntityOwner m_tilemapEntity;
			Nz::CursorRef m_eraserCursor;
			std::shared_ptr<Nz::Sprite> m_hoveringTileSprite;
			Nz::TileMapRef m_tileMap;
			EditionMode m_editionMode;
			TileMapData m_tilemapData;
			QDockWidget* m_tileEditorWidget;
			bool m_clearMode;
	};
}

#include <MapEditor/Logic/TileMapEditorMode.inl>

#endif
