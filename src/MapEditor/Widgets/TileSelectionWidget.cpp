// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/TileSelectionWidget.hpp>
#include <MapEditor/Widgets/ScrollCanvas.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <iostream>

namespace bw
{
	TileSelectionWidget::TileSelectionWidget(QWidget* parent) :
	QWidget(parent)
	{
		setWindowTitle(tr("Tile selector"));

		m_tileSelectionCanvas = new ScrollCanvas;

		Nz::SpriteRef tiles = Nz::Sprite::New();
		tiles->SetTexture("../resources/terrain_atlas.png");

		const Ndk::EntityHandle& atlasEntity = m_tileSelectionCanvas->GetWorld().CreateEntity();
		atlasEntity->AddComponent<Ndk::NodeComponent>();
		atlasEntity->AddComponent<Ndk::GraphicsComponent>().Attach(tiles);

		m_tileSelectionCanvas->SetContentSize(Nz::Vector2i(tiles->GetSize()));

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(new QPushButton("Clear mode"));
		buttonLayout->addWidget(new QPushButton("Tile mode"));

		QVBoxLayout* layout = new QVBoxLayout;
		layout->addLayout(buttonLayout);
		layout->addWidget(m_tileSelectionCanvas);

		setLayout(layout);
	}
}