// Copyright (C) 2019 Jérôme Leclercq
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
#include <NDK/EntityOwner.hpp>
#include <optional>

class QDockWidget;

namespace bw
{
	class TileMapEditorMode : public EntityEditorMode
	{
		public:
			TileMapEditorMode(const Ndk::EntityHandle& targetEntity, TileMapData tilemapData, const std::vector<TileData>& tiles, EditorWindow& editor);
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
			std::vector<TileSelectionWidget::TileData> m_tileData;
			Ndk::EntityOwner m_tileSelectionEntity;
			Ndk::EntityOwner m_tilemapEntity;
			Nz::CursorRef m_eraserCursor;
			Nz::SpriteRef m_hoveringTileSprite;
			Nz::TileMapRef m_tileMap;
			EditionMode m_editionMode;
			TileMapData m_tilemapData;
			QDockWidget* m_tileEditorWidget;
			bool m_clearMode;
	};
}

#include <MapEditor/Logic/TileMapEditorMode.inl>

#endif
