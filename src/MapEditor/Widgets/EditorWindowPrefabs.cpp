// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindowPrefabs.hpp>
#include <CoreLib/Utils.hpp>
#include <MapEditor/Commands/EntityCommands.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/FileDescDialog.hpp>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <nlohmann/json.hpp>

namespace bw
{
	void EditorWindowPrefabs::BuildMenu(QMenuBar* menuBar)
	{
		m_prefabMenu = menuBar->addMenu(tr("&Prefab"));
		{
			QAction* createPrefab = m_prefabMenu->addAction(tr("Create prefab..."));
			connect(createPrefab, &QAction::triggered, this, &EditorWindowPrefabs::OnCreatePrefab);

			QAction* loadPrefab = m_prefabMenu->addAction(tr("Load prefab..."));
			connect(loadPrefab, &QAction::triggered, this, &EditorWindowPrefabs::OnLoadPrefab);
		}
	}

	void EditorWindowPrefabs::OnCreatePrefab()
	{
		auto layerOpt = m_parent->GetCurrentLayer();
		if (!layerOpt)
		{
			QMessageBox::critical(m_parent, tr("No active layer"), tr("You must select at least one entity in a layer before creating a prefab"));
			return;
		}

		LayerIndex layerIndex = *layerOpt;

		std::vector<std::size_t> selectedEntities = m_parent->GetSelectedEntities();
		if (selectedEntities.empty())
		{
			QMessageBox::critical(m_parent, tr("No entity selected"), tr("You must select at least one entity in a layer before creating a prefab"));
			return;
		}

		const Map& map = m_parent->GetWorkingMap();
		const auto& layer = map.GetLayer(layerIndex);

		Nz::Vector2f centerPos = Nz::Vector2f::Zero();
		std::vector<Map::Entity> entityData;
		for (std::size_t entityIndex : selectedEntities)
		{
			assert(entityIndex < layer.entities.size());
			const auto& entity = layer.entities[entityIndex];
			centerPos += entity.position;

			entityData.push_back(entity);
		}

		centerPos /= entityData.size();
		for (auto& entity : entityData)
			entity.position -= centerPos;

		FileDescDialog* createPrefabDialog = new FileDescDialog(m_parent);
		connect(createPrefabDialog, &QDialog::accepted, [this, createPrefabDialog, layerIndex, entityData = std::move(entityData)]() mutable
		{
			FileDescInfo fileDescInfo = createPrefabDialog->GetInfo();

			QString folder = QString::fromStdString("prefabs/" + fileDescInfo.name + ".prefab");

			QString path = QFileDialog::getSaveFileName(m_parent, tr("Save prefab"), folder, "Prefabs (*.prefab)");
			if (path.isEmpty())
				return;

			tsl::hopscotch_map<EntityId /*uniqueId*/, EntityId /*prefabId*/> entitiesUniqueToPrefabId;
			for (auto& entity : entityData)
			{
				EntityId prefabId = static_cast<EntityId>(entitiesUniqueToPrefabId.size() + 1);

				assert(entitiesUniqueToPrefabId.find(entity.uniqueId) == entitiesUniqueToPrefabId.end());
				entitiesUniqueToPrefabId[entity.uniqueId] = prefabId;

				entity.uniqueId = prefabId;
			}

			MapInfo prefabInfo;
			prefabInfo.author = std::move(fileDescInfo.author);
			prefabInfo.description = std::move(fileDescInfo.description);
			prefabInfo.name = std::move(fileDescInfo.name);

			Map prefab(std::move(prefabInfo));
			prefab.AddLayer();

			for (auto& entity : entityData)
				prefab.AddEntity(0, Map::PreserveUniqueId{}, std::move(entity));

			prefab.ForeachEntityPropertyValue<PropertyType::Entity>([&](Map::Entity& entity, const std::string& name, EntityId& uniqueId)
			{
				auto it = entitiesUniqueToPrefabId.find(uniqueId);
				if (it == entitiesUniqueToPrefabId.end())
				{
					//FIXME: Entity ID should not be prefab ID
					QString errMessage = tr("Entity #%1 (%2) property %3 links entity #%4 which is not part of the prefab")
					                     .arg(entity.uniqueId)
					                     .arg(QString::fromStdString(entity.name))
					                     .arg(QString::fromStdString(name))
					                     .arg(uniqueId);

					QMessageBox::warning(m_parent, tr("Prefab is not self-containing"), errMessage);
					uniqueId = 0;
				}
				else
					uniqueId = it->second;
			});

			prefab.ForeachEntityPropertyValue<PropertyType::Layer>([&](Map::Entity& entity, const std::string& name, LayerIndex& layerId)
			{
				//FIXME: Entity ID should not be prefab ID
				QString errMessage = tr("Entity #%1 (%2) property %3 links to layer #%4 which is not part of the prefab")
				                     .arg(entity.uniqueId)
				                     .arg(QString::fromStdString(entity.name))
				                     .arg(QString::fromStdString(name))
				                     .arg(layerId);

				QMessageBox::warning(m_parent, tr("Prefab is not self-containing"), errMessage);
				layerId = 0;
			});

			std::string content = Map::Serialize(prefab).dump(1, '\t');

			Nz::File infoFile(path.toStdString(), Nz::OpenMode::Write | Nz::OpenMode::Truncate);
			if (!infoFile.IsOpen())
			{
				QMessageBox::critical(m_parent, tr("Failed to save prefab"), tr("An error occurred while opening the prefab file"));
				return;
			}

			if (infoFile.Write(content.data(), content.size()) != content.size())
			{
				QMessageBox::critical(m_parent, tr("Failed to save prefab"), tr("An error occurred while writing the prefab file"));
				return;
			}
		});
		createPrefabDialog->open();
	}

	void EditorWindowPrefabs::OnLoadPrefab()
	{
		auto layerOpt = m_parent->GetCurrentLayer();
		if (!layerOpt)
		{
			QMessageBox::critical(m_parent, tr("No active layer"), tr("You must select one layer before loading a prefab"));
			return;
		}

		LayerIndex layerIndex = *layerOpt;

		QString prefabPath = QFileDialog::getOpenFileName(m_parent, tr("Load prefab"), "prefabs/", "Prefabs (*.prefab)");
		if (prefabPath.isEmpty())
			return;

		Nz::File infoFile(prefabPath.toStdString(), Nz::OpenMode::Read);
		if (!infoFile.IsOpen())
		{
			QMessageBox::critical(m_parent, tr("Failed to load prefab"), tr("An error occurred while opening the prefab file"));
			return;
		}

		std::vector<Nz::UInt8> content(infoFile.GetSize());
		if (infoFile.Read(content.data(), content.size()) != content.size())
		{
			QMessageBox::critical(m_parent, tr("Failed to load prefab"), tr("An error occurred while reading the prefab file"));
			return;
		}

		Map prefab;
		try
		{
			nlohmann::json prefabJson = nlohmann::json::parse(content.begin(), content.end());
			prefab = Map::Unserialize(prefabJson);

			if (prefab.GetLayerCount() != 1)
				throw std::logic_error("one layer expected, " + std::to_string(prefab.GetLayerCount()) + " found");
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(m_parent, tr("Failed to load prefab"), tr("An error occurred while parsing the prefab file: %1").arg(e.what()));
			return;
		}

		Map& map = m_parent->GetWorkingMapMut();
		const auto& layer = map.GetLayer(layerIndex);

		auto& prefabLayer = prefab.GetLayer(0);

		std::size_t entityIndex = layer.entities.size();
		Nz::Vector2f positionOffset = AlignPosition(m_parent->GetCameraCenter(), layer.positionAlignment);

		tsl::hopscotch_map<EntityId /*prefabId*/, EntityId /*uniqueId*/> prefabIdToEntitiesUniqueId;

		for (auto& entityData : prefabLayer.entities)
		{
			EntityId newUniqueId = map.GenerateUniqueId();
			assert(prefabIdToEntitiesUniqueId.find(entityData.uniqueId) == prefabIdToEntitiesUniqueId.end());
			prefabIdToEntitiesUniqueId[entityData.uniqueId] = newUniqueId;

			entityData.position += positionOffset;
			entityData.uniqueId = newUniqueId;
		}

		prefab.ForeachEntityPropertyValue<PropertyType::Entity>([&](Map::Entity& /*entity*/, const std::string& /*name*/, EntityId& uniqueId)
		{
			auto it = prefabIdToEntitiesUniqueId.find(uniqueId);
			if (it == prefabIdToEntitiesUniqueId.end())
			{
				//TODO: Output warning
				uniqueId = 0;
			}
			else
				uniqueId = it->second;
		});

		std::vector<std::size_t> entityIndices(prefabLayer.entities.size());
		std::iota(entityIndices.begin(), entityIndices.end(), entityIndex);

		m_parent->PushCommand<Commands::PrefabInstantiate>(Map::EntityIndices{ layerIndex, layer.entities.size() }, std::move(prefabLayer.entities));

		m_parent->SelectEntities(std::move(entityIndices));
	}
}
