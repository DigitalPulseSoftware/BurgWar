// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMessageBox>
#include <iostream>

namespace bw
{
	TileMapEditorMode::TileMapEditorMode(const Ndk::EntityHandle& targetEntity, TileMapData tilemapData, const std::vector<TileData>& tiles, EditorWindow& editor) :
	EntityEditorMode(targetEntity, editor),
	m_editionMode(EditionMode::None),
	m_tilemapData(std::move(tilemapData)),
	m_clearMode(false)
	{
		Nz::ImageRef eraserImage = Nz::ImageLibrary::Get("Eraser");
		if (eraserImage)
		{
			m_eraserCursor = Nz::Cursor::New();
			m_eraserCursor->Create(*eraserImage, Nz::Vector2i(3, 31), Nz::SystemCursor_Default);
		}

		// Compute tilemap and load materials

		const std::string& gameAssetsFolder = GetEditorWindow().GetConfig().GetStringOption("Assets.ResourceFolder");

		tsl::hopscotch_map<std::string /*materialPath*/, std::size_t /*materialIndex*/> materials;
		for (const auto& tile : tiles)
		{
			auto it = materials.find(tile.materialPath);
			if (it == materials.end())
			{
				materials.emplace(tile.materialPath, materials.size());

				Nz::MaterialRef material = Nz::MaterialManager::Get(gameAssetsFolder + "/" + tile.materialPath);
				if (material)
				{
					// Force alpha blending
					material->Configure("Translucent2D");
					material->SetDiffuseMap(gameAssetsFolder + "/" + tile.materialPath); //< FIXME
				}
				else
					material = Nz::Material::GetDefault();

				m_materials.emplace_back(material);
			}
		}

		m_tileData.resize(tiles.size());
		for (std::size_t i = 0; i < tiles.size(); ++i)
		{
			const auto& tile = tiles[i];

			auto it = materials.find(tile.materialPath);
			assert(it != materials.end());

			auto& widgetTile = m_tileData[i];
			widgetTile.materialIndex = it->second;
			widgetTile.texCoords = tile.texCoords;
		}

		m_hoveringTileSprite = Nz::Sprite::New();
		m_hoveringTileSprite->SetMaterial(Nz::MaterialLibrary::Get("TileSelection"));
		m_hoveringTileSprite->SetSize(m_tilemapData.tileSize);
	}

	void TileMapEditorMode::EnableClearMode(bool clearMode)
	{
		if (m_clearMode == clearMode)
			return;

		m_clearMode = clearMode;

		MapCanvas* canvas = GetEditorWindow().GetMapCanvas();
		if (m_clearMode)
		{
			if (m_eraserCursor)
				canvas->SetCursor(m_eraserCursor);

			m_hoveringTileSprite->SetColor(Nz::Color::Red);
		}
		else
		{
			canvas->SetCursor(Nz::SystemCursor_Default);
			m_hoveringTileSprite->SetColor(Nz::Color::White);
		}
	}

	void TileMapEditorMode::OnEnter()
	{
		EntityEditorMode::OnEnter();

		EditorWindow& editorWindow = GetEditorWindow();

		m_tileEditorWidget = new QDockWidget(QObject::tr("Tile selector"), &editorWindow);
		m_tileEditorWidget->setAttribute(Qt::WA_DeleteOnClose);
		m_tileEditorWidget->setFloating(true);
		QObject::connect(m_tileEditorWidget, &QObject::destroyed, [this](QObject* w)
		{
			assert(m_tileEditorWidget == w);

			m_tileEditorWidget = nullptr;

			EditorWindow& editorWindow = GetEditorWindow();

			QMessageBox::StandardButton button = QMessageBox::question(&editorWindow, QObject::tr("Save work?"), QObject::tr("Do you want to save and apply your modifications?"));
			if (button == QMessageBox::Yes)
				OnEditionFinished(this, GetTileMapData());
			else
				OnEditionCancelled(this);

			editorWindow.SwitchToMode(std::make_shared<BasicEditorMode>(editorWindow));

			// Warning: "this" is potentially destroyed at this point
		});

		const std::string& editorAssetsFolder = GetEditorWindow().GetConfig().GetStringOption("Assets.EditorFolder");

		TileSelectionWidget* tileWidget = new TileSelectionWidget(editorAssetsFolder, m_tileData, m_materials);
		tileWidget->OnNoTileSelected.Connect([this](TileSelectionWidget* tileSelection)
		{
			EnableClearMode(true);
		});

		tileWidget->OnTileSelected.Connect([this](TileSelectionWidget* tileSelection, std::size_t tileIndex)
		{
			EnableClearMode(false);
			OnTileSelected(tileIndex);
		});

		tileWidget->SelectTile(0);

		tileWidget->resize(256, 256); //< FIXME: This is ignored for some reason

		m_tileEditorWidget->setWidget(tileWidget);

		editorWindow.addDockWidget(Qt::LeftDockWidgetArea, m_tileEditorWidget);

		MapCanvas* mapCanvas = editorWindow.GetMapCanvas();

		m_tilemapEntity = mapCanvas->GetWorld().CreateEntity();

		m_tileMap = Nz::TileMap::New(m_tilemapData.mapSize, m_tilemapData.tileSize, m_materials.size());
		for (std::size_t matIndex = 0; matIndex < m_materials.size(); ++matIndex)
			m_tileMap->SetMaterial(matIndex, m_materials[matIndex]);

		m_tilemapData.content.resize(m_tilemapData.mapSize.x * m_tilemapData.mapSize.y);
		for (std::size_t i = 0; i < m_tilemapData.content.size(); ++i)
		{
			Nz::UInt32 value = m_tilemapData.content[i];
			
			Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % m_tilemapData.mapSize.x), static_cast<unsigned int>(i / m_tilemapData.mapSize.x) };
			if (value > 0)
			{
				assert(value - 1 < m_tileData.size());

				const auto& tileData = m_tileData[value - 1];
				m_tileMap->EnableTile(tilePos, tileData.texCoords, Nz::Color::White, tileData.materialIndex);
			}
		}

		auto& tileMapNode = m_tilemapEntity->AddComponent<Ndk::NodeComponent>();
		tileMapNode.SetPosition(m_tilemapData.origin);
		tileMapNode.SetRotation(m_tilemapData.rotation);

		auto& tileMapGraphics = m_tilemapEntity->AddComponent<Ndk::GraphicsComponent>();
		tileMapGraphics.Attach(m_tileMap, 1);

		m_tileSelectionEntity = mapCanvas->GetWorld().CreateEntity();

		m_tileSelectionEntity->AddComponent<Ndk::NodeComponent>();

		auto& spriteTexture = m_tileSelectionEntity->AddComponent<Ndk::GraphicsComponent>();
		spriteTexture.Attach(m_hoveringTileSprite, 2);
	}

	void TileMapEditorMode::OnLeave()
	{
		EntityEditorMode::OnLeave();

		m_tileEditorWidget->deleteLater();

		m_tileMap.Reset();
		m_tilemapEntity.Reset();
	}

	void TileMapEditorMode::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Left)
			return;

		m_editionMode = (m_clearMode) ? EditionMode::DisableTile : EditionMode::EnableTile;

		if (std::optional<Nz::Vector2ui> tilePosition = GetTilePositionFromMouse(mouseButton.x, mouseButton.y))
			ApplyTile(tilePosition);
	}

	void TileMapEditorMode::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Left)
			return;

		m_editionMode = EditionMode::None;
	}

	void TileMapEditorMode::OnMouseEntered()
	{
		MapCanvas* canvas = GetEditorWindow().GetMapCanvas();

		if (m_clearMode)
		{
			if (m_eraserCursor)
				canvas->SetCursor(m_eraserCursor);
		}
		else
			canvas->SetCursor(Nz::SystemCursor_Default);
	}

	void TileMapEditorMode::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		MapCanvas* canvas = GetEditorWindow().GetMapCanvas();

		std::optional<Nz::Vector2ui> tilePosition = GetTilePositionFromMouse(mouseMoved.x, mouseMoved.y);
		if (tilePosition)
		{
			m_tileSelectionEntity->Enable();

			auto& node = m_tileSelectionEntity->GetComponent<Ndk::NodeComponent>();
			node.SetPosition(Nz::Vector2f(*tilePosition) * m_tilemapData.tileSize + m_tilemapData.origin);

			ApplyTile(tilePosition);
		}
		else
			m_tileSelectionEntity->Disable();
	}
	
	void TileMapEditorMode::ApplyTile(std::optional<Nz::Vector2ui> tilePosition)
	{
		switch (m_editionMode)
		{
			case EditionMode::DisableTile:
			{
				m_tileMap->DisableTile(*tilePosition);

				std::size_t tileIndex = tilePosition->y * m_tilemapData.mapSize.x + tilePosition->x;

				assert(tileIndex < m_tilemapData.content.size());
				m_tilemapData.content[tileIndex] = 0;
				break;
			}

			case EditionMode::EnableTile:
			{
				const auto& tileData = m_tileData[m_selectedTile];

				m_tileMap->EnableTile(*tilePosition, tileData.texCoords, Nz::Color::White, tileData.materialIndex);

				std::size_t tileIndex = tilePosition->y * m_tilemapData.mapSize.x + tilePosition->x;

				assert(tileIndex < m_tilemapData.content.size());
				m_tilemapData.content[tileIndex] = static_cast<Nz::UInt32>(m_selectedTile + 1);
				break;
			}

			case EditionMode::None:
			default:
				break;
		}
	}

	std::optional<Nz::Vector2ui> TileMapEditorMode::GetTilePositionFromMouse(int mouseX, int mouseY) const
	{
		const MapCanvas* canvas = GetEditorWindow().GetMapCanvas();

		auto& cameraComponent = canvas->GetCameraEntity()->GetComponent<Ndk::CameraComponent>();
		Nz::Vector2f worldPos = Nz::Vector2f(cameraComponent.Unproject(Nz::Vector3f(mouseX, mouseY, 0.f)));

		Nz::Rectf tilemapRect(m_tilemapData.origin.x, m_tilemapData.origin.y, m_tilemapData.mapSize.x * m_tilemapData.tileSize.x, m_tilemapData.mapSize.y * m_tilemapData.tileSize.y);
		if (tilemapRect.Contains(worldPos))
		{
			Nz::Vector2f relativePosition = worldPos - m_tilemapData.origin;
			relativePosition.x = std::floor(relativePosition.x / m_tilemapData.tileSize.x);
			relativePosition.y = std::floor(relativePosition.y / m_tilemapData.tileSize.y);

			return Nz::Vector2ui(relativePosition);
		}
		else
			return std::nullopt;
	}

	void TileMapEditorMode::OnTileSelected(std::size_t tileIndex)
	{
		assert(tileIndex < m_tileData.size());

		m_selectedTile = tileIndex;
	}
}
