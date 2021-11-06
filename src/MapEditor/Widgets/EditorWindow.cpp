// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindow.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Components/ClientMatchComponent.hpp>
#include <ClientLib/Components/SoundEmitterComponent.hpp>
#include <MapEditor/Commands/EntityCommands.hpp>
#include <MapEditor/Commands/MapCommands.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Logic/TileMapEditorMode.hpp>
#include <MapEditor/Widgets/AlignmentDialog.hpp>
#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <MapEditor/Widgets/LayerEditDialog.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <MapEditor/Widgets/FileDescDialog.hpp>
#include <MapEditor/Widgets/PlayWindow.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <QtCore/QSettings>
#include <QtCore/QStringBuilder>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCommonStyle>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <tsl/bhopscotch_map.h>
#include <tsl/hopscotch_set.h>

namespace bw
{
	namespace
	{
		constexpr std::size_t MaxRecentFiles = 4;
		const char* settings_recentFiles = "recentFiles";
		const char* settings_showBackgroundColor = "showLayerBackgroundColor";
		const char* settings_showColliders = "showColliders";
		const char* settings_showGrid = "showGrid";
	}

	EditorWindow::EditorWindow(int argc, char* argv[]) :
	ClientEditorApp(argc, argv, LogSide::Editor, m_configFile),
	m_entityInfoDialog(nullptr),
	m_canvas(nullptr),
	m_playWindow(nullptr),
	m_configFile(*this),
	m_prefabs(this),
	m_mapDirtyFlag(false)
	{
		if (!m_configFile.LoadFromFile("editorconfig.lua"))
			throw std::runtime_error("failed to load config file");

		Ndk::InitializeComponent<CanvasComponent>("CanvsCmp");

		LoadMods();
		FillStores();

		const std::string& editorAssetsFolder = m_config.GetStringValue("Resources.EditorDirectory");

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
		BuildToolbar(editorAssetsFolder);

		m_canvas = new MapCanvas(*this);

		m_canvas->OnCameraZoomFactorUpdated.Connect([this](MapCanvas* /*emitter*/, float factor)
		{
			statusBar()->showMessage(tr("Zoom level: %1%").arg(static_cast<int>(std::round(factor * 100.f))));
		});

		m_canvas->OnDeleteEntities.Connect([this](MapCanvas* /*emitter*/, const EntityId* entityId, std::size_t entityCount)
		{
			std::vector<std::size_t> entityIndices(entityCount);
			for (std::size_t i = 0; i < entityCount; ++i)
			{
				auto it = m_entityIndices.find(entityId[i]);
				assert(it != m_entityIndices.end());

				entityIndices[i] = it->second;
			}

			OnDeleteEntities(entityIndices.data(), entityIndices.size());
		});

		m_canvas->OnEntitiesPositionUpdated.Connect([this](MapCanvas* /*emitter*/, const EntityId* canvasIndices, std::size_t entityCount, const Nz::Vector2f& offset)
		{
			assert(m_currentLayer.has_value());

			const Map& map = GetWorkingMap();

			std::vector<EntityId> entityUniqueIds;
			for (std::size_t i = 0; i < entityCount; ++i)
			{
				auto it = m_entityIndices.find(canvasIndices[i]);
				assert(it != m_entityIndices.end());

				std::size_t entityIndex = it.value();
				const auto& entity = map.GetEntity(m_currentLayer.value(), entityIndex);

				entityUniqueIds.push_back(entity.uniqueId);
			}

			PushCommand<Commands::PositionUpdate>(std::move(entityUniqueIds), offset);
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

		m_centralTab = new QTabWidget;
		m_centralTab->addTab(m_canvas, tr("Map editor"));

		setCentralWidget(m_centralTab);

		BuildLayerList(editorAssetsFolder);
		BuildEntityList(editorAssetsFolder);

		resize(1280, 720);
		setWindowTitle(tr("Burg'war map editor"));

		ClearWorkingMap();

		m_currentMode = std::make_shared<BasicEditorMode>(*this);
		m_currentMode->OnEnter();

		// Apply settings
		QSettings settings;
		m_showBackgroundColor->setChecked(settings.value(settings_showBackgroundColor, false).toBool());
		m_showColliders->setChecked(settings.value(settings_showColliders, false).toBool());
		m_showGrid->setChecked(settings.value(settings_showGrid, true).toBool());

		statusBar()->showMessage(tr("Ready"), 0);
	}

	EditorWindow::~EditorWindow()
	{
		m_currentMode->OnLeave();
		m_currentMode.reset();

		// Delete canvas before releasing everything else
		delete m_canvas;
	}

	void EditorWindow::ClearSelectedEntity()
	{
		m_entityList.listWidget->clearSelection();
	}

	void EditorWindow::ClearWorkingMap()
	{
		UpdateWorkingMap(Map());
	}

	Map::Entity& EditorWindow::CreateEntity(LayerIndex layerIndex, std::size_t entityIndex, Map::Entity entityData)
	{
		auto& newEntity = GetWorkingMapMut().EmplaceEntity(layerIndex, entityIndex, std::move(entityData));
		m_canvas->CreateEntity(layerIndex, newEntity.uniqueId, newEntity.entityType, newEntity.position, newEntity.rotation, newEntity.properties);

		if (m_currentLayer == layerIndex)
			RegisterEntity(entityIndex);

		return newEntity;
	}

	Map::Layer& EditorWindow::CreateLayer(LayerIndex layerIndex, Map::Layer layerData)
	{
		auto& layer = GetWorkingMapMut().EmplaceLayer(layerIndex, std::move(layerData));
		RebuildCanvas();
		RefreshLayerList();

		return layer;
	}

	Map::Entity EditorWindow::DeleteEntity(EntityId entityId)
	{
		const auto& layerEntity = m_canvas->RetrieveLayerEntityByUniqueId(entityId);
		assert(layerEntity);

		if (m_currentLayer == layerEntity->GetLayerIndex())
		{
			std::size_t entityIndex;
			{
				auto it = m_entityIndices.find(entityId);
				assert(it != m_entityIndices.end());

				entityIndex = it->second;
			}

			delete m_entityList.listWidget->takeItem(int(entityIndex));
			EntityId uniqueId = layerEntity->GetUniqueId();

			m_entityIndices.erase(uniqueId);

			// FIXME...
			for (auto it = m_entityIndices.begin(); it != m_entityIndices.end(); ++it)
			{
				if (it->second >= entityIndex)
				{
					std::size_t newEntityIndex = --it.value();
					m_entityList.listWidget->item(int(newEntityIndex))->setData(Qt::UserRole, qulonglong(newEntityIndex));
				}
			}
		}

		m_canvas->DeleteEntity(entityId);

		Map& map = GetWorkingMapMut();
		return map.DropEntity(entityId);
	}

	Map::Layer EditorWindow::DeleteLayer(LayerIndex layerIndex)
	{
		if (m_currentLayer == layerIndex)
			m_layerList.listWidget->setCurrentRow(-1);

		auto layer = GetWorkingMapMut().DropLayer(layerIndex);
		RebuildCanvas();
		RefreshLayerList();

		return layer;
	}

	Nz::Vector2f EditorWindow::GetCameraCenter() const
	{
		const Camera& camera = m_canvas->GetCamera();
		const Nz::Recti& viewport = camera.GetViewport();

		return camera.Unproject({ viewport.width / 2.f, viewport.height / 2.f });
	}

	void EditorWindow::MoveEntity(EntityId entityId, LayerIndex targetLayer, std::size_t targetEntityIndex)
	{
		Map& map = GetWorkingMapMut();

		const Map::EntityIndices& indices = map.GetEntityIndices(entityId);

		const Map::Entity& mapEntity = map.GetEntity(entityId);
		m_canvas->DeleteEntity(mapEntity.uniqueId);
		m_canvas->CreateEntity(targetLayer, mapEntity.uniqueId, mapEntity.entityType, mapEntity.position, mapEntity.rotation, mapEntity.properties);

		if (m_currentLayer == indices.layerIndex)
		{
			delete m_entityList.listWidget->takeItem(int(indices.entityIndex));

			m_entityIndices.erase(mapEntity.uniqueId);

			// FIXME...
			for (auto it = m_entityIndices.begin(); it != m_entityIndices.end(); ++it)
			{
				if (it->second >= indices.entityIndex)
				{
					std::size_t newEntityIndex = --it.value();
					m_entityList.listWidget->item(int(newEntityIndex))->setData(Qt::UserRole, qulonglong(newEntityIndex));
				}
			}
		}

		// Move entity last (because it will invalidate indices and entity references)
		map.MoveEntity(indices.layerIndex, indices.entityIndex, targetLayer, targetEntityIndex);

		if (m_currentLayer == targetLayer)
			RegisterEntity(targetEntityIndex);
	}

	void EditorWindow::OpenEntityContextMenu(std::optional<std::size_t> entityIndexOpt, const QPoint& pos, QWidget* parent)
	{
		if (!m_workingMap.IsValid())
			return;

		QMenu contextMenu(parent);

		if (entityIndexOpt)
		{
			std::size_t entityIndex = *entityIndexOpt;

			QAction* editEntity = contextMenu.addAction(tr("Edit entity"));
			connect(editEntity, &QAction::triggered, [this, entityIndex](bool)
			{
				OnEditEntity(entityIndex);
			});

			QAction* cloneEntity = contextMenu.addAction(tr("Clone entity"));
			connect(cloneEntity, &QAction::triggered, [this, entityIndex](bool)
			{
				OnCloneEntity(entityIndex);
			});

			QAction* deleteEntity = contextMenu.addAction(tr("Delete entity"));
			connect(deleteEntity, &QAction::triggered, [this, entityIndex](bool)
			{
				OnDeleteEntities(&entityIndex, 1);
			});

			std::size_t layerCount = m_workingMap.GetLayerCount();
			if (layerCount > 1)
			{
				contextMenu.addSeparator();

				QMenu* cloneEntityMenu = contextMenu.addMenu(tr("Clone entity to layer"));
				QMenu* moveEntityMenu = contextMenu.addMenu(tr("Move entity to layer"));

				assert(m_currentLayer);
				std::size_t currentLayer = m_currentLayer.value();

				for (std::size_t i = 0; i < m_workingMap.GetLayerCount(); ++i)
				{
					if (i == currentLayer)
						continue;

					auto& layer = m_workingMap.GetLayer(LayerIndex(i));

					QString layerName = QString::fromStdString(layer.name);

					QAction* cloneAction = cloneEntityMenu->addAction(layerName);
					connect(cloneAction, &QAction::triggered, [this, entityIndex, layerIndex = LayerIndex(i)](bool)
					{
						OnCloneEntity(entityIndex, layerIndex);
					});

					QAction* moveAction = moveEntityMenu->addAction(layerName);
					connect(moveAction, &QAction::triggered, [this, entityIndex, layerIndex = LayerIndex(i)](bool)
					{
						OnMoveEntity(entityIndex, layerIndex);
					});
				}
			}
		}
		else
		{
			QAction* createEntity = contextMenu.addAction(tr("Create entity..."));
			connect(createEntity, &QAction::triggered, [this](bool)
			{
				OnCreateEntity();
			});
		}

		contextMenu.exec(pos);
	}

	void EditorWindow::PushCommand(QUndoCommand* command)
	{
		m_undoStack.push(command);
	}

	void EditorWindow::RefreshEntityPositionAndRotation(LayerIndex layerIndex, std::size_t entityIndex)
	{
		const Map& map = GetWorkingMap();
		const auto& entity = map.GetEntity(layerIndex, entityIndex);
		m_canvas->UpdateEntityPositionAndRotation(entity.uniqueId, entity.position, entity.rotation);
	}

	void EditorWindow::SelectEntity(std::size_t entityIndex, bool clearPrevious)
	{
		if (clearPrevious)
			m_entityList.listWidget->clearSelection();

		m_entityList.listWidget->item(int(entityIndex))->setSelected(true);
	}

	void EditorWindow::SelectEntities(const std::vector<std::size_t>& entityIndices)
	{
		m_entityList.listWidget->clearSelection();
		for (std::size_t entityIndex : entityIndices)
		{
			assert(int(entityIndex) < m_entityList.listWidget->count());
			m_entityList.listWidget->item(int(entityIndex))->setSelected(true);
		}
	}

	void EditorWindow::SwapEntities(LayerIndex layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex)
	{
		auto& map = GetWorkingMapMut();

		EntityId firstEntityId = map.GetEntity(layerIndex, firstEntityIndex).uniqueId;
		EntityId secondEntityId = map.GetEntity(layerIndex, secondEntityIndex).uniqueId;

		map.SwapEntities(layerIndex, firstEntityIndex, secondEntityIndex);

		if (m_currentLayer != layerIndex)
			return;

		QListWidgetItem* firstItem = m_entityList.listWidget->item(int(firstEntityIndex));
		QListWidgetItem* secondItem = m_entityList.listWidget->item(int(secondEntityIndex));

		QString firstItemText = firstItem->text();
		QString secondItemText = secondItem->text();

		firstItem->setText(secondItemText);
		secondItem->setText(firstItemText);

		m_entityIndices[firstEntityId] = secondEntityIndex;
		m_entityIndices[secondEntityId] = firstEntityIndex;

		if (m_entityList.listWidget->currentRow() == int(firstEntityIndex))
			m_entityList.listWidget->setCurrentRow(int(secondEntityIndex));
		else if (m_entityList.listWidget->currentRow() == int(secondEntityIndex))
			m_entityList.listWidget->setCurrentRow(int(firstEntityIndex));

		UpdateEntityListButtons();
	}

	void EditorWindow::SwapLayers(LayerIndex firstLayerIndex, LayerIndex secondLayerIndex)
	{
		Map& map = GetWorkingMapMut();
		map.SwapLayers(firstLayerIndex, secondLayerIndex);

		// Swap items text
		QListWidgetItem* oldItem = m_layerList.listWidget->item(int(firstLayerIndex));
		QListWidgetItem* newItem = m_layerList.listWidget->item(int(secondLayerIndex));

		QString oldItemText = oldItem->text();

		oldItem->setText(newItem->text());
		newItem->setText(oldItemText);

		// Update current layer before selecting the new row, to prevent a layer refresh
		if (m_currentLayer)
		{
			if (*m_currentLayer == firstLayerIndex)
				m_currentLayer = secondLayerIndex;
			else if (*m_currentLayer == secondLayerIndex)
				m_currentLayer = firstLayerIndex;
		}

		if (m_layerList.listWidget->currentRow() == int(firstLayerIndex))
			m_layerList.listWidget->setCurrentRow(int(secondLayerIndex));
		else if (m_layerList.listWidget->currentRow() == int(secondLayerIndex))
			m_layerList.listWidget->setCurrentRow(int(firstLayerIndex));

		UpdateLayerListButtons();

		RebuildCanvas();
	}

	void EditorWindow::SwitchToMode(std::shared_ptr<EditorMode> editorMode)
	{
		m_currentMode->OnLeave();
		m_currentMode = std::move(editorMode);
		m_currentMode->OnEnter();
	}

	void EditorWindow::ToggleEntitySelection(std::size_t entityIndex)
	{
		QListWidgetItem* item = m_entityList.listWidget->item(int(entityIndex));
		item->setSelected(!item->isSelected());
	}

	void EditorWindow::UpdateEntity(LayerIndex layerIndex, std::size_t entityIndex, Map::Entity entityData, EntityInfoUpdateFlags updateFlags)
	{
		auto& mapEntity = GetWorkingMapMut().GetEntity(layerIndex, entityIndex);

		bool resetItemName = false;

		if (updateFlags & EntityInfoUpdate::EntityClass)
		{
			mapEntity.entityType = std::move(entityData.entityType);
			resetItemName = true;
		}

		if (updateFlags & EntityInfoUpdate::EntityName)
		{
			mapEntity.name = std::move(entityData.name);
			resetItemName = true;
		}

		if (updateFlags & EntityInfoUpdate::PositionRotation)
		{
			mapEntity.position = entityData.position;
			mapEntity.rotation = entityData.rotation;
		}

		if (updateFlags & EntityInfoUpdate::Properties)
			mapEntity.properties = std::move(entityData.properties);

		QListWidgetItem* item = m_entityList.listWidget->item(int(entityIndex));
		assert(item);

		if (updateFlags & (EntityInfoUpdate::EntityClass | EntityInfoUpdate::Properties))
		{
			m_canvas->DeleteEntity(mapEntity.uniqueId);
			m_canvas->CreateEntity(layerIndex, mapEntity.uniqueId, mapEntity.entityType, mapEntity.position, mapEntity.rotation, mapEntity.properties)->GetId();
		}
		else if (updateFlags & EntityInfoUpdate::PositionRotation)
			m_canvas->UpdateEntityPositionAndRotation(mapEntity.uniqueId, mapEntity.position, mapEntity.rotation);

		if (m_currentLayer == layerIndex)
		{
			if (resetItemName)
			{
				QString entryName = QString::fromStdString(mapEntity.entityType);
				if (!mapEntity.name.empty())
					entryName = entryName % " (" % QString::fromStdString(mapEntity.name) % ")";

				item->setText(entryName);
			}

			// Trigger selection signal
			if (item->isSelected())
			{
				m_entityList.listWidget->setCurrentRow(-1);
				m_entityList.listWidget->setCurrentRow(int(entityIndex));
			}
		}
	}

	void EditorWindow::UpdateWorkingMap(Map map, std::filesystem::path mapPath)
	{
		m_currentLayer.reset();
		m_workingMap = std::move(map);
		m_workingMapPath = std::move(mapPath);
		m_mapDirtyFlag = false;

		// Reset entity info dialog (as it depends on the map)
		if (m_entityInfoDialog)
		{
			m_entityInfoDialog->deleteLater();
			m_entityInfoDialog = nullptr;
		}

		setWindowFilePath(QString::fromStdString(mapPath.generic_u8string()));

		bool hasWorkingMap = m_workingMap.IsValid();

		m_closeMap->setEnabled(hasWorkingMap);
		m_compileMap->setEnabled(hasWorkingMap);
		m_createEntityActionToolbar->setEnabled(hasWorkingMap);
		m_mapMenu->setEnabled(hasWorkingMap);
		m_playMap->setEnabled(hasWorkingMap);
		m_saveMap->setEnabled(hasWorkingMap);
		m_saveMapToolbar->setEnabled(mapPath.empty()); //< Enabled for new maps/disabled for opened maps

		if (!hasWorkingMap)
			m_layerMenu->setEnabled(false);

		m_undoStack.clear();

		m_canvas->UpdateBackgroundColor(Nz::Color::Black);

		if (hasWorkingMap)
			RebuildCanvas();

		RefreshLayerList();

		if (m_layerList.listWidget->count() > 0)
			m_layerList.listWidget->setCurrentRow(0);
	}

	void EditorWindow::closeEvent(QCloseEvent* event)
	{
		if (CanCloseMap())
			event->accept();
		else
			event->ignore();
	}

	bool EditorWindow::event(QEvent* e)
	{
		switch (e->type())
		{
			case QEvent::KeyPress:
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
				if (keyEvent->key() == Qt::Key_Delete)
					OnDeleteEntities();
			}

			default:
				break;
		}

		return QMainWindow::event(e);
	}

	void EditorWindow::AddScriptsToMap(Map& scriptedMap)
	{
		assert(!m_workingMapPath.empty());

		std::filesystem::path rootScriptFolder = m_workingMapPath / "scripts";
		if (!std::filesystem::is_directory(rootScriptFolder))
			return;

		auto& mapScripts = scriptedMap.GetScripts();
		for (std::filesystem::path filepath : std::filesystem::recursive_directory_iterator(rootScriptFolder))
		{
			if (!std::filesystem::is_regular_file(filepath))
				continue;

			std::string filepathStr = filepath.generic_u8string();

			Nz::File file(filepath.generic_u8string());
			if (!file.Open(Nz::OpenMode_ReadOnly))
			{
				bwLog(GetLogger(), LogLevel::Warning, "failed to open script {} when preparing map test", filepathStr);
				continue;
			}

			std::vector<Nz::UInt8> content(file.GetSize());
			if (file.Read(content.data(), content.size()) != content.size())
			{
				bwLog(GetLogger(), LogLevel::Warning, "failed to read script {} when preparing map test", filepathStr);
				continue;
			}

			std::filesystem::path relativePath = std::filesystem::relative(filepath, rootScriptFolder);

			mapScripts.push_back({
				relativePath.generic_u8string(),
				std::move(content)
			});
		}
	}

	void EditorWindow::AddToRecentFileList(const QString& mapFolder)
	{
		QSettings settings;
		QStringList recentlyOpenedMaps = settings.value(settings_recentFiles).toStringList();

		recentlyOpenedMaps.removeAll(mapFolder);
		recentlyOpenedMaps.prepend(mapFolder);
		while (recentlyOpenedMaps.size() > int(MaxRecentFiles))
			recentlyOpenedMaps.removeLast();

		settings.setValue(settings_recentFiles, recentlyOpenedMaps);

		RefreshRecentFileListMenu(recentlyOpenedMaps);
	}

	void EditorWindow::AlignLayerEntities(LayerIndex layerIndex)
	{
		auto& layer = GetWorkingMapMut().GetLayer(layerIndex);
		for (auto& layerEntity : layer.entities)
			layerEntity.position = AlignPosition(layerEntity.position, layer.positionAlignment);

		for (std::size_t i = 0; i < layer.entities.size(); ++i)
		{
			const auto& layerEntity = layer.entities[i];
			m_canvas->UpdateEntityPositionAndRotation(layerEntity.uniqueId, layerEntity.position, layerEntity.rotation);
		}
	}

	void EditorWindow::BuildAssetList()
	{
		Map& map = GetWorkingMapMut();

		tsl::hopscotch_set<std::string> textures;
		map.ForeachEntityPropertyValue<PropertyType::Texture>([&](Map::Entity& /*entity*/, const std::string& /*name*/, const std::string& texturePath)
		{
			textures.insert(texturePath);
		});

		std::filesystem::path gameResourceFolder = std::filesystem::u8path(m_config.GetStringValue("Resources.AssetDirectory"));

		std::vector<Map::Asset>& assets = map.GetAssets();
		assets.clear();

		auto hash = Nz::AbstractHash::Get(Nz::HashType_SHA1);

		for (const std::string& texturePath : textures)
		{
			std::filesystem::path fullPath = gameResourceFolder / std::filesystem::u8path(texturePath);

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

		std::sort(assets.begin(), assets.end(), [&](const Map::Asset& lhs, const Map::Asset& rhs)
		{
			return lhs.filepath < rhs.filepath;
		});

		bwLog(GetLogger(), LogLevel::Info, "Finished building assets");
	}

	void EditorWindow::BuildEntityList(const std::string& editorAssetsFolder)
	{
		QDockWidget* entityListDock = new QDockWidget("Layer entities", this);

		m_entityList.listWidget = new QListWidget;
		m_entityList.listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		m_entityList.listWidget->setSelectionMode(QListWidget::ExtendedSelection);
		connect(m_entityList.listWidget, &QListWidget::customContextMenuRequested, [this](const QPoint& pos)
		{
			std::optional<std::size_t> entityIndex;

			QListWidgetItem* item = m_entityList.listWidget->itemAt(pos);
			if (item)
				entityIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());

			OpenEntityContextMenu(entityIndex, m_entityList.listWidget->mapToGlobal(pos), m_entityList.listWidget);
		});

		connect(m_entityList.listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item)
		{
			std::size_t entityIndex = static_cast<std::size_t>(item->data(Qt::UserRole).value<qulonglong>());

			OnEditEntity(entityIndex);
		});

		connect(m_entityList.listWidget, &QListWidget::itemSelectionChanged, this, &EditorWindow::OnEntitySelectionUpdate);

		m_entityList.upArrowButton = new QPushButton;
		m_entityList.upArrowButton->setIcon(QIcon(QPixmap((editorAssetsFolder + "/up-24.png").c_str())));
		m_entityList.upArrowButton->setEnabled(false);
		connect(m_entityList.upArrowButton, &QPushButton::released, this, &EditorWindow::OnEntityMovedUp);

		m_entityList.downArrowButton = new QPushButton;
		m_entityList.downArrowButton->setIcon(QIcon(QPixmap((editorAssetsFolder + "/down-24.png").c_str())));
		m_entityList.downArrowButton->setEnabled(false);
		connect(m_entityList.downArrowButton, &QPushButton::released, this, &EditorWindow::OnEntityMovedDown);

		QVBoxLayout* arrowLayout = new QVBoxLayout;
		arrowLayout->addWidget(m_entityList.upArrowButton);
		arrowLayout->addWidget(m_entityList.downArrowButton);

		QHBoxLayout* entityListLayout = new QHBoxLayout;
		entityListLayout->addWidget(m_entityList.listWidget);
		entityListLayout->addLayout(arrowLayout);

		QWidget* content = new QWidget;
		content->setLayout(entityListLayout);

		entityListDock->setWidget(content);

		addDockWidget(Qt::RightDockWidgetArea, entityListDock);
	}

	void EditorWindow::BuildLayerList(const std::string& editorAssetsFolder)
	{
		QDockWidget* layerListDock = new QDockWidget("Layer list", this);

		m_layerList.listWidget = new QListWidget;
		m_layerList.listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_layerList.listWidget, &QListWidget::customContextMenuRequested, [this](const QPoint& pos)
		{
			if (!m_workingMap.IsValid())
				return;

			QMenu contextMenu(m_layerList.listWidget);

			QListWidgetItem* item = m_layerList.listWidget->itemAt(pos);
			if (item)
			{
				LayerIndex layerIndex = static_cast<LayerIndex>(item->data(Qt::UserRole).value<qulonglong>());

				QAction* editLayer = contextMenu.addAction(tr("Edit layer"));
				connect(editLayer, &QAction::triggered, [this, layerIndex](bool)
				{
					OnEditLayer(layerIndex);
				});

				QAction* cloneLayer = contextMenu.addAction(tr("Clone layer"));
				connect(cloneLayer, &QAction::triggered, [this, layerIndex](bool)
				{
					OnCloneLayer(layerIndex);
				});

				QAction* deleteLayer = contextMenu.addAction(tr("Delete layer"));
				connect(deleteLayer, &QAction::triggered, [this, layerIndex](bool)
				{
					OnDeleteLayer(layerIndex);
				});
			}
			else
			{
				QAction* deleteLayer = contextMenu.addAction(tr("Create layer"));
				connect(deleteLayer, &QAction::triggered, [this](bool)
				{
					OnCreateLayer();
				});
			}

			contextMenu.exec(m_layerList.listWidget->mapToGlobal(pos));
		});

		connect(m_layerList.listWidget, &QListWidget::currentRowChanged, this, &EditorWindow::OnLayerChanged);
		connect(m_layerList.listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item)
		{
			LayerIndex layerIndex = static_cast<LayerIndex>(item->data(Qt::UserRole).value<qulonglong>());

			OnEditLayer(layerIndex);
		});

		m_layerList.upArrowButton = new QPushButton;
		m_layerList.upArrowButton->setIcon(QIcon(QPixmap((editorAssetsFolder + "/up-24.png").c_str())));
		m_layerList.upArrowButton->setEnabled(false);
		connect(m_layerList.upArrowButton, &QPushButton::released, this, &EditorWindow::OnLayerMovedUp);

		m_layerList.downArrowButton = new QPushButton;
		m_layerList.downArrowButton->setIcon(QIcon(QPixmap((editorAssetsFolder + "/down-24.png").c_str())));
		m_layerList.downArrowButton->setEnabled(false);
		connect(m_layerList.downArrowButton, &QPushButton::released, this, &EditorWindow::OnLayerMovedDown);

		QVBoxLayout* arrowLayout = new QVBoxLayout;
		arrowLayout->addWidget(m_layerList.upArrowButton);
		arrowLayout->addWidget(m_layerList.downArrowButton);

		QHBoxLayout* entityListLayout = new QHBoxLayout;
		entityListLayout->addWidget(m_layerList.listWidget);
		entityListLayout->addLayout(arrowLayout);
		
		QWidget* content = new QWidget;
		content->setLayout(entityListLayout);

		layerListDock->setWidget(content);

		addDockWidget(Qt::RightDockWidgetArea, layerListDock);
	}

	void EditorWindow::BuildMenu()
	{
		QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
		{
			QAction* createMap = fileMenu->addAction(tr("Create map..."));
			createMap->setShortcut(QKeySequence::New);
			connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

			QAction* openMap = fileMenu->addAction(tr("Open map..."));
			createMap->setShortcut(QKeySequence::Open);
			connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

			QMenu* recentMaps = fileMenu->addMenu(tr("Open recent"));
			recentMaps->setToolTipsVisible(true);

			for (QAction* action : m_recentMapActions)
				recentMaps->addAction(action);

			RefreshRecentFileListMenu();

			m_saveMap = fileMenu->addAction(tr("Save map"));
			m_saveMap->setShortcut(QKeySequence::Save);
			connect(m_saveMap, &QAction::triggered, this, &EditorWindow::OnSaveMap);

			m_closeMap = fileMenu->addAction(tr("Close map"));
			m_closeMap->setShortcut(QKeySequence::Close);
			connect(m_closeMap, &QAction::triggered, this, &EditorWindow::OnCloseMap);

			fileMenu->addSeparator();

			m_compileMap = fileMenu->addAction(tr("Compile map..."));
			connect(m_compileMap, &QAction::triggered, this, &EditorWindow::OnCompileMap);
		}

		QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
		{
			QAction* undoAction = m_undoStack.createUndoAction(this, tr("&Undo"));
			undoAction->setShortcut(QKeySequence::Undo);
			editMenu->addAction(undoAction);

			QAction* redoAction = m_undoStack.createRedoAction(this, tr("&Redo"));
			redoAction->setShortcut(QKeySequence::Redo);
			editMenu->addAction(redoAction);
		}

		QMenu* editorMenu = menuBar()->addMenu(tr("&Editor"));
		{
			QAction* reloadScripts = editorMenu->addAction(tr("Reload scripts"));
			connect(reloadScripts, &QAction::triggered, this, &EditorWindow::OnReloadScripts);
		}

		m_mapMenu = menuBar()->addMenu(tr("&Map"));
		{
			QMenu* layerMenu = m_mapMenu->addMenu("Layers");
			QAction* addLayer = layerMenu->addAction(tr("Add layer"));
			connect(addLayer, &QAction::triggered, this, &EditorWindow::OnCreateLayer);

			QAction* rebuildAssetList = editorMenu->addAction(tr("Rebuild asset list"));
			connect(rebuildAssetList, &QAction::triggered, this, &EditorWindow::BuildAssetList);

			QAction* playMap = m_mapMenu->addAction(tr("Play map"));
			connect(playMap, &QAction::triggered, this, &EditorWindow::OnPlayMap);
		}

		m_layerMenu = menuBar()->addMenu(tr("&Layer"));
		{
			QAction* alignEntities = m_layerMenu->addAction(tr("Align layers entities"));
			connect(alignEntities, &QAction::triggered, this, &EditorWindow::OnAlignEntities);

			QAction* setAlignment = m_layerMenu->addAction(tr("Set position alignment..."));
			connect(setAlignment, &QAction::triggered, this, &EditorWindow::OnSetAlignment);
		}

		m_prefabs.BuildMenu(menuBar());

		QMenu* showMenu = menuBar()->addMenu(tr("&Show"));
		{
			m_showColliders = showMenu->addAction(tr("Show colliders"));
			m_showColliders->setCheckable(true);

			connect(m_showColliders, &QAction::toggled, [this](bool checked)
			{
				m_canvas->EnablePhysicsDebugDraw(checked);

				QSettings().setValue(settings_showColliders, checked);
			});

			m_showGrid = showMenu->addAction(tr("Show grid"));
			m_showGrid->setCheckable(true);

			connect(m_showGrid, &QAction::toggled, [this](bool checked)
			{
				m_canvas->ShowGrid(checked);
				
				QSettings().setValue(settings_showGrid, checked);
			});

			m_showBackgroundColor = showMenu->addAction(tr("Show background"));
			m_showBackgroundColor->setCheckable(true);

			connect(m_showBackgroundColor, &QAction::toggled, [this](bool checked)
			{
				if (checked)
				{
					if (m_currentLayer)
						m_canvas->UpdateBackgroundColor(m_workingMap.GetLayer(*m_currentLayer).backgroundColor);
				}
				else
					m_canvas->UpdateBackgroundColor(Nz::Color::Black);

				QSettings().setValue(settings_showBackgroundColor, checked);
			});
		}

		QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
		{
			QAction* aboutQt = helpMenu->addAction(tr("About Qt..."));
			aboutQt->setMenuRole(QAction::AboutQtRole);
			connect(aboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
		}
	}

	void EditorWindow::BuildToolbar(const std::string& editorAssetsFolder)
	{
		QToolBar* toolBar = new QToolBar;
		QAction* createMap = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/file-48.png").c_str())), tr("Create map..."));
		connect(createMap, &QAction::triggered, this, &EditorWindow::OnCreateMap);

		QAction* openMap = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/opened_folder-48.png").c_str())), tr("Open map..."));
		connect(openMap, &QAction::triggered, this, &EditorWindow::OnOpenMap);

		m_saveMapToolbar = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/icons8-save-48.png").c_str())), tr("Save map..."));
		connect(m_saveMapToolbar, &QAction::triggered, this, &EditorWindow::OnSaveMap);

		toolBar->addSeparator();

		m_createEntityActionToolbar = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/idea-48.png").c_str())), tr("Create entity"));
		connect(m_createEntityActionToolbar, &QAction::triggered, this, &EditorWindow::OnCreateEntity);

		toolBar->addSeparator();

		m_perspectiveAction = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/perspective-48.png").c_str())), tr("Switch between perspective/orthogonal view"));
		m_perspectiveAction->setCheckable(true);
		connect(m_perspectiveAction, &QAction::toggled, this, &EditorWindow::OnPerspectiveSwitch);

		toolBar->addSeparator();

		m_playMap = toolBar->addAction(QIcon(QPixmap((editorAssetsFolder + "/start-48.png").c_str())), tr("Play map"));
		connect(m_playMap, &QAction::triggered, this, &EditorWindow::OnPlayMap);

		QDockWidget* toolbarDock = new QDockWidget("Toolbar", this);
		toolbarDock->setWidget(toolBar);

		addDockWidget(Qt::TopDockWidgetArea, toolbarDock);
	}

	bool EditorWindow::CanCloseMap()
	{
		if (!m_mapDirtyFlag)
			return true;

		QString warningText = tr("Your changes to this map have not been saved, do you want to save before closing it?");
		QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Quit without saving?"), warningText, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (response == QMessageBox::Cancel)
			return false;

		if (response == QMessageBox::Yes)
		{
			// Save before exit
			if (!SaveMap())
				return false; //< Save failed, do not close
		}

		return true;
	}

	EntityInfoDialog* EditorWindow::GetEntityInfoDialog()
	{
		if (!m_entityInfoDialog)
			m_entityInfoDialog = new EntityInfoDialog(GetLogger(), m_workingMap, m_canvas->GetEntityStore(), m_canvas->GetScriptingContext(), this);

		return m_entityInfoDialog;
	}

	void EditorWindow::InvalidateMap()
	{
		m_mapDirtyFlag = true;
		m_saveMapToolbar->setEnabled(true);
	}

	void EditorWindow::RefreshRecentFileListMenu()
	{
		QSettings settings;
		QStringList recentMaps = settings.value(settings_recentFiles).toStringList();

		RefreshRecentFileListMenu(recentMaps);
	}

	void EditorWindow::RefreshRecentFileListMenu(const QStringList& recentFileList)
	{
		std::size_t fileCount = std::min<std::size_t>(m_recentMapActions.size(), recentFileList.size());
		for (std::size_t i = 0; i < fileCount; ++i)
		{
			QString fullPath = recentFileList[int(i)];
			QString filename = QFileInfo(fullPath).fileName();

			QAction* recentMap = m_recentMapActions[int(i)];
			recentMap->setData(recentFileList[int(i)]);
			recentMap->setText(tr("&%1 %2").arg(int(i + 1)).arg(filename));
			recentMap->setVisible(true);
			recentMap->setToolTip(fullPath);
		}

		for (std::size_t i = fileCount; i < m_recentMapActions.size(); ++i)
			m_recentMapActions[int(i)]->setVisible(false);
	}

	void EditorWindow::OnAlignEntities()
	{
		assert(m_currentLayer);
		LayerIndex currentLayerIndex = *m_currentLayer;

		const auto& layerData = m_workingMap.GetLayer(currentLayerIndex);

		QString warningText = tr("You are about to align all entities from layer %1, are you sure?").arg(QString::fromStdString(layerData.name));
		QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Are you sure?"), warningText, QMessageBox::Yes | QMessageBox::Cancel);
		if (response == QMessageBox::Yes)
			AlignLayerEntities(currentLayerIndex);
	}

	void EditorWindow::OnCloneEntity(std::size_t entityIndex)
	{
		assert(m_currentLayer);
		LayerIndex currentLayer = *m_currentLayer;

		PushCommand<Commands::EntityClone>(Map::EntityIndices{ currentLayer, entityIndex }, Map::EntityIndices{ currentLayer, entityIndex + 1 });
	}

	void EditorWindow::OnCloneEntity(std::size_t entityIndex, LayerIndex layerIndex)
	{
		assert(m_currentLayer);
		LayerIndex currentLayer = *m_currentLayer;

		std::size_t entityCount = GetWorkingMap().GetEntityCount(layerIndex);
		PushCommand<Commands::EntityClone>(Map::EntityIndices{ currentLayer, entityIndex }, Map::EntityIndices{ layerIndex, entityCount });
	}

	void EditorWindow::OnCloneLayer(LayerIndex layerIndex)
	{
		PushCommand<Commands::LayerClone>(layerIndex, LayerIndex(GetWorkingMap().GetLayerCount()));
	}

	void EditorWindow::OnCloseMap()
	{
		if (CanCloseMap())
			ClearWorkingMap();
	}

	void EditorWindow::OnCompileMap()
	{
		QString filter("*.bmap");
		QString fileName = QFileDialog::getSaveFileName(this, tr("Where to save compiled map file"), QString(), filter, &filter);
		if (fileName.isEmpty())
			return;

		if (!fileName.endsWith(".bmap"))
			fileName += ".bmap";

		if (QMessageBox::question(this, tr("Build asset list?"), tr("Do you want to rebuild asset list before compiling map?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			BuildAssetList();

		Map scriptedMap = GetWorkingMap();
		AddScriptsToMap(scriptedMap);

		if (scriptedMap.Compile(fileName.toStdString()))
			QMessageBox::information(this, tr("Compilation succeeded"), tr("Map has been successfully compiled"), QMessageBox::Ok);
		else
			QMessageBox::critical(this, tr("Failed to compile map"), tr("Map failed to compile"), QMessageBox::Ok);
	}

	void EditorWindow::OnCreateEntity()
	{
		auto currentLayerOpt = GetCurrentLayer();
		if (!currentLayerOpt)
			return;

		const auto& workingMap = GetWorkingMap();

		LayerIndex layerIndex = *currentLayerOpt;
		const auto& layer = workingMap.GetLayer(layerIndex);

		EntityInfoDialog* createEntityDialog = GetEntityInfoDialog();

		EntityInfo entityInfo;
		entityInfo.position = AlignPosition(GetCameraCenter(), layer.positionAlignment);

		createEntityDialog->Open(workingMap.GetFreeUniqueId(), layerIndex, entityInfo, Ndk::EntityHandle::InvalidHandle, [this, layerIndex](EntityInfoDialog* /*createEntityDialog*/, EntityInfo&& entityInfo, EntityInfoUpdateFlags /*dummy*/)
		{
			Map& map = GetWorkingMapMut();
			std::size_t entityIndex = map.GetEntityCount(layerIndex);

			Map::EntityIndices indices{ layerIndex, entityIndex };
			Map::Entity entityData;
			entityData.entityType = std::move(entityInfo.entityClass);
			entityData.name = std::move(entityInfo.entityName);
			entityData.position = entityInfo.position;
			entityData.properties = std::move(entityInfo.properties);
			entityData.rotation = entityInfo.rotation;
			entityData.uniqueId = map.GenerateUniqueId();

			PushCommand<Commands::EntityCreate>(std::move(indices), std::move(entityData));

			SelectEntity(entityIndex);
		});
	}

	void EditorWindow::OnCreateMap()
	{
		FileDescDialog* createMapDialog = new FileDescDialog(this);
		connect(createMapDialog, &QDialog::accepted, [this, createMapDialog]()
		{
			FileDescInfo fileDescInfo = createMapDialog->GetInfo();

			MapInfo mapInfo;
			mapInfo.author = std::move(fileDescInfo.author);
			mapInfo.description = std::move(fileDescInfo.description);
			mapInfo.name = std::move(fileDescInfo.name);

			Map newMap(mapInfo);
			auto& layer = newMap.AddLayer();
			layer.name = "Layer #1";

			UpdateWorkingMap(newMap);
		});
		createMapDialog->exec();
	}

	void EditorWindow::OnCreateLayer()
	{
		if (!m_workingMap.IsValid())
			return;

		const Map& map = GetWorkingMap();

		LayerIndex newLayerIndex = LayerIndex(map.GetLayerCount());

		Map::Layer newLayer;
		newLayer.name = "Layer #" + std::to_string(newLayerIndex);

		PushCommand<Commands::LayerCreate>(newLayerIndex, std::move(newLayer));
	}

	bool EditorWindow::OnDeleteEntities()
	{
		if (OnDeleteEntities(m_selectedEntities.data(), m_selectedEntities.size()))
		{
			m_entityList.listWidget->setCurrentRow(-1);
			return true;
		}

		return false;
	}

	bool EditorWindow::OnDeleteEntities(const std::size_t* entityIndices, std::size_t entityCount)
	{
		assert(m_currentLayer);
		auto& layer = m_workingMap.GetLayer(m_currentLayer.value());

		QString warningText;
		if (entityCount > 1)
		{
			assert(entityCount != 0);

			warningText = tr("You are about to delete %1 entities, are you sure you want to do that?").arg(QString::number(entityCount));
		}
		else
		{
			assert(entityCount != 0);

			auto& layerEntity = layer.entities[*entityIndices];
			warningText = tr("You are about to delete entity %1 of type %2, are you sure you want to do that?").arg(QString::fromStdString(layerEntity.name)).arg(QString::fromStdString(layerEntity.entityType));
		}

		QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Are you sure?"), warningText, QMessageBox::Yes | QMessageBox::Cancel);
		if (response == QMessageBox::Yes)
		{
			std::vector<EntityId> entityUniqueIds(entityCount);
			for (std::size_t i = 0; i < entityCount; ++i)
				entityUniqueIds[i] = layer.entities[entityIndices[i]].uniqueId;

			PushCommand<Commands::EntityDelete>(std::move(entityUniqueIds));

			return true;
		}
		else
			return false;
	}

	void EditorWindow::OnDeleteLayer(LayerIndex layerIndex)
	{
		auto& layer = m_workingMap.GetLayer(layerIndex);

		QString warningText = tr("You are about to delete layer %1, are you sure you want to do that?").arg(QString::fromStdString(layer.name));
		QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Are you sure?"), warningText, QMessageBox::Yes | QMessageBox::Cancel);
		if (response == QMessageBox::Yes)
			PushCommand<Commands::LayerDelete>(layerIndex);
	}

	void EditorWindow::OnEditEntity(std::size_t entityIndex)
	{
		QListWidgetItem* item = m_entityList.listWidget->item(int(entityIndex));
		if (!item)
			return;

		LayerIndex layerIndex = static_cast<LayerIndex>(m_layerList.listWidget->currentRow());

		const auto& workingMap = GetWorkingMap();
		auto& layer = workingMap.GetLayer(layerIndex);

		auto& mapEntity = layer.entities[entityIndex];

		EntityInfo entityInfo;
		entityInfo.entityClass = mapEntity.entityType;
		entityInfo.entityName = mapEntity.name;
		entityInfo.position = mapEntity.position;
		entityInfo.properties = mapEntity.properties;
		entityInfo.rotation = mapEntity.rotation;

		const auto& entity = m_canvas->RetrieveEntityByUniqueId(mapEntity.uniqueId);

		EntityInfoDialog* editEntityDialog = GetEntityInfoDialog();
		editEntityDialog->Open(mapEntity.uniqueId, layerIndex, std::move(entityInfo), entity, [this, uniqueId = mapEntity.uniqueId](EntityInfoDialog* /*editEntityDialog*/, EntityInfo&& entityInfo, EntityInfoUpdateFlags updateFlags)
		{
			Map::Entity entity;
			entity.entityType = std::move(entityInfo.entityClass);
			entity.name = std::move(entityInfo.entityName);
			entity.position = std::move(entityInfo.position);
			entity.properties = std::move(entityInfo.properties);
			entity.rotation = std::move(entityInfo.rotation);
			entity.uniqueId = uniqueId;

			PushCommand<Commands::EntityUpdate>(uniqueId, std::move(entity), updateFlags);
		});
	}
	
	void EditorWindow::OnEditLayer(LayerIndex layerIndex)
	{
		QListWidgetItem* item = m_layerList.listWidget->item(int(layerIndex));
		if (!item)
			return;

		auto& layer = m_workingMap.GetLayer(layerIndex);

		LayerInfo layerInfo;
		layerInfo.backgroundColor = layer.backgroundColor;
		layerInfo.name = layer.name;

		LayerEditDialog* layerInfoDialog = new LayerEditDialog(LayerIndex(layerIndex), layerInfo, m_workingMap, this);
		connect(layerInfoDialog, &QDialog::accepted, [this, layerInfoDialog, layerIndex, item]()
		{
			LayerInfo layerInfo = layerInfoDialog->GetLayerInfo();

			auto& layer = m_workingMap.GetLayer(layerIndex);

			layer.backgroundColor = layerInfo.backgroundColor;
			if (m_showBackgroundColor->isChecked())
				m_canvas->UpdateBackgroundColor(layer.backgroundColor);

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

	void EditorWindow::OnEntityMovedDown()
	{
		if (m_selectedEntities.size() != 1)
			return;

		std::size_t entityIndex = m_selectedEntities.front();
		if (int(entityIndex + 1) >= m_entityList.listWidget->count())
			return;

		std::size_t newEntityIndex = entityIndex + 1;

		assert(m_currentLayer);
		PushCommand<Commands::EntitySwap>(*m_currentLayer, entityIndex, newEntityIndex);
	}

	void EditorWindow::OnEntityMovedUp()
	{
		if (m_selectedEntities.size() != 1)
			return;

		std::size_t entityIndex = m_selectedEntities.front();
		if (entityIndex == 0)
			return;

		std::size_t newEntityIndex = entityIndex - 1;

		assert(m_currentLayer);
		PushCommand<Commands::EntitySwap>(*m_currentLayer, entityIndex, newEntityIndex);
	}

	void EditorWindow::OnEntitySelectionUpdate()
	{
		m_selectedEntities.clear();

		for (QListWidgetItem* item : m_entityList.listWidget->selectedItems())
			m_selectedEntities.push_back(static_cast<std::size_t>(m_entityList.listWidget->row(item)));

		UpdateEntityListButtons();

		if (m_selectedEntities.empty())
		{
			m_canvas->ClearEntitySelection();
			return;
		}

		assert(m_currentLayer);
		LayerIndex layerIndex = m_currentLayer.value();

		const Map& map = GetWorkingMap();

		std::vector<EntityId> selectedEntities;
		for (std::size_t entityIndex : m_selectedEntities)
			selectedEntities.push_back(map.GetEntity(layerIndex, entityIndex).uniqueId);

		m_canvas->EditEntitiesPosition(std::move(selectedEntities));
	}

	void EditorWindow::OnLayerChanged(int layerIndex)
	{
		if (layerIndex < 0)
		{
			m_currentLayer.reset();
			m_entityIndices.clear();
			m_canvas->UpdateActiveLayer({});
			m_canvas->UpdateBackgroundColor(Nz::Color::Black);

			m_entityList.listWidget->clear();

			m_layerMenu->setEnabled(false);
			UpdateEntityListButtons();
			UpdateLayerListButtons();
			return;
		}

		if (m_currentLayer == layerIndex)
			return;

		assert(layerIndex >= 0);
		LayerIndex layerIdx = static_cast<LayerIndex>(layerIndex);

		m_currentLayer = layerIdx;

		m_layerMenu->setEnabled(true);
		UpdateLayerListButtons();

		assert(layerIdx < m_workingMap.GetLayerCount());
		auto& layer = m_workingMap.GetLayer(layerIdx);

		if (m_showBackgroundColor->isChecked())
			m_canvas->UpdateBackgroundColor(layer.backgroundColor);

		m_entityList.listWidget->clear();

		m_canvas->UpdateActiveLayer(layerIndex);

		for (std::size_t entityIndex = 0; entityIndex < layer.entities.size(); ++entityIndex)
			RegisterEntity(entityIndex);
	}

	void EditorWindow::OnLayerMovedDown()
	{
		if (!m_currentLayer)
			return;

		LayerIndex oldPosition = m_currentLayer.value();
		if (oldPosition + 1 >= m_layerList.listWidget->count())
			return;

		LayerIndex newPosition = oldPosition + 1;
		PushCommand<Commands::LayerSwap>(oldPosition, newPosition);
	}

	void EditorWindow::OnLayerMovedUp()
	{
		if (!m_currentLayer)
			return;

		LayerIndex oldPosition = m_currentLayer.value();
		if (oldPosition == 0)
			return;

		LayerIndex newPosition = oldPosition - 1;
		PushCommand<Commands::LayerSwap>(oldPosition, newPosition);
	}

	void EditorWindow::OnMoveEntity(std::size_t entityIndex, LayerIndex targetLayer)
	{
		assert(m_currentLayer);
		PushCommand<Commands::EntityLayerUpdate>(GetWorkingMap().GetEntity(*m_currentLayer, entityIndex).uniqueId, targetLayer);
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

	void EditorWindow::OnPerspectiveSwitch(bool enable)
	{
		m_canvas->GetCamera().EnablePerspective(enable);
	}

	void EditorWindow::OnPlayMap()
	{
		const ConfigFile& config = GetConfig();

		float tickRate = config.GetFloatValue<float>("ServerSettings.TickRate");

		if (m_playWindow)
			m_playWindow->deleteLater();

		// Make a copy of the map to inject scripts
		Map scriptedMap = GetWorkingMap();
		if (!m_workingMapPath.empty())
			AddScriptsToMap(scriptedMap);

		m_playWindow = new PlayWindow(*this, scriptedMap, tickRate);
		m_playWindow->resize(1280, 720);
		m_playWindow->show();

		//m_centralTab->addTab(m_playWindow, tr("In-game test"));

		connect(m_playWindow, &QObject::destroyed, [this]()
		{
			m_playWindow = nullptr;
		});
	}

	void EditorWindow::OnReloadScripts()
	{
		m_canvas->ReloadScripts();
		RebuildCanvas();

		// Force entity info dialog update
		if (m_entityInfoDialog)
		{
			m_entityInfoDialog->deleteLater();
			m_entityInfoDialog = nullptr;
		}
	}

	void EditorWindow::OnSaveMap()
	{
		SaveMap();
	}

	void EditorWindow::OnSetAlignment()
	{
		assert(m_currentLayer);
		LayerIndex currentLayerIndex = *m_currentLayer;

		const auto& layerData = m_workingMap.GetLayer(currentLayerIndex);

		AlignmentDialog* dialog = new AlignmentDialog(layerData.positionAlignment, this);
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);

		connect(dialog, &QDialog::accepted, [=]()
		{
			Nz::Vector2f newAlignment = dialog->GetAlignment();
			OnLayerAlignmentUpdate(this, currentLayerIndex, newAlignment);

			auto& layerData = GetWorkingMapMut().GetLayer(currentLayerIndex);
			layerData.positionAlignment = newAlignment;
		});

		dialog->show();
	}

	void EditorWindow::OpenMap(const QString& mapFolder)
	{
		Map map;

		std::filesystem::path workingMapPath = std::filesystem::u8path(mapFolder.toStdString());
		try
		{
			map = Map::LoadFromDirectory(workingMapPath);
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

		if (int(entityIndex) != m_entityList.listWidget->count())
		{
			assert(int(entityIndex) < m_entityList.listWidget->count());
			m_entityList.listWidget->insertItem(int(entityIndex), item);

			for (auto it = m_entityIndices.begin(); it != m_entityIndices.end(); ++it)
			{
				if (it->second >= entityIndex)
				{
					std::size_t newEntityIndex = ++it.value();
					m_entityList.listWidget->item(int(newEntityIndex))->setData(Qt::UserRole, qulonglong(newEntityIndex));
				}
			}
		}
		else
			m_entityList.listWidget->addItem(item);

		m_entityIndices.emplace(entity.uniqueId, entityIndex);
	}

	bool EditorWindow::SaveMap()
	{
		if (m_workingMapPath.empty())
		{
			QDir mapFolder;
			QString workingPath;

			for (;;)
			{
				QString path = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly);
				if (path.isEmpty())
					return true;

				mapFolder.setPath(path);
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
		{
			m_mapDirtyFlag = false;
			m_saveMapToolbar->setEnabled(false);

			statusBar()->showMessage(tr("Map saved"), 3000);

			return true;
		}
		else
		{
			QMessageBox::warning(this, tr("Failed to save map"), tr("Failed to save map (is map folder read-only?)"), QMessageBox::Ok);
			statusBar()->showMessage(tr("Failed to save map"), 5000);

			return false;
		}
	}

	void EditorWindow::RebuildCanvas()
	{
		m_canvas->Clear();

		m_canvas->ResetLayers(m_workingMap.GetLayerCount());
		for (std::size_t i = 0; i < m_workingMap.GetLayerCount(); ++i)
		{
			const auto& layer = m_workingMap.GetLayer(LayerIndex(i));

			for (const auto& entity : layer.entities)
				m_canvas->CreateEntity(LayerIndex(i), entity.uniqueId, entity.entityType, entity.position, entity.rotation, entity.properties);
		}

		m_canvas->UpdateActiveLayer(m_currentLayer);
	}

	void EditorWindow::UpdateEntityListButtons()
	{
		if (m_selectedEntities.size() == 1)
		{
			std::size_t selectedEntity = m_selectedEntities.front();

			m_entityList.downArrowButton->setEnabled(int(selectedEntity + 1) < m_entityList.listWidget->count());
			m_entityList.upArrowButton->setEnabled(selectedEntity > 0);
		}
		else
		{
			m_entityList.downArrowButton->setEnabled(false);
			m_entityList.upArrowButton->setEnabled(false);
		}
	}

	void EditorWindow::UpdateLayerListButtons()
	{
		if (m_currentLayer)
		{
			int currentLayer = int(*m_currentLayer);
			m_layerList.downArrowButton->setEnabled(currentLayer + 1 < m_layerList.listWidget->count());
			m_layerList.upArrowButton->setEnabled(currentLayer != 0);
		}
		else
		{
			m_layerList.downArrowButton->setEnabled(false);
			m_layerList.upArrowButton->setEnabled(false);
		}
	}

	void EditorWindow::RefreshLayerList()
	{
		int currentRow = m_layerList.listWidget->currentRow();

		m_layerList.listWidget->clear();

		for (LayerIndex layerIndex = 0; layerIndex < m_workingMap.GetLayerCount(); ++layerIndex)
		{
			const auto& layer = m_workingMap.GetLayer(layerIndex);

			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(layer.name));
			item->setData(Qt::UserRole, qulonglong(layerIndex));

			m_layerList.listWidget->addItem(item);
		}

		if (currentRow >= 0 && currentRow < m_layerList.listWidget->count())
			m_layerList.listWidget->setCurrentRow(currentRow);
		else
			OnLayerChanged(-1);
	}
}
