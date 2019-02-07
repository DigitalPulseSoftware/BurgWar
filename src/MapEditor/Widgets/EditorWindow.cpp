// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <MapEditor/Widgets/MapWidget.hpp>
#include <MapEditor/Widgets/MapInfoDialog.hpp>
#include <QtCore/QStringBuilder>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <iostream>

namespace bw
{
	EditorWindow::EditorWindow()
	{
		BuildMenu();

		m_canvas = new MapWidget;
		setCentralWidget(m_canvas);

		QDockWidget* layerListDock = new QDockWidget("Layer list", this);

		m_layerList = new QListWidget;
		connect(m_layerList, &QListWidget::currentRowChanged, this, &EditorWindow::OnLayerChanged);

		layerListDock->setWidget(m_layerList);

		addDockWidget(Qt::RightDockWidgetArea, layerListDock);

		QDockWidget* entityListDock = new QDockWidget("Layer entities", this);

		m_entityList = new QListWidget;
		connect(m_entityList, &QListWidget::currentItemChanged, this, &EditorWindow::OnEntitySelected);
		connect(m_entityList, &QListWidget::itemDoubleClicked, this, &EditorWindow::OnEntityDoubleClicked);

		entityListDock->setWidget(m_entityList);

		addDockWidget(Qt::RightDockWidgetArea, entityListDock);

		QToolBar* toolBar = new QToolBar;
		QAction* createMap = toolBar->addAction(QIcon(QPixmap("../resources/gui/icons/file-48.png")), tr("Create map..."));
		connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

		QAction* openMap =  toolBar->addAction(QIcon(QPixmap("../resources/gui/icons/opened_folder-48.png")), tr("Open map..."));
		connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

		m_saveMapToolbar = toolBar->addAction(QIcon(QPixmap("../resources/gui/icons/icons8-save-48.png")), tr("Save map..."));
		connect(m_saveMapToolbar, &QAction::triggered, this, &EditorWindow::OnSaveMap);

		toolBar->addSeparator();

		m_createEntityActionToolbar = toolBar->addAction(QIcon(QPixmap("../resources/gui/icons/idea-48.png")), tr("Create entity"));
		connect(m_createEntityActionToolbar, &QAction::triggered, this, &EditorWindow::OnCreateEntity);

		QDockWidget* toolbarDock = new QDockWidget("Toolbar", this);
		toolbarDock->setWidget(toolBar);

		addDockWidget(Qt::TopDockWidgetArea, toolbarDock);

		resize(1280, 720);
		setWindowTitle(tr("Burg'war map editor"));

		statusBar()->showMessage(tr("Ready"), 0);

		ClearWorkingMap();
	}

	void EditorWindow::ClearWorkingMap()
	{
		UpdateWorkingMap(Map());
	}

	void EditorWindow::UpdateWorkingMap(Map map, std::filesystem::path mapPath)
	{
		m_workingMap = std::move(map);
		m_workingMapPath = std::move(mapPath);

		bool enableMapActions = m_workingMap.IsValid();

		m_createEntityActionToolbar->setEnabled(enableMapActions);
		m_saveMap->setEnabled(enableMapActions);
		m_saveMapToolbar->setEnabled(enableMapActions);

		if (m_workingMap.IsValid())
		{
			m_layerList->clear();
			for (std::size_t i = 0; i < m_workingMap.GetLayerCount(); ++i)
			{
				const auto& layer = m_workingMap.GetLayer(i);
				m_layerList->addItem(QString(layer.name.data()));
			}
		}
	}

	void EditorWindow::BuildMenu()
	{
		QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
		QAction* createMap = fileMenu->addAction(tr("Create map..."));
		connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

		QAction* openMap = fileMenu->addAction(tr("Open map..."));
		connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

		m_saveMap = fileMenu->addAction(tr("Save map..."));
		connect(m_saveMap, &QAction::triggered, this, &EditorWindow::OnSaveMap);

		QAction* compileMap = fileMenu->addAction(tr("Compile map..."));
		connect(compileMap, &QAction::triggered, this, &EditorWindow::OnCompileMap);

		QMenu* mapMenu = menuBar()->addMenu(tr("&Map"));

		QMenu* showMenu = menuBar()->addMenu(tr("&Show"));

		QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
		QAction* aboutQt = helpMenu->addAction(tr("About Qt..."));
		aboutQt->setMenuRole(QAction::AboutQtRole);
		connect(aboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
	}

	void EditorWindow::OnCompileMap()
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Where to save compiled map file"), QString(), QString(".bmap"));
		if (fileName.isEmpty())
			return;

		if (!fileName.endsWith(".bmap"))
			fileName += ".bmap";

		if (m_workingMap.Compile(fileName.toStdString()))
			QMessageBox::information(this, tr("Compilation succeeded"), tr("Map has been successfully compiled"), QMessageBox::Ok);
		else
			QMessageBox::critical(this, tr("Failed to compile map"), tr("Map failed to compile"), QMessageBox::Ok);
	}

	void EditorWindow::OnCreateEntity()
	{
		std::size_t layerIndex = static_cast<std::size_t>(m_layerList->currentRow());

		EntityInfoDialog* createEntityDialog = new EntityInfoDialog(this);
		connect(createEntityDialog, &QDialog::finished, [this, createEntityDialog, layerIndex](int code)
		{
			if (code != QDialog::Accepted)
				return;

			const EntityInfo& entityInfo = createEntityDialog->GetEntityInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);
			
			auto& layerEntity = layer.entities.emplace_back();
			layerEntity.entityType = entityInfo.entityClass;
			layerEntity.position = entityInfo.position;
			layerEntity.properties = entityInfo.properties;
			layerEntity.rotation = entityInfo.rotation;
		});

		createEntityDialog->exec();
	}

	void EditorWindow::OnCreateMap()
	{
		MapInfoDialog* createMapDialog = new MapInfoDialog(this);
		connect(createMapDialog, &QDialog::finished, [this, createMapDialog](int code)
		{
			if (code != QDialog::Accepted)
				return;

			MapInfo mapInfo = createMapDialog->GetMapInfo();

			QDir mapFolder;
			Map map;
			std::filesystem::path workingPath;

			for (;;)
			{
				QString path = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly);
				if (path.isEmpty())
					return;

				mapFolder = path;
				if (!mapFolder.isEmpty())
				{
					QMessageBox::critical(this, tr("Folder not empty"), tr("Map folder must be empty"), QMessageBox::Ok);
					continue;
				}

				workingPath = mapFolder.path().toStdString();

				map = Map(mapInfo);
				if (!map.Save(workingPath))
				{
					QMessageBox::critical(this, tr("Failed to create file"), tr("Failed to create map files (is map folder read-only?)"), QMessageBox::Ok);
					continue;
				}

				break;
			}

			if (!mapFolder.mkdir("assets"))
				QMessageBox::warning(this, tr("Failed to create folder"), tr("Failed to create assets subdirectory (is map folder read-only?)"), QMessageBox::Ok);

			if (!mapFolder.mkdir("scripts"))
				QMessageBox::warning(this, tr("Failed to create folder"), tr("Failed to create scripts subdirectory (is map folder read-only?)"), QMessageBox::Ok);

			UpdateWorkingMap(std::move(map), std::move(workingPath));
		});
		createMapDialog->exec();
	}

	void EditorWindow::OnEntityDoubleClicked(QListWidgetItem* item)
	{
		if (!item)
			return;

		std::size_t entityIndex = item->data(Qt::UserRole).value<std::size_t>();
		Ndk::EntityId canvasId = item->data(Qt::UserRole + 1).value<Ndk::EntityId>();
		std::size_t layerIndex = static_cast<std::size_t>(m_layerList->currentRow());

		auto& layer = m_workingMap.GetLayer(layerIndex);

		auto& layerEntity = layer.entities[entityIndex];

		EntityInfo entityInfo;
		entityInfo.entityClass = layerEntity.entityType;
		entityInfo.entityName = layerEntity.name;
		entityInfo.position = layerEntity.position;
		entityInfo.properties = layerEntity.properties;
		entityInfo.rotation = layerEntity.rotation;

		EntityInfoDialog* editEntityDialog = new EntityInfoDialog(std::move(entityInfo), this);
		connect(editEntityDialog, &QDialog::finished, [this, editEntityDialog, entityIndex, layerIndex, item, canvasId](int code)
		{
			if (code != QDialog::Accepted)
				return;

			const EntityInfo& entityInfo = editEntityDialog->GetEntityInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);

			auto& layerEntity = layer.entities[entityIndex];
			layerEntity.entityType = entityInfo.entityClass;
			layerEntity.position = entityInfo.position;
			layerEntity.properties = entityInfo.properties;
			layerEntity.rotation = entityInfo.rotation;

			m_canvas->UpdateEntityPositionAndRotation(canvasId, layerEntity.position, layerEntity.rotation);

			bool resetItemName = false;
			if (layerEntity.entityType != entityInfo.entityClass)
			{
				layerEntity.entityType = entityInfo.entityClass;
				resetItemName = true;
			}

			if (layerEntity.name != entityInfo.entityName)
			{
				layerEntity.name = entityInfo.entityName;
				resetItemName = true;
			}

			if (resetItemName)
			{
				QString entryName = QString::fromStdString(layerEntity.entityType);
				if (!layerEntity.name.empty())
					entryName = entryName % " (" % QString::fromStdString(layerEntity.name) % ")";

				item->setText(entryName);
			}
		});

		editEntityDialog->exec();
	}

	void EditorWindow::OnEntitySelected(QListWidgetItem* item)
	{
		if (!item)
			return;

		std::size_t entityIndex = item->data(Qt::UserRole).value<std::size_t>();
		Ndk::EntityId canvasId = item->data(Qt::UserRole + 1).value<Ndk::EntityId>();
		std::size_t layerIndex = static_cast<std::size_t>(m_layerList->currentRow());

		m_canvas->EditEntityPosition(canvasId);
	}

	void EditorWindow::OnLayerChanged(int layerIndex)
	{
		if (layerIndex == -1)
			return;

		assert(layerIndex >= 0);
		std::size_t layer = static_cast<std::size_t>(layerIndex);

		assert(layer < m_workingMap.GetLayerCount());

		m_entityList->clear();

		m_canvas->ClearEntities();

		std::size_t entityIndex = 0;
		for (const Map::Entity& entity : m_workingMap.GetLayer(layer).entities)
		{
			QString entryName = QString::fromStdString(entity.entityType);
			if (!entity.name.empty())
				entryName = entryName % " (" % QString::fromStdString(entity.name) % ")";

			QListWidgetItem* item = new QListWidgetItem(entryName);
			item->setData(Qt::UserRole, entityIndex++);

			Ndk::EntityId canvasId = m_canvas->CreateEntity(entity.position, entity.rotation);
			item->setData(Qt::UserRole + 1, canvasId);

			m_entityList->addItem(item);
		}
	}

	void EditorWindow::OnOpenMap()
	{
		QString mapFolder = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly);
		if (mapFolder.isEmpty())
			return;

		std::filesystem::path workingMapPath = mapFolder.toStdString();

		Map map;

		try
		{
			map = Map(workingMapPath);
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Failed to open map"), tr("Failed to open map: %1").arg(e.what()), QMessageBox::Ok);
			return;
		}

		statusBar()->showMessage(tr("Map %1 loaded").arg(map.GetMapInfo().name.data()), 3000);
		UpdateWorkingMap(std::move(map), std::move(workingMapPath));
	}

	void EditorWindow::OnSaveMap()
	{
		if (!m_workingMap.Save(m_workingMapPath))
			QMessageBox::warning(this, tr("Failed to save map"), tr("Failed to save map (is map folder read-only?)"), QMessageBox::Ok);
		
		statusBar()->showMessage(tr("Map saved"), 3000);
	}
}
