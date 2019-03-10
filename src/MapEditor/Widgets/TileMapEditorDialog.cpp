// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/TileMapEditorDialog.hpp>
#include <MapEditor/Widgets/ScrollCanvas.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <iostream>

namespace bw
{
	TileMapEditorDialog::TileMapEditorDialog(const Nz::Vector2ui& mapSize, const Nz::Vector2f& tileSize, const std::vector<Nz::UInt32>& content, QWidget* parent) :
	QDialog(parent)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		int maxWidth = int(std::ceil(mapSize.x * tileSize.x));
		int maxHeight = int(std::ceil(mapSize.y * tileSize.y));

		m_tileSelectionCanvas = new ScrollCanvas;

		Nz::SpriteRef tiles = Nz::Sprite::New();
		tiles->SetTexture("../resources/terrain_atlas.png");

		const Ndk::EntityHandle& atlasEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		atlasEntity->AddComponent<Ndk::NodeComponent>();
		atlasEntity->AddComponent<Ndk::GraphicsComponent>().Attach(tiles);

		m_tileSelectionCanvas->SetContentSize(Nz::Vector2i(tiles->GetSize()));

		m_tilemapCanvas = new ScrollCanvas;
		m_tilemapCanvas->SetContentSize({ maxWidth, maxHeight });

		Nz::MaterialRef tileMapMat = Nz::Material::New("Translucent2D");
		tileMapMat->SetDiffuseMap("../resources/dirt.png");

		m_tileMap = Nz::TileMap::New(mapSize, tileSize);
		m_tileMap->SetMaterial(0, tileMapMat);

		for (std::size_t i = 0; i < mapSize.x * mapSize.y; ++i)
		{
			unsigned int value = content[i];

			Nz::Vector2ui tilePos = { static_cast<unsigned int>(i % mapSize.x), static_cast<unsigned int>(i / mapSize.x) };
			if (value > 0)
				m_tileMap->EnableTile(tilePos, Nz::Rectf(0.f, 0.f, 1.f, 1.f));
		}

		const Ndk::EntityHandle& tileMapEntity = m_tilemapCanvas->GetWorld().CreateEntity();
		tileMapEntity->AddComponent<Ndk::NodeComponent>();
		tileMapEntity->AddComponent<Ndk::GraphicsComponent>().Attach(m_tileMap);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(new QPushButton("Clear mode"));
		buttonLayout->addWidget(new QPushButton("Tile mode"));

		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(m_tilemapCanvas);
		layout->addLayout(buttonLayout);
		layout->addWidget(m_tileSelectionCanvas);

		setLayout(layout);
	}
}