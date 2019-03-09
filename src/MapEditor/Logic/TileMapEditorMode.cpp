// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtWidgets/QDockWidget>
#include <iostream>

namespace bw
{
	TileMapEditorMode::TileMapEditorMode(const Ndk::EntityHandle& targetEntity, const Nz::Vector2f& origin, const Nz::DegreeAnglef& rotation, 
	                                     const Nz::Vector2ui& mapSize, const Nz::Vector2f& tileSize, std::vector<Nz::UInt8> content) :
	EntityEditorMode(targetEntity),
	m_tilemapContent(std::move(content)),
	m_rotation(rotation),
	m_origin(origin),
	m_tileSize(tileSize),
	m_mapSize(mapSize),
	m_editionMode(EditionMode::None)
	{
	}

	void TileMapEditorMode::OnEnter(EditorWindow& editor)
	{
		EntityEditorMode::OnEnter(editor);

		QDockWidget* tileSelectorDock = new QDockWidget(QObject::tr("Tile selector"), &editor);
		tileSelectorDock->setFloating(true);

		TileSelectionWidget* tileWidget = new TileSelectionWidget;
		tileWidget->resize(256, 256); //< FIXME: This is ignored for some reason

		tileSelectorDock->setWidget(tileWidget);

		editor.addDockWidget(Qt::LeftDockWidgetArea, tileSelectorDock);

		MapCanvas* mapCanvas = editor.GetMapCanvas();

		m_tilemapEntity = mapCanvas->GetWorld().CreateEntity();

		Nz::MaterialRef tileMapMat = Nz::Material::New("Translucent2D");
		tileMapMat->SetDiffuseMap("../resources/dirt.png");

		m_tileMap = Nz::TileMap::New(m_mapSize, m_tileSize);
		m_tileMap->SetMaterial(0, tileMapMat);

		for (std::size_t i = 0; i < m_mapSize.x * m_mapSize.y; ++i)
		{
			unsigned int value = m_tilemapContent[i];

			Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % m_mapSize.x), static_cast<unsigned int>(i / m_mapSize.x) };
			if (value > 0)
				m_tileMap->EnableTile(tilePos, Nz::Rectf(0.f, 0.f, 1.f, 1.f));
		}

		auto& tileMapNode = m_tilemapEntity->AddComponent<Ndk::NodeComponent>();
		tileMapNode.SetPosition(m_origin);
		tileMapNode.SetRotation(m_rotation);

		auto& tileMapGraphics = m_tilemapEntity->AddComponent<Ndk::GraphicsComponent>();
		tileMapGraphics.Attach(m_tileMap, 1);

		m_tilePreviewEntity = mapCanvas->GetWorld().CreateEntity();

		Nz::MaterialRef tileMat = Nz::Material::New(*tileMapMat);
		tileMat->SetDiffuseColor(Nz::Color(255, 255, 255, 200));

		m_tileSprite = Nz::Sprite::New();
		m_tileSprite->SetMaterial(tileMat);
		m_tileSprite->SetSize(m_tileSize);

		m_tilePreviewEntity->AddComponent<Ndk::NodeComponent>();

		auto& spriteTexture = m_tilePreviewEntity->AddComponent<Ndk::GraphicsComponent>();
		spriteTexture.Attach(m_tileSprite, 2);
	}

	void TileMapEditorMode::OnLeave(EditorWindow& editor)
	{
		EntityEditorMode::OnLeave(editor);

		m_tileMap.Reset();
		m_tileSprite.Reset();
		m_tilemapEntity.Reset();
	}

	void TileMapEditorMode::OnMouseButtonPressed(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		switch (mouseButton.button)
		{
			case Nz::Mouse::Left:
				m_editionMode = EditionMode::EnableTile;
				break;

			case Nz::Mouse::Right:
				m_editionMode = EditionMode::DisableTile;
				break;

			default:
				break;
		}
	}

	void TileMapEditorMode::OnMouseButtonReleased(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		Nz::Mouse::Button modeButton;

		switch (m_editionMode)
		{
			case EditionMode::DisableTile:
				modeButton = Nz::Mouse::Right;
				break;

			case EditionMode::EnableTile:
				modeButton = Nz::Mouse::Left;
				break;

			case EditionMode::None:
			default:
				return;
		}

		if (mouseButton.button != modeButton)
			return;

		m_editionMode = EditionMode::None;
	}

	void TileMapEditorMode::OnMouseMoved(EditorWindow& editor, const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		MapCanvas* canvas = editor.GetMapCanvas();

		std::optional<Nz::Vector2ui> tilePosition = GetTilePositionFromMouse(editor, mouseMoved.x, mouseMoved.y);
		if (tilePosition)
		{
			auto& node = m_tilePreviewEntity->GetComponent<Ndk::NodeComponent>();
			node.SetPosition(Nz::Vector2f(*tilePosition) * m_tileSize + m_origin);

			switch (m_editionMode)
			{
				case EditionMode::DisableTile:
				{
					m_tileMap->DisableTile(*tilePosition);

					std::size_t tileIndex = tilePosition->y * m_mapSize.x + tilePosition->x;

					assert(tileIndex < m_tilemapContent.size());
					m_tilemapContent[tileIndex] = 0;
					break;
				}

				case EditionMode::EnableTile:
				{
					m_tileMap->EnableTile(*tilePosition, Nz::Rectf(0.f, 0.f, 1.f, 1.f));

					std::size_t tileIndex = tilePosition->y * m_mapSize.x + tilePosition->x;

					assert(tileIndex < m_tilemapContent.size());
					m_tilemapContent[tileIndex] = 1;
					break;
				}

				case EditionMode::None:
				default:
					break;
			}
		}
		else
			m_tilePreviewEntity->Disable();
	}

	std::optional<Nz::Vector2ui> TileMapEditorMode::GetTilePositionFromMouse(EditorWindow& editor, int mouseX, int mouseY) const
	{
		MapCanvas* canvas = editor.GetMapCanvas();

		auto& cameraComponent = canvas->GetCameraEntity()->GetComponent<Ndk::CameraComponent>();
		Nz::Vector2f worldPos = Nz::Vector2f(cameraComponent.Unproject(Nz::Vector3f(mouseX, mouseY, 0.f)));

		Nz::Rectf tilemapRect(m_origin.x, m_origin.y, m_mapSize.x * m_tileSize.x, m_mapSize.y * m_tileSize.y);
		if (tilemapRect.Contains(worldPos))
		{
			m_tilePreviewEntity->Enable();

			Nz::Vector2f relativePosition = worldPos - m_origin;
			relativePosition.x = std::floor(relativePosition.x / m_tileSize.x);
			relativePosition.y = std::floor(relativePosition.y / m_tileSize.y);

			return Nz::Vector2ui(relativePosition);
		}
		else
			return std::nullopt;
	}
}
