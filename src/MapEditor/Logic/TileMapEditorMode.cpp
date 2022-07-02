// Copyright (C) 2020 Jérôme Leclercq
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
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMessageBox>

namespace bw
{
	TileMapEditorMode::TileMapEditorMode(EditorWindow& editor, entt::entity targetEntity, TileMapData tilemapData, const std::vector<TileMaterialData>& materials, const std::vector<TileData>& tiles) :
	EntityEditorMode(targetEntity, editor),
	m_editionMode(EditionMode::None),
	m_tilemapData(std::move(tilemapData)),
	m_clearMode(false)
	{
		std::shared_ptr<Nz::Image> eraserImage = Nz::ImageLibrary::Get("Eraser");
		if (eraserImage)
		{
			m_eraserCursor = Nz::Cursor::New();
			m_eraserCursor->Create(*eraserImage, Nz::Vector2i(3, 31), Nz::SystemCursor_Default);
		}

		// Compute tilemap and load materials

		const std::string& gameAssetsFolder = GetEditorWindow().GetConfig().GetStringValue("Resources.AssetDirectory");

		tsl::hopscotch_map<std::string /*materialPath*/, std::size_t /*materialIndex*/> matPathToIndex;

		std::size_t matIndex = 0;
		for (const auto& material : materials)
		{
			if (!matPathToIndex.emplace(material.path, matIndex).second)
				throw std::runtime_error(material.path + " appeared multiple times in materials");

			auto it = std::find_if(m_tilesetGroups.begin(), m_tilesetGroups.end(), [&](const TileSelectionWidget::TilesetGroup& group) { return group.groupName == material.group; });
			if (it == m_tilesetGroups.end())
			{
				auto& newGroup = m_tilesetGroups.emplace_back();
				newGroup.groupName = material.group;

				it = m_tilesetGroups.end() - 1;
			}

			auto& newMaterial = it->materials.emplace_back();

			//TODO: Use asset manager
			newMaterial.material = Nz::MaterialManager::Get(gameAssetsFolder + "/" + material.path);
			if (newMaterial.material)
			{
				// Force alpha blending
				newMaterial.material->Configure("Translucent2D");
				newMaterial.material->SetDiffuseMap(gameAssetsFolder + "/" + material.path); //< FIXME
			}
			else
				newMaterial.material = Nz::Material::GetDefault();

			newMaterial.tileCount = material.tileCount;

			++matIndex;
		}

		std::size_t tileIndex = 0;
		for (const auto& tile : tiles)
		{
			auto matIt = matPathToIndex.find(tile.materialPath);
			if (matIt == matPathToIndex.end())
				throw std::runtime_error("Tile #" + std::to_string(tileIndex) + " uses material " + tile.materialPath + " which is not referenced in materials");

			auto& tileData = m_tiles.emplace_back();
			tileData.materialIndex = matIt.value();
			tileData.texCoords = tile.texCoords;

			tileIndex++;
		}

		m_hoveringTileSprite = Nz::Sprite::New();
		m_hoveringTileSprite->SetMaterial(Nz::MaterialLibrary::Get("TileSelection"));
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

		m_tileEditorWidget = new QDockWidget(editorWindow.tr("Tile selector"), &editorWindow);
		m_tileEditorWidget->setAttribute(Qt::WA_DeleteOnClose);
		m_tileEditorWidget->setFloating(true);
		QObject::connect(m_tileEditorWidget, &QObject::destroyed, [this](QObject* w)
		{
			assert(m_tileEditorWidget == w);
			NazaraUnused(w);

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

		const std::string& editorAssetsFolder = GetEditorWindow().GetConfig().GetStringValue("Resources.EditorDirectory");

		TileSelectionWidget* tileWidget = new TileSelectionWidget(editorAssetsFolder, m_tilesetGroups);
		tileWidget->OnClearMode.Connect([this](TileSelectionWidget* /*tileSelection*/)
		{
			UpdateSelection(1, 1, {});
			EnableClearMode(true);
		});

		tileWidget->OnSelectionMode.Connect([this](TileSelectionWidget* /*tileSelection*/, std::size_t width, std::size_t height, std::vector<TileSelectionWidget::TileSelection> tiles)
		{
			UpdateSelection(width, height, tiles);
			EnableClearMode(false);
		});

		tileWidget->resize(256, 256); //< FIXME: This is ignored for some reason

		m_tileEditorWidget->setWidget(tileWidget);

		editorWindow.addDockWidget(Qt::LeftDockWidgetArea, m_tileEditorWidget);

		MapCanvas* mapCanvas = editorWindow.GetMapCanvas();

		m_tilemapEntity = mapCanvas->GetWorld().CreateEntity();

		std::size_t materialCount = 0;
		for (const auto& tilesetGroup : m_tilesetGroups)
			materialCount += tilesetGroup.materials.size();

		m_tileMap = Nz::TileMap::New(m_tilemapData.mapSize, m_tilemapData.tileSize, materialCount);

		std::size_t materialIndex = 0;
		for (const auto& tilesetGroup : m_tilesetGroups)
		{
			for (const auto& mat : tilesetGroup.materials)
				m_tileMap->SetMaterial(materialIndex++, mat.material);
		}

		m_tilemapData.content.resize(m_tilemapData.mapSize.x * m_tilemapData.mapSize.y);
		for (std::size_t i = 0; i < m_tilemapData.content.size(); ++i)
		{
			Nz::UInt32 value = m_tilemapData.content[i];
			
			Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % m_tilemapData.mapSize.x), static_cast<unsigned int>(i / m_tilemapData.mapSize.x) };
			if (value > 0)
			{
				assert(value - 1 < m_tiles.size());

				const auto& tileData = m_tiles[value - 1];
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
		m_tileSelectionEntity.Reset();
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
		std::optional<Nz::Vector2ui> tilePosition = GetTilePositionFromMouse(mouseMoved.x, mouseMoved.y);
		if (tilePosition)
		{
			m_tileSelectionEntity->Enable();

			auto& node = m_tileSelectionentity.get<Nz::NodeComponent>();
			node.SetPosition(Nz::Vector2f(*tilePosition) * m_tilemapData.tileSize + m_tilemapData.origin);

			std::size_t selectionWidth = std::min<std::size_t>(m_tilemapData.mapSize.x - tilePosition->x, m_selection.width);
			std::size_t selectionHeight = std::min<std::size_t>(m_tilemapData.mapSize.y - tilePosition->y, m_selection.height);

			m_hoveringTileSprite->SetSize({ m_tilemapData.tileSize.x * selectionWidth, m_tilemapData.tileSize.y * selectionHeight });

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
				assert(tilePosition);
				m_tileMap->DisableTile(*tilePosition);

				std::size_t tileIndex = tilePosition->y * m_tilemapData.mapSize.x + tilePosition->x;

				assert(tileIndex < m_tilemapData.content.size());
				m_tilemapData.content[tileIndex] = 0;
				break;
			}

			case EditionMode::EnableTile:
			{
				assert(tilePosition);

				std::size_t selectionWidth = std::min<std::size_t>(m_tilemapData.mapSize.x - tilePosition->x, m_selection.width);
				std::size_t selectionHeight = std::min<std::size_t>(m_tilemapData.mapSize.y - tilePosition->y, m_selection.height);

				for (std::size_t y = 0; y < selectionHeight; ++y)
				{
					for (std::size_t x = 0; x < selectionWidth; ++x)
					{
						std::size_t tileDataIndex = m_selection.tiles[y * m_selection.width + x];

						assert(tileDataIndex < m_tiles.size());
						const auto& tileData = m_tiles[tileDataIndex];

						Nz::Vector2ui position(tilePosition->x + static_cast<unsigned int>(x), tilePosition->y + static_cast<unsigned int>(y));

						m_tileMap->EnableTile(position, tileData.texCoords, Nz::Color::White, tileData.materialIndex);

						std::size_t tileIndex = position.y * m_tilemapData.mapSize.x + position.x;

						assert(tileIndex < m_tilemapData.content.size());
						m_tilemapData.content[tileIndex] = static_cast<Nz::UInt32>(tileDataIndex + 1);
					}
				}

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

		Nz::Vector2f worldPos = canvas->GetCamera().Unproject(Nz::Vector2f(mouseX, mouseY));

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

	void TileMapEditorMode::UpdateSelection(std::size_t width, std::size_t height, const std::vector<TileSelectionWidget::TileSelection>& selectedTiles)
	{
		m_tileSelectionEntity->Disable();
		m_selection.width = width;
		m_selection.height = height;
		m_selection.tiles.clear();
		for (const auto& tileSelection : selectedTiles)
		{
			assert(tileSelection.groupIndex < m_tilesetGroups.size());
			std::size_t tileIndex = 0;
			for (std::size_t groupIndex = 0; groupIndex < tileSelection.groupIndex; ++groupIndex)
			{
				const auto& group = m_tilesetGroups[groupIndex];
				for (const auto& material : group.materials)
					tileIndex += material.tileCount.x * material.tileCount.y;
			}

			const auto& currentGroup = m_tilesetGroups[tileSelection.groupIndex];
			assert(tileSelection.materialIndex < currentGroup.materials.size());

			for (std::size_t matIndex = 0; matIndex < tileSelection.materialIndex; ++matIndex)
			{
				const auto& mat = currentGroup.materials[matIndex];
				tileIndex += mat.tileCount.x * mat.tileCount.y;
			}

			const auto& currentMaterial = currentGroup.materials[tileSelection.materialIndex];
			assert(tileSelection.tileIndex < currentMaterial.tileCount.x * currentMaterial.tileCount.y);

			tileIndex += tileSelection.tileIndex;
			assert(tileIndex < m_tiles.size());

			m_selection.tiles.push_back(tileIndex);
		}
	}
}
