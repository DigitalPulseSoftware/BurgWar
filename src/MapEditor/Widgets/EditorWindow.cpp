// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <ClientLib/Scripting/ClientEntityLibrary.hpp>
#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <MapEditor/Widgets/LayerInfoDialog.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <MapEditor/Widgets/MapInfoDialog.hpp>
#include <QtCore/QSettings>
#include <QtCore/QStringBuilder>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <tsl/hopscotch_set.h>

namespace bw
{
	namespace
	{
		constexpr std::size_t MaxRecentFiles = 4;
	}

	EditorWindow::EditorWindow() :
	BurgApp(LogSide::Editor),
	m_entityInfoDialog(nullptr)
	{
		RegisterEditorConfig();

		if (!m_config.LoadFromFile("editorconfig.lua"))
			throw std::runtime_error("Failed to load config file");

		const std::string& editorAssetsFolder = m_config.GetStringOption("Assets.EditorFolder");
		const std::string& gameResourceFolder = m_config.GetStringOption("Assets.ResourceFolder");
		const std::string& scriptFolder = m_config.GetStringOption("Assets.ScriptFolder");

		std::shared_ptr<VirtualDirectory> virtualDir = std::make_shared<VirtualDirectory>(scriptFolder);

		m_scriptingContext = std::make_shared<ScriptingContext>(GetLogger(), virtualDir);
		m_scriptingContext->LoadLibrary(std::make_shared<EditorScriptingLibrary>(GetLogger()));
		m_scriptingContext->GetLuaState()["Editor"] = this;

		m_assetStore.emplace(GetLogger(), std::make_shared<VirtualDirectory>(gameResourceFolder));

		m_entityStore.emplace(*m_assetStore, GetLogger(), m_scriptingContext);
		m_entityStore->LoadLibrary(std::make_shared<ClientElementLibrary>(GetLogger()));
		m_entityStore->LoadLibrary(std::make_shared<ClientEntityLibrary>(GetLogger(), *m_assetStore));

		VirtualDirectory::Entry entry;
		
		if (virtualDir->GetEntry("entities", &entry))
		{
			std::filesystem::path path = "entities";

			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				m_entityStore->LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), path / entryName);
			});
		}

		// Load some resources

		Nz::MaterialRef arrowMat = Nz::Material::New("Translucent2D");
		arrowMat->EnableDepthBuffer(false);
		arrowMat->SetDiffuseMap(editorAssetsFolder + "/arrow.png");

		Nz::MaterialLibrary::Register("GizmoArrow", arrowMat);

		Nz::ImageLibrary::Register("Eraser", Nz::Image::LoadFromFile(editorAssetsFolder + "/eraser.png"));

		Nz::MaterialRef selectionMaterial = Nz::Material::New("Translucent2D");
		selectionMaterial->SetDiffuseMap(editorAssetsFolder + "/tile_selection.png");

		Nz::MaterialLibrary::Register("TileSelection", selectionMaterial);

		// GUI
		m_recentMapActions.resize(MaxRecentFiles);

		for (QAction*& action : m_recentMapActions)
		{
			action = new QAction(this);
			action->setVisible(false);

			connect(action, &QAction::triggered, this, &EditorWindow::OnOpenRecentMap);
		}

		BuildMenu();

		m_canvas = new MapCanvas(*this);

		m_canvas->OnCameraZoomFactorUpdated.Connect([this](MapCanvas* /*emitter*/, float factor)
		{
			statusBar()->showMessage(tr("Zoom level: %1%").arg(static_cast<int>(std::round(factor * 100.f))));
		});

		m_canvas->OnDeleteEntity.Connect([this](MapCanvas* /*emitter*/, Ndk::EntityId canvasIndex)
		{
			auto it = m_entityIndexes.find(canvasIndex);
			assert(it != m_entityIndexes.end());

			OnDeleteEntity(it.value());
		});

		m_canvas->OnEntityPositionUpdated.Connect([this](MapCanvas* /*emitter*/, Ndk::EntityId canvasIndex, const Nz::Vector2f& newPosition)
		{
			assert(m_currentLayer.has_value());

			auto it = m_entityIndexes.find(canvasIndex);
			assert(it != m_entityIndexes.end());

			std::size_t entityIndex = it.value();

			auto& layer = m_workingMap.GetLayer(m_currentLayer.value());

			auto& layerEntity = layer.entities[entityIndex];
			layerEntity.position = newPosition;
		});

		m_canvas->OnCanvasMouseButtonPressed.Connect([this](MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			m_currentMode->OnMouseButtonPressed(mouseButton);
		});

		m_canvas->OnCanvasMouseButtonReleased.Connect([this](MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			m_currentMode->OnMouseButtonReleased(mouseButton);
		});

		m_canvas->OnCanvasMouseEntered.Connect([this](MapCanvas* /*emitter*/)
		{
			m_currentMode->OnMouseEntered();
		});

		m_canvas->OnCanvasMouseLeft.Connect([this](MapCanvas* /*emitter*/)
		{
			m_currentMode->OnMouseLeft();
		});

		m_canvas->OnCanvasMouseMoved.Connect([this](MapCanvas* /*emitter*/, const Nz::WindowEvent::MouseMoveEvent& mouseMove)
		{
			m_currentMode->OnMouseMoved(mouseMove);
		});

		setCentralWidget(m_canvas);

		QDockWidget* layerListDock = new QDockWidget("Layer list", this);

		m_layerList = new QListWidget;
		connect(m_layerList, &QListWidget::currentRowChanged, this, &EditorWindow::OnLayerChanged);
		connect(m_layerList, &QListWidget::itemDoubleClicked, this, &EditorWindow::OnLayerDoubleClicked);

		layerListDock->setWidget(m_layerList);

		addDockWidget(Qt::RightDockWidgetArea, layerListDock);

		QDockWidget* entityListDock = new QDockWidget("Layer entities", this);

		m_entityList = new QListWidget;
		connect(m_entityList, &QListWidget::itemDoubleClicked, this, &EditorWindow::OnEntityDoubleClicked);
		connect(m_entityList, &QListWidget::itemSelectionChanged, this, &EditorWindow::OnEntitySelectionUpdate);

		entityListDock->setWidget(m_entityList);

		addDockWidget(Qt::RightDockWidgetArea, entityListDock);

		QToolBar* toolBar = new QToolBar;
		QAction* createMap = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/gui/icons/file-48.png").c_str())), tr("Create map..."));
		connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

		QAction* openMap = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/gui/icons/opened_folder-48.png").c_str())), tr("Open map..."));
		connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

		m_saveMapToolbar = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/gui/icons/icons8-save-48.png").c_str())), tr("Save map..."));
		connect(m_saveMapToolbar, &QAction::triggered, this, &EditorWindow::OnSaveMap);

		toolBar->addSeparator();

		m_createEntityActionToolbar = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/gui/icons/idea-48.png").c_str())), tr("Create entity"));
		connect(m_createEntityActionToolbar, &QAction::triggered, this, &EditorWindow::OnCreateEntity);

		QDockWidget* toolbarDock = new QDockWidget("Toolbar", this);
		toolbarDock->setWidget(toolBar);

		addDockWidget(Qt::TopDockWidgetArea, toolbarDock);

		resize(1280, 720);
		setWindowTitle(tr("Burg'war map editor"));

		ClearWorkingMap();

		m_currentMode = std::make_shared<BasicEditorMode>(*this);
		m_currentMode->OnEnter();

		statusBar()->showMessage(tr("Ready"), 0);
	}

	EditorWindow::~EditorWindow()
	{
		m_currentMode->OnLeave();
		m_currentMode.reset();

		// Delete canvas before releasing everything else
		delete m_canvas;

		m_entityStore.reset();
	}

	void EditorWindow::ClearWorkingMap()
	{
		UpdateWorkingMap(Map());
	}

	void EditorWindow::SelectEntity(Ndk::EntityId entityId)
	{
		auto it = m_entityIndexes.find(entityId);
		assert(it != m_entityIndexes.end());

		std::size_t entityIndex = it.value();

		m_entityList->clearSelection();
		m_entityList->setItemSelected(m_entityList->item(int(entityIndex)), true);
	}

	void EditorWindow::SwitchToMode(std::shared_ptr<EditorMode> editorMode)
	{
		m_currentMode->OnLeave();
		m_currentMode = std::move(editorMode);
		m_currentMode->OnEnter();
	}

	void EditorWindow::UpdateWorkingMap(Map map, std::filesystem::path mapPath)
	{
		m_workingMap = std::move(map);
		m_workingMapPath = std::move(mapPath);

		setWindowFilePath(QString::fromStdString(mapPath.generic_u8string()));

		bool enableMapActions = m_workingMap.IsValid();

		m_compileMap->setEnabled(enableMapActions);
		m_createEntityActionToolbar->setEnabled(enableMapActions);
		m_mapMenu->setEnabled(enableMapActions);
		m_saveMap->setEnabled(enableMapActions);
		m_saveMapToolbar->setEnabled(enableMapActions);

		RefreshLayerList();

		if (m_layerList->count() > 0)
			m_layerList->setCurrentRow(0);
	}

	bool EditorWindow::event(QEvent* e)
	{
		switch (e->type())
		{
			case QEvent::KeyPress:
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
				if (keyEvent->key() == Qt::Key_Delete)
					OnDeleteEntity();
			}
		}

		return QMainWindow::event(e);
	}

	void EditorWindow::AddToRecentFileList(const QString& mapFolder)
	{
		QSettings settings;
		QStringList recentlyOpenedMaps = settings.value("recentFiles").toStringList();

		recentlyOpenedMaps.removeAll(mapFolder);
		recentlyOpenedMaps.prepend(mapFolder);
		while (recentlyOpenedMaps.size() > MaxRecentFiles)
			recentlyOpenedMaps.removeLast();

		settings.setValue("recentFiles", recentlyOpenedMaps);

		RefreshRecentFileListMenu(recentlyOpenedMaps);
	}

	void EditorWindow::BuildAssetList()
	{
		tsl::hopscotch_set<std::string> textures;

		std::size_t layerCount = m_workingMap.GetLayerCount();
		for (std::size_t i = 0; i < layerCount; ++i)
		{
			Map::Layer& layer = m_workingMap.GetLayer(i);

			for (const auto& entity : layer.entities)
			{
				if (std::size_t entityIndex = m_entityStore->GetElementIndex(entity.entityType); entityIndex != m_entityStore->InvalidIndex)
				{
					auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore->GetElement(entityIndex));

					for (const auto& [key, value] : entity.properties)
					{
						if (auto it = entityTypeInfo->properties.find(key); it != entityTypeInfo->properties.end())
						{
							const auto& propertyData = it->second;
							switch (propertyData.type)
							{
								case PropertyType::Texture:
								{
									if (propertyData.isArray)
									{
										for (const std::string& texture : std::get<EntityPropertyArray<std::string>>(value))
											textures.insert(texture);
									}
									else
									{
										textures.insert(std::get<std::string>(value));
									}
									break;
								}

								default:
									break;
							}
						}
					}
				}
				else
					bwLog(GetLogger(), LogLevel::Error, "Unknown entity type: {0}", entity.entityType);
			}
		}

		std::filesystem::path gameResourceFolder = m_config.GetStringOption("Assets.ResourceFolder");

		std::vector<Map::Asset>& assets = m_workingMap.GetAssets();
		assets.clear();

		auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);

		for (const std::string& texturePath : textures)
		{
			std::filesystem::path fullPath = gameResourceFolder / texturePath;

			auto& asset = assets.emplace_back();
			asset.filepath = texturePath;

			if (std::filesystem::is_regular_file(fullPath))
			{
				asset.size = std::filesystem::file_size(fullPath);

				Nz::ByteArray assetHash = Nz::File::ComputeHash(hash.get(), fullPath.generic_u8string());
				assert(assetHash.GetSize() == asset.sha1Checksum.size());

				std::memcpy(asset.sha1Checksum.data(), assetHash.GetConstBuffer(), assetHash.GetSize());
			}
			else
				bwLog(GetLogger(), LogLevel::Error, "Texture not found: {0}", fullPath.generic_u8string());
		}

		bwLog(GetLogger(), LogLevel::Info, "Finished building assets");
	}

	void EditorWindow::BuildMenu()
	{
		QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
		QAction* createMap = fileMenu->addAction(tr("Create map..."));
		createMap->setShortcut(QKeySequence::New);
		connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

		QAction* openMap = fileMenu->addAction(tr("Open map..."));
		createMap->setShortcut(QKeySequence::Open);
		connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

		QMenu* recentMaps = fileMenu->addMenu(tr("Open recent..."));
		for (QAction* action : m_recentMapActions)
			recentMaps->addAction(action);

		RefreshRecentFileListMenu();

		m_saveMap = fileMenu->addAction(tr("Save map..."));
		m_saveMap->setShortcut(QKeySequence::Save);
		connect(m_saveMap, &QAction::triggered, this, &EditorWindow::OnSaveMap);

		fileMenu->addSeparator();

		m_compileMap = fileMenu->addAction(tr("Compile map..."));
		connect(m_compileMap, &QAction::triggered, this, &EditorWindow::OnCompileMap);

		m_mapMenu = menuBar()->addMenu(tr("&Map"));
		QMenu* layerMenu = m_mapMenu->addMenu("Layers");
		QAction* addLayer = layerMenu->addAction(tr("Add layer"));
		connect(addLayer, &QAction::triggered, this, &EditorWindow::OnCreateLayer);

		QMenu* showMenu = menuBar()->addMenu(tr("&Show"));

		QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
		QAction* aboutQt = helpMenu->addAction(tr("About Qt..."));
		aboutQt->setMenuRole(QAction::AboutQtRole);
		connect(aboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
	}

	EntityInfoDialog* EditorWindow::GetEntityInfoDialog()
	{
		if (!m_entityInfoDialog)
			m_entityInfoDialog = new EntityInfoDialog(GetLogger(), *m_entityStore, *m_scriptingContext, this);

		return m_entityInfoDialog;
	}

	void EditorWindow::RefreshRecentFileListMenu()
	{
		QSettings settings;
		QStringList recentMaps = settings.value("recentFiles").toStringList();

		RefreshRecentFileListMenu(recentMaps);
	}

	void EditorWindow::RefreshRecentFileListMenu(const QStringList& recentFileList)
	{
		std::size_t fileCount = std::min<std::size_t>(m_recentMapActions.size(), recentFileList.size());
		for (std::size_t i = 0; i < fileCount; ++i)
		{
			QString strippedName = QFileInfo(recentFileList[int(i)]).fileName();

			QAction* recentMap = m_recentMapActions[int(i)];
			recentMap->setData(recentFileList[int(i)]);
			recentMap->setText(tr("&%1 %2").arg(int(i + 1)).arg(strippedName));
			recentMap->setVisible(true);
		}

		for (std::size_t i = fileCount; i < m_recentMapActions.size(); ++i)
			m_recentMapActions[int(i)]->setVisible(false);
	}

	void EditorWindow::OnCompileMap()
	{
		QString filter("*.bmap");
		QString fileName = QFileDialog::getSaveFileName(this, tr("Where to save compiled map file"), QString(), filter, &filter);
		if (fileName.isEmpty())
			return;

		if (!fileName.endsWith(".bmap"))
			fileName += ".bmap";

		BuildAssetList();

		if (m_workingMap.Compile(fileName.toStdString()))
			QMessageBox::information(this, tr("Compilation succeeded"), tr("Map has been successfully compiled"), QMessageBox::Ok);
		else
			QMessageBox::critical(this, tr("Failed to compile map"), tr("Map failed to compile"), QMessageBox::Ok);
	}

	void EditorWindow::OnCreateEntity()
	{
		std::size_t layerIndex = static_cast<std::size_t>(m_layerList->currentRow());

		EntityInfoDialog* createEntityDialog = GetEntityInfoDialog();
		createEntityDialog->Open(std::nullopt, Ndk::EntityHandle::InvalidHandle, [this, layerIndex](EntityInfoDialog* createEntityDialog)
		{
			const EntityInfo& entityInfo = createEntityDialog->GetInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);
			
			std::size_t entityIndex = layer.entities.size();
			auto& layerEntity = layer.entities.emplace_back();
			layerEntity.entityType = entityInfo.entityClass;
			layerEntity.name = entityInfo.entityName;
			layerEntity.position = entityInfo.position;
			layerEntity.properties = entityInfo.properties;
			layerEntity.rotation = entityInfo.rotation;

			RegisterEntity(entityIndex);
		});

		createEntityDialog->exec();
	}

	void EditorWindow::OnCreateMap()
	{
		MapInfoDialog* createMapDialog = new MapInfoDialog(this);
		connect(createMapDialog, &QDialog::accepted, [this, createMapDialog]()
		{
			MapInfo mapInfo = createMapDialog->GetMapInfo();

			UpdateWorkingMap(Map(mapInfo));
		});
		createMapDialog->exec();
	}

	void EditorWindow::OnCreateLayer()
	{
		if (!m_workingMap.IsValid())
			return;

		auto& layer = m_workingMap.AddLayer();
		layer.name = "Layer #" + std::to_string(m_workingMap.GetLayerCount());

		RefreshLayerList();
	}

	void EditorWindow::OnDeleteEntity()
	{
		QList<QListWidgetItem*> items = m_entityList->selectedItems();
		if (!items.empty())
		{
			assert(items.size() == 1);

			QListWidgetItem* item = items.front();
			std::size_t entityIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());

			OnDeleteEntity(entityIndex);

			m_entityList->clearSelection();
		}
	}

	void EditorWindow::OnDeleteEntity(std::size_t entityIndex)
	{
		auto& layer = m_workingMap.GetLayer(m_currentLayer.value());

		auto& layerEntity = layer.entities[entityIndex];

		QString warningText = tr("You are about to delete entity %1 of type %2, are you sure you want to do that?").arg(QString::fromStdString(layerEntity.name)).arg(QString::fromStdString(layerEntity.entityType));
		QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Are you sure?"), warningText, QMessageBox::Yes | QMessageBox::Cancel);
		if (response == QMessageBox::Yes)
		{
			QListWidgetItem* item = m_entityList->takeItem(int(entityIndex));
			Ndk::EntityId canvasId = item->data(Qt::UserRole + 1).value<Ndk::EntityId>();

			delete item;

			m_canvas->DeleteEntity(canvasId);

			m_entityIndexes.erase(canvasId);

			layer.entities.erase(layer.entities.begin() + entityIndex);

			// FIXME...
			for (auto it = m_entityIndexes.begin(); it != m_entityIndexes.end(); ++it)
			{
				if (it->second >= entityIndex)
				{
					std::size_t newEntityIndex = --it.value();
					m_entityList->item(int(newEntityIndex))->setData(Qt::UserRole, qulonglong(newEntityIndex));
				}
			}
		}
	}

	void EditorWindow::OnEntityDoubleClicked(QListWidgetItem* item)
	{
		if (!item)
			return;

		std::size_t entityIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());
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

		const auto& entity = m_canvas->GetWorld().GetEntity(canvasId);

		EntityInfoDialog* editEntityDialog = GetEntityInfoDialog();
		editEntityDialog->Open(std::move(entityInfo), entity, [this, entityIndex, layerIndex, item, canvasId](EntityInfoDialog* editEntityDialog)
		{
			const EntityInfo& entityInfo = editEntityDialog->GetInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);

			auto& layerEntity = layer.entities[entityIndex];
			layerEntity.entityType = entityInfo.entityClass;
			layerEntity.position = entityInfo.position;
			layerEntity.properties = entityInfo.properties;
			layerEntity.rotation = entityInfo.rotation;

			// TODO: Recreate entity only if properties/class updated

			//m_canvas->UpdateEntityPositionAndRotation(canvasId, layerEntity.position, layerEntity.rotation);
			m_canvas->DeleteEntity(canvasId);
			m_entityIndexes.erase(canvasId);

			Ndk::EntityId newCanvasId = m_canvas->CreateEntity(layerEntity.entityType, layerEntity.position, layerEntity.rotation, layerEntity.properties)->GetId();
			m_entityIndexes.emplace(newCanvasId, entityIndex);
			item->setData(Qt::UserRole + 1, newCanvasId);

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

			// Trigger selection signal
			if (item->isSelected())
			{
				item->setSelected(false);
				item->setSelected(true);
			}
		});

		editEntityDialog->exec();
	}

	void EditorWindow::OnEntitySelectionUpdate()
	{
		QList<QListWidgetItem*> items = m_entityList->selectedItems();
		if (!items.empty())
		{
			assert(items.size() == 1);

			QListWidgetItem* item = items.front();

			std::size_t entityIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());
			Ndk::EntityId canvasId = item->data(Qt::UserRole + 1).value<Ndk::EntityId>();
			std::size_t layerIndex = static_cast<std::size_t>(m_layerList->currentRow());

			m_canvas->EditEntityPosition(canvasId);
		}
		else
			m_canvas->ClearEntitySelection();
	}

	void EditorWindow::OnLayerChanged(int layerIndex)
	{
		if (layerIndex == -1)
		{
			m_currentLayer.reset();
			m_entityIndexes.clear();
			return;
		}

		assert(layerIndex >= 0);
		std::size_t layerIdx = static_cast<std::size_t>(layerIndex);

		m_currentLayer = layerIndex;

		assert(layerIdx < m_workingMap.GetLayerCount());
		auto& layer = m_workingMap.GetLayer(layerIdx);

		m_entityList->clear();

		m_canvas->UpdateBackgroundColor(layer.backgroundColor);

		m_canvas->ClearEntities();
		m_entityIndexes.clear();

		for (std::size_t entityIndex = 0; entityIndex < layer.entities.size(); ++entityIndex)
			RegisterEntity(entityIndex);
	}

	void EditorWindow::OnLayerDoubleClicked(QListWidgetItem* item)
	{
		std::size_t layerIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());

		auto& layer = m_workingMap.GetLayer(layerIndex);

		LayerInfo layerInfo;
		layerInfo.backgroundColor = layer.backgroundColor;
		layerInfo.depth = layer.depth;
		layerInfo.name = layer.name;

		LayerInfoDialog* layerInfoDialog = new LayerInfoDialog(layerInfo, this);
		connect(layerInfoDialog, &QDialog::accepted, [this, layerInfoDialog, layerIndex, item]()
		{
			LayerInfo layerInfo = layerInfoDialog->GetLayerInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);

			layer.backgroundColor = layerInfo.backgroundColor;
			m_canvas->UpdateBackgroundColor(layer.backgroundColor);

			layer.depth = layerInfo.depth;

			bool resetItemName = false;
			if (layer.name != layerInfo.name)
			{
				layer.name = layerInfo.name;
				resetItemName = true;
			}

			if (resetItemName)
				item->setText(QString::fromStdString(layer.name));
		});

		layerInfoDialog->exec();
	}

	void EditorWindow::OnOpenMap()
	{
		QString mapFolder = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly);
		if (mapFolder.isEmpty())
			return;

		OpenMap(mapFolder);
	}

	void EditorWindow::OnOpenRecentMap()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
			OpenMap(action->data().toString());
	}

	void EditorWindow::OnSaveMap()
	{
		if (m_workingMapPath.empty())
		{
			QDir mapFolder;
			QString workingPath;

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

				workingPath = mapFolder.path();
				break;
			}

			if (!mapFolder.mkdir("assets"))
				QMessageBox::warning(this, tr("Failed to create folder"), tr("Failed to create assets subdirectory (is map folder read-only?)"), QMessageBox::Ok);

			if (!mapFolder.mkdir("scripts"))
				QMessageBox::warning(this, tr("Failed to create folder"), tr("Failed to create scripts subdirectory (is map folder read-only?)"), QMessageBox::Ok);

			m_workingMapPath = workingPath.toStdString();

			AddToRecentFileList(workingPath);
		}

		if (m_workingMap.Save(m_workingMapPath))
			statusBar()->showMessage(tr("Map saved"), 3000);
		else
		{
			QMessageBox::warning(this, tr("Failed to save map"), tr("Failed to save map (is map folder read-only?)"), QMessageBox::Ok);
			statusBar()->showMessage(tr("Failed to save map"), 5000);
		}
	}

	void EditorWindow::OpenMap(const QString& mapFolder)
	{
		Map map;

		std::filesystem::path workingMapPath = mapFolder.toStdString();
		try
		{
			map = Map::LoadFromFolder(workingMapPath);
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Failed to open map"), tr("Failed to open map: %1").arg(e.what()), QMessageBox::Ok);
			return;
		}

		statusBar()->showMessage(tr("Map %1 loaded").arg(map.GetMapInfo().name.data()), 3000);
		UpdateWorkingMap(std::move(map), std::move(workingMapPath));

		AddToRecentFileList(mapFolder);
	}

	void EditorWindow::RegisterEditorConfig()
	{
		m_config.RegisterStringOption("Assets.EditorFolder");
	}

	void EditorWindow::RegisterEntity(std::size_t entityIndex)
	{
		assert(m_currentLayer.has_value());

		auto& layer = m_workingMap.GetLayer(*m_currentLayer);

		assert(entityIndex < layer.entities.size());
		const Map::Entity& entity = layer.entities[entityIndex];

		QString entryName = QString::fromStdString(entity.entityType);
		if (!entity.name.empty())
			entryName = entryName % " (" % QString::fromStdString(entity.name) % ")";

		QListWidgetItem* item = new QListWidgetItem(entryName);
		item->setData(Qt::UserRole, qulonglong(entityIndex));

		Ndk::EntityId canvasId = m_canvas->CreateEntity(entity.entityType, entity.position, entity.rotation, entity.properties)->GetId();
		item->setData(Qt::UserRole + 1, canvasId);

		m_entityList->addItem(item);

		m_entityIndexes.emplace(canvasId, entityIndex);
	}

	void EditorWindow::RefreshLayerList()
	{
		m_layerList->clear();
		m_layerList->clearSelection();

		for (std::size_t layerIndex = 0; layerIndex < m_workingMap.GetLayerCount(); ++layerIndex)
		{
			const auto& layer = m_workingMap.GetLayer(layerIndex);

			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(layer.name));
			item->setData(Qt::UserRole, qulonglong(layerIndex));

			m_layerList->addItem(item);
		}
	}
}
