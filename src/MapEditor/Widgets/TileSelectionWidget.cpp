// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <MapEditor/Widgets/ScrollCanvas.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Platform/EventHandler.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QWidget>
#include <tsl/hopscotch_map.h>
#include <algorithm>

namespace bw
{
	TileSelectionWidget::TileSelectionWidget(const std::string& editorResourceFolder, const std::vector<TilesetGroup>& tilesetGroups, QWidget* parent) :
	QWidget(parent),
	m_activeGroup(InvalidGroup),
	m_firstSelectedTile(0),
	m_lastSelectedTile(0),
	m_tileSize(64.f, 64.f)
	{
		setWindowTitle(tr("Tile selector"));

		m_tileSelectionCanvas = new ScrollCanvas;
		WorldCanvas* worldCanvas = m_tileSelectionCanvas->GetWorldCanvas();
		Nz::WindowEventHandler& eventHandler = worldCanvas->GetEventHandler();
		eventHandler.OnMouseButtonPressed.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
		{
			OnMouseButtonPressed(mouseEvent);
		});

		eventHandler.OnMouseButtonReleased.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
		{
			OnMouseButtonReleased(mouseEvent);
		});

		eventHandler.OnMouseMoved.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouseEvent)
		{
			OnMouseMoved(mouseEvent);
		});


		QTabBar* groupBar = new QTabBar;

		for (const TilesetGroup& tilesetGroup : tilesetGroups)
		{
			groupBar->addTab(QString::fromStdString(tilesetGroup.groupName));

			Nz::Vector2ui mapSize = Nz::Vector2ui::Zero();
			for (const MaterialData& material : tilesetGroup.materials)
			{
				mapSize.x = std::max(mapSize.x, material.tileCount.x);
				mapSize.y += material.tileCount.y;
			}
			//unsigned int tileMapSide = static_cast<unsigned int>(std::ceil(std::sqrt(tileData.size() + 1)));

			std::shared_ptr<Nz::Tilemap> tileMap = Nz::TileMap::New(mapSize, m_tileSize, tilesetGroup.materials.size());

			Nz::Vector2ui tileCursor = Nz::Vector2ui::Zero();

			std::size_t matIndex = 0;
			for (const MaterialData& materialData : tilesetGroup.materials)
			{
				tileMap->SetMaterial(matIndex, materialData.material);

				Nz::Vector2f invTileCount = 1.f / Nz::Vector2f(materialData.tileCount);

				for (std::size_t y = 0; y < materialData.tileCount.y; ++y)
				{
					for (std::size_t x = 0; x < materialData.tileCount.x; ++x)
					{
						Nz::Rectf texCoords = Nz::Rectf(invTileCount * Nz::Vector2f(x, y), invTileCount * Nz::Vector2f(x + 1, y + 1));
						tileMap->EnableTile(Nz::Vector2ui(tileCursor.x + static_cast<unsigned int>(x), tileCursor.y + static_cast<unsigned int>(y)), texCoords, Nz::Color::White, matIndex);
					}
				}
				
				tileCursor.y += materialData.tileCount.y;

				matIndex++;
			}

			entt::entity tilemapEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
			tilemapEntity->AddComponent<Ndk::NodeComponent>();
			tilemapEntity->AddComponent<Ndk::GraphicsComponent>().Attach(tileMap);
			tilemapEntity->Disable();

			Nz::Vector2f tileMapSize = Nz::Vector2f(tileMap->GetMapSize()) * tileMap->GetTileSize();

			GroupData& group = m_groups.emplace_back();
			group.contentSize = Nz::Vector2i(std::ceil(tileMapSize.x), std::ceil(tileMapSize.y));
			group.mapSize = mapSize;
			group.tilemap = tilemapEntity;

			std::size_t rectIndex = 0;
			for (const MaterialData& materialData : tilesetGroup.materials)
			{
				group.materialFirstRectIndices.push_back(rectIndex);
				rectIndex += mapSize.x * materialData.tileCount.y;
			}
		}
		
		connect(groupBar, &QTabBar::currentChanged, [this](int newIndex)
		{
			if (newIndex < 0)
				return;

			SelectTilesetGroup(static_cast<std::size_t>(newIndex));
		});

		m_selectedSprite = Nz::Sprite::New();
		m_selectedSprite->SetMaterial(Nz::MaterialLibrary::Get("TileSelection"));
		m_selectedSprite->SetSize(m_tileSize);

		m_selectionSprite = Nz::Sprite::New(*m_selectedSprite);
		m_selectionSprite->SetColor(Nz::Color(192, 192, 192, 128));
		m_selectionSprite->SetMaterial(Nz::MaterialLibrary::Get("TileSelection"));
		m_selectionSprite->SetSize(m_tileSize);

		m_selectedEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		m_selectedEntity->AddComponent<Ndk::NodeComponent>();
		m_selectedEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_selectedSprite, 1);

		m_selectionEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		m_selectionEntity->AddComponent<Ndk::NodeComponent>();
		m_selectionEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_selectionSprite, 1);
		m_selectionEntity->Disable();

		QToolBar* toolbar = new QToolBar;
		QAction* tileAction = toolbar->addAction(QIcon(QPixmap((editorResourceFolder + "/cloth-24.png").c_str())), tr("Tile mode"));
		connect(tileAction, &QAction::triggered, [this](bool) { EnableTileMode(); });

		QAction* clearAction = toolbar->addAction(QIcon(QPixmap((editorResourceFolder + "/remove_image-24.png").c_str())), tr("Clear mode"));
		connect(clearAction, &QAction::triggered, [this](bool) { EnableClearMode(); });

		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(toolbar);
		layout->addWidget(groupBar);
		layout->addWidget(m_tileSelectionCanvas);

		setLayout(layout);

		SelectTilesetGroup(0);
		EnableTileMode();
	}

	void TileSelectionWidget::SelectTilesetGroup(std::size_t groupIndex)
	{
		assert(groupIndex < m_groups.size());

		if (m_activeGroup != InvalidGroup)
			m_groups[m_activeGroup].tilemap->Disable();

		m_activeGroup = groupIndex;

		m_groups[m_activeGroup].tilemap->Enable();
		m_tileSelectionCanvas->SetContentSize(m_groups[m_activeGroup].contentSize);
	}

	void TileSelectionWidget::SelectRect(std::size_t firstRect, std::size_t lastRect)
	{
		assert(m_activeGroup < m_groups.size());
		auto& currentGroup = m_groups[m_activeGroup];

		assert(firstRect < currentGroup.mapSize.x * currentGroup.mapSize.y);
		assert(lastRect < currentGroup.mapSize.x * currentGroup.mapSize.y);

		if (firstRect > lastRect)
			std::swap(firstRect, lastRect);

		std::size_t firstX = firstRect % currentGroup.mapSize.x;
		std::size_t firstY = firstRect / currentGroup.mapSize.x;
		std::size_t lastX = lastRect % currentGroup.mapSize.x;
		std::size_t lastY = lastRect / currentGroup.mapSize.x;

		if (firstX > lastX)
			std::swap(firstX, lastX);

		if (firstY > lastY)
			std::swap(firstY, lastY);

		Nz::Vector2f firstPosition = Nz::Vector2f(firstX, firstY) * m_tileSize;
		Nz::Vector2f lastPosition = Nz::Vector2f(lastX, lastY) * m_tileSize;

		auto& selectedEntityNode = m_selectedentity.get<Nz::NodeComponent>();
		selectedEntityNode.SetPosition(firstPosition);
		m_selectedSprite->SetSize(lastPosition - firstPosition + m_tileSize);

		m_firstSelectedTile = firstRect;
		m_lastSelectedTile = lastRect;

		EnableTileMode();
	}

	void TileSelectionWidget::EnableClearMode()
	{
		m_selectedEntity->Disable();
		OnClearMode(this);
	}

	void TileSelectionWidget::EnableTileMode()
	{
		assert(m_activeGroup < m_groups.size());
		auto& currentGroup = m_groups[m_activeGroup];

		m_selectedEntity->Enable();

		auto BuildTileSelection = [&](std::size_t tileIndex)
		{
			TileSelection selection;
			selection.groupIndex = m_activeGroup;

			auto it = std::upper_bound(currentGroup.materialFirstRectIndices.begin(), currentGroup.materialFirstRectIndices.end(), tileIndex) - 1;
			assert(it != currentGroup.materialFirstRectIndices.end());

			selection.materialIndex = std::distance(currentGroup.materialFirstRectIndices.begin(), it);
			selection.tileIndex = tileIndex - *it;

			return selection;
		};

		if (m_firstSelectedTile == m_lastSelectedTile)
			OnSelectionMode(this, 1, 1, { BuildTileSelection(m_firstSelectedTile) });
		else
		{
			std::size_t firstX = m_firstSelectedTile % currentGroup.mapSize.x;
			std::size_t firstY = m_firstSelectedTile / currentGroup.mapSize.x;
			std::size_t lastX = m_lastSelectedTile % currentGroup.mapSize.x;
			std::size_t lastY = m_lastSelectedTile / currentGroup.mapSize.x;

			if (firstX > lastX)
				std::swap(firstX, lastX);

			if (firstY > lastY)
				std::swap(firstY, lastY);

			std::vector<TileSelection> selectedTiles;
			for (std::size_t y = firstY; y <= lastY; ++y)
			{
				for (std::size_t x = firstX; x <= lastX; ++x)
					selectedTiles.push_back(BuildTileSelection(y * currentGroup.mapSize.x + x));
			}

			OnSelectionMode(this, lastX - firstX + 1, lastY - firstY + 1, std::move(selectedTiles));
		}
	}

	std::size_t TileSelectionWidget::GetHoveredTile(int x, int y)
	{
		WorldCanvas* worldCanvas = m_tileSelectionCanvas->GetWorldCanvas();
		auto& camera = worldCanvas->GetCamera();
		Nz::Vector2f worldPos = camera.Unproject({ float(x), float(y) });

		assert(m_activeGroup < m_groups.size());
		auto& currentGroup = m_groups[m_activeGroup];

		Nz::Rectf tilemapRect(0.f, 0.f, currentGroup.mapSize.x * m_tileSize.x, currentGroup.mapSize.y * m_tileSize.y);

		if (tilemapRect.Contains(worldPos))
		{
			worldPos.x = std::floor(worldPos.x / m_tileSize.x);
			worldPos.y = std::floor(worldPos.y / m_tileSize.y);

			Nz::Vector2ui tilePos(worldPos);
			return tilePos.y * currentGroup.mapSize.x + tilePos.x;
		}

		return InvalidTile;
	}

	void TileSelectionWidget::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
	{
		if (mouseEvent.button != Nz::Mouse::Left)
			return;
		
		std::size_t hoveredTile = GetHoveredTile(mouseEvent.x, mouseEvent.y);
		if (hoveredTile != InvalidTile)
			m_currentSelectionFirstRect = hoveredTile;
	}
	
	void TileSelectionWidget::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
	{
		if (mouseEvent.button != Nz::Mouse::Left)
			return;

		std::size_t selectionFirstRect = (m_currentSelectionFirstRect) ? *m_currentSelectionFirstRect : InvalidTile;
		std::size_t selectionLastRect = GetHoveredTile(mouseEvent.x, mouseEvent.y);

		m_currentSelectionFirstRect.reset();
		m_selectionEntity->Disable();

		if (selectionFirstRect == InvalidTile && selectionLastRect != InvalidTile)
			SelectRect(selectionLastRect, selectionLastRect);
		else if (selectionFirstRect != InvalidTile && selectionLastRect == InvalidTile)
			SelectRect(selectionFirstRect, selectionFirstRect);
		else if (selectionFirstRect != InvalidTile && selectionLastRect != InvalidTile)
			SelectRect(selectionFirstRect, selectionLastRect);
	}

	void TileSelectionWidget::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseEvent)
	{
		std::size_t selectionLastRect = GetHoveredTile(mouseEvent.x, mouseEvent.y);

		assert(m_activeGroup < m_groups.size());
		auto& currentGroup = m_groups[m_activeGroup];

		if (m_currentSelectionFirstRect)
		{
			std::size_t selectionFirstRect = (m_currentSelectionFirstRect) ? *m_currentSelectionFirstRect : InvalidTile;

			auto UpdateSelection = [&](std::size_t firstRect, std::size_t lastRect)
			{
				std::size_t firstX = firstRect % currentGroup.mapSize.x;
				std::size_t firstY = firstRect / currentGroup.mapSize.x;
				std::size_t lastX = lastRect % currentGroup.mapSize.x;
				std::size_t lastY = lastRect / currentGroup.mapSize.x;

				if (firstX > lastX)
					std::swap(firstX, lastX);

				if (firstY > lastY)
					std::swap(firstY, lastY);

				Nz::Vector2f firstPosition = Nz::Vector2f(firstX, firstY) * m_tileSize;
				Nz::Vector2f lastPosition = Nz::Vector2f(lastX, lastY) * m_tileSize;

				m_selectionEntity->Enable();
				auto& selectionEntityNode = m_selectionentity.get<Nz::NodeComponent>();
				selectionEntityNode.SetPosition(firstPosition);
				m_selectionSprite->SetSize(lastPosition - firstPosition + m_tileSize);
			};

			if (selectionFirstRect == InvalidTile && selectionLastRect != InvalidTile)
				UpdateSelection(selectionLastRect, selectionLastRect);
			else if (selectionFirstRect != InvalidTile && selectionLastRect == InvalidTile)
				UpdateSelection(selectionFirstRect, selectionFirstRect);
			else if (selectionFirstRect != InvalidTile && selectionLastRect != InvalidTile)
				UpdateSelection(selectionFirstRect, selectionLastRect);
		}
		else if (selectionLastRect != InvalidTile)
		{
			Nz::Vector2f firstPosition = Nz::Vector2f(selectionLastRect % currentGroup.mapSize.x, selectionLastRect / currentGroup.mapSize.x) * m_tileSize;

			m_selectionEntity->Enable();
			auto& selectionEntityNode = m_selectionentity.get<Nz::NodeComponent>();
			selectionEntityNode.SetPosition(firstPosition);
			m_selectionSprite->SetSize(m_tileSize);
		}
		else
			m_selectionEntity->Disable();
	}
}
