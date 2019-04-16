// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Platform/Event.hpp>
#include <NDK/EntityOwner.hpp>
#include <QtWidgets/QWidget>
#include <vector>

class QScrollBar;

namespace bw
{
	class ScrollCanvas;

	class TileSelectionWidget : public QWidget
	{
		public:
			struct TileData;

			TileSelectionWidget(const std::string& editorResourceFolder, const std::vector<TileData>& tileData, const std::vector<Nz::MaterialRef>& materials, QWidget* parent = nullptr);
			~TileSelectionWidget() = default;

			void SelectTile(std::size_t tileIndex);

			NazaraSignal(OnNoTileSelected, TileSelectionWidget* /*emitter*/);
			NazaraSignal(OnTileSelected, TileSelectionWidget* /*emitter*/, std::size_t /*tileIndex*/);

			struct TileData
			{
				std::size_t materialIndex;
				Nz::Rectf texCoords;
			};

		private:
			void EnableClearMode();
			void EnableTileMode();

			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseEvent);
			void SelectRect(std::size_t rectIndex);

			std::size_t m_selectedTile;
			std::size_t m_tileCount;
			Ndk::EntityOwner m_selectedEntity;
			Ndk::EntityOwner m_tileMapEntity;
			Nz::SpriteRef m_selectionSprite;
			Nz::Vector2ui m_mapSize;
			Nz::Vector2f m_tileSize;
			ScrollCanvas* m_tileSelectionCanvas;
	};
}


#include <MapEditor/Widgets/TileSelectionWidget.inl>

#endif