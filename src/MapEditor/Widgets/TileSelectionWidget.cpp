// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <MapEditor/Widgets/ScrollCanvas.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Platform/EventHandler.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <iostream>
#include <tsl/hopscotch_map.h>

namespace bw
{
	TileSelectionWidget::TileSelectionWidget(const std::vector<TileData>& tileData, const std::vector<Nz::MaterialRef>& materials, QWidget* parent) :
	QWidget(parent),
	m_tileSize(64.f, 64.f),
	m_selectedTile(0)
	{
		setWindowTitle(tr("Tile selector"));

		m_tileSelectionCanvas = new ScrollCanvas;
		WorldCanvas* worldCanvas = m_tileSelectionCanvas->GetWorldCanvas();
		Nz::EventHandler& eventHandler = worldCanvas->GetEventHandler();
		eventHandler.OnMouseButtonPressed.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
		{
			OnMouseButtonPressed(mouseEvent);
		});

		unsigned int tileMapSide = static_cast<unsigned int>(std::ceil(std::sqrt(tileData.size() + 1)));
		m_mapSize = Nz::Vector2ui(tileMapSide, tileMapSide);
		m_tileCount = tileData.size();

		Nz::TileMapRef tileMap = Nz::TileMap::New(m_mapSize, m_tileSize, materials.size());
		for (std::size_t matIndex = 0; matIndex < materials.size(); ++matIndex)
			tileMap->SetMaterial(matIndex, materials[matIndex]);

		for (std::size_t tileId = 0; tileId < m_tileCount; ++tileId)
		{
			const auto& tile = tileData[tileId];

			Nz::Vector2ui tilePosition(Nz::Vector2<std::size_t>(tileId % tileMapSide, tileId / tileMapSide));
			tileMap->EnableTile(tilePosition, tile.texCoords, Nz::Color::White, tile.materialIndex);
		}

		m_tileMapEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		m_tileMapEntity->AddComponent<Ndk::NodeComponent>();
		m_tileMapEntity->AddComponent<Ndk::GraphicsComponent>().Attach(tileMap);

		Nz::Vector2f tileMapSize = Nz::Vector2f(tileMap->GetMapSize()) * tileMap->GetTileSize();
		m_tileSelectionCanvas->SetContentSize(Nz::Vector2i(std::ceil(tileMapSize.x), std::ceil(tileMapSize.y)));

		Nz::MaterialRef selectionMaterial = Nz::Material::New("Translucent2D");
		selectionMaterial->SetDiffuseMap("resources/tile_selection.png");

		m_selectionSprite = Nz::Sprite::New();
		m_selectionSprite->SetMaterial(selectionMaterial);
		m_selectionSprite->SetSize(tileMap->GetTileSize());

		m_selectedEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		m_selectedEntity->AddComponent<Ndk::NodeComponent>();
		m_selectedEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_selectionSprite, 1);

		QToolBar* toolbar = new QToolBar;
		QAction* tileAction = toolbar->addAction(QIcon(QPixmap("resources/gui/icons/cloth-24.png")), tr("Tile mode"));
		connect(tileAction, &QAction::triggered, [this](bool) { EnableTileMode(); });

		QAction* clearAction = toolbar->addAction(QIcon(QPixmap("resources/gui/icons/remove_image-24.png")), tr("Clear mode"));
		connect(clearAction, &QAction::triggered, [this](bool) { EnableClearMode(); });

		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(toolbar);
		layout->addWidget(m_tileSelectionCanvas);

		setLayout(layout);

		EnableTileMode();
	}

	void TileSelectionWidget::SelectTile(std::size_t tileIndex)
	{
		assert(tileIndex < m_tileCount);

		SelectRect(tileIndex);
	}

	void TileSelectionWidget::SelectRect(std::size_t rectIndex)
	{
		assert(rectIndex < m_mapSize.x * m_mapSize.y);

		auto& selectedEntityNode = m_selectedEntity->GetComponent<Ndk::NodeComponent>();
		selectedEntityNode.SetPosition(Nz::Vector2f(rectIndex % m_mapSize.x, rectIndex / m_mapSize.y) * m_tileSize);

		if (rectIndex >= 0 && rectIndex < m_tileCount)
		{
			m_selectedTile = rectIndex;
			EnableTileMode();
		}
		else
			EnableClearMode();
	}

	void TileSelectionWidget::EnableClearMode()
	{
		m_selectionSprite->SetColor(Nz::Color(128, 128, 128));
		OnNoTileSelected(this);
	}

	void TileSelectionWidget::EnableTileMode()
	{
		m_selectionSprite->SetColor(Nz::Color::White);
		OnTileSelected(this, m_selectedTile);
	}

	void TileSelectionWidget::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseEvent)
	{
		if (mouseEvent.button != Nz::Mouse::Left)
			return;

		WorldCanvas* worldCanvas = m_tileSelectionCanvas->GetWorldCanvas();
		auto& cameraComponent = worldCanvas->GetCameraEntity()->GetComponent<Ndk::CameraComponent>();
		Nz::Vector2f worldPos = Nz::Vector2f(cameraComponent.Unproject(Nz::Vector3f(mouseEvent.x, mouseEvent.y, 0.f)));

		Nz::Rectf tilemapRect(0.f, 0.f, m_mapSize.x * m_tileSize.x, m_mapSize.y * m_tileSize.y);

		if (tilemapRect.Contains(worldPos))
		{
			worldPos.x = std::floor(worldPos.x / m_tileSize.x);
			worldPos.y = std::floor(worldPos.y / m_tileSize.y);

			Nz::Vector2ui tilePos(worldPos);
			SelectRect(tilePos.y * m_mapSize.x + tilePos.x);
		}
	}
}