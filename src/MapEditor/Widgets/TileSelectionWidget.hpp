// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP

#include <Nazara/Utils/Signal.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Platform/Event.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <QtWidgets/QWidget>
#include <optional>
#include <vector>

class QScrollBar;

namespace bw
{
	class ScrollCanvas;

	class TileSelectionWidget : public QWidget
	{
		public:
			struct MaterialData;
			struct TilesetGroup;
			struct TileSelection;

			TileSelectionWidget(const std::string& editorResourceFolder, const std::vector<TilesetGroup>& tilesetGroups, QWidget* parent = nullptr);
			~TileSelectionWidget() = default;

			void SelectTilesetGroup(std::size_t groupIndex);

			NazaraSignal(OnClearMode, TileSelectionWidget* /*emitter*/);
			NazaraSignal(OnSelectionMode, TileSelectionWidget* /*emitter*/, std::size_t /*width*/, std::size_t /*height*/, std::vector<TileSelection> /*tiles*/);

			struct MaterialData
			{
				Nz::MaterialRef material;
				Nz::Vector2ui tileCount;
			};

			struct TilesetGroup
			{
				std::string groupName;
				std::vector<MaterialData> materials;
			};

			struct TileSelection
			{
				std::size_t groupIndex;
				std::size_t materialIndex;
				std::size_t tileIndex;
			};

		private:
			void EnableClearMode();
			void EnableTileMode();
			std::size_t GetHoveredTile(int x, int y);
			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseEvent);
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseEvent);
			void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseEvent);
			void SelectRect(std::size_t firstRect, std::size_t lastRect);

			struct GroupData
			{
				Ndk::EntityOwner tilemap;
				Nz::Vector2i contentSize;
				Nz::Vector2ui mapSize;
				std::vector<std::size_t> materialFirstRectIndices;
			};

			static constexpr std::size_t InvalidGroup = std::numeric_limits<std::size_t>::max();
			static constexpr std::size_t InvalidTile = std::numeric_limits<std::size_t>::max();

			std::size_t m_activeGroup;
			std::size_t m_firstSelectedTile;
			std::size_t m_lastSelectedTile;
			std::optional<std::size_t> m_currentSelectionFirstRect;
			std::vector<GroupData> m_groups;
			Ndk::EntityOwner m_selectedEntity;
			Ndk::EntityOwner m_selectionEntity;
			Nz::Bitset<Nz::UInt64> m_activeTiles;
			Nz::SpriteRef m_selectedSprite;
			Nz::SpriteRef m_selectionSprite;
			Nz::Vector2f m_tileSize;
			ScrollCanvas* m_tileSelectionCanvas;
	};
}


#include <MapEditor/Widgets/TileSelectionWidget.inl>

#endif
