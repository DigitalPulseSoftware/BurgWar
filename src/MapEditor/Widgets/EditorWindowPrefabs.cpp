// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EditorWindowPrefabs.hpp>
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
		connect(createPrefabDialog, &QDialog::accepted, [this, createPrefabDialog, layerIndex, entityData = std::move(entityData)]()
		{
			FileDescInfo fileDescInfo = createPrefabDialog->Getnfo();

			QString folder = QString::fromStdString("prefabs/" + fileDescInfo.name + ".prefab");

			QString path = QFileDialog::getSaveFileName(m_parent, tr("Save prefab"), folder, "Prefabs (*.prefab)");
			if (path.isEmpty())
				return;

			nlohmann::json prefabInfo;
			prefabInfo["name"] = fileDescInfo.name;
			prefabInfo["author"] = fileDescInfo.author;
			prefabInfo["description"] = fileDescInfo.description;

			auto entityArray = nlohmann::json::array();
			for (auto& entity : entityData)
			{
				nlohmann::json entityInfo = Map::SerializeEntity(entity);
				entityInfo.erase("uniqueId"); //< Don't store unique id as part of the prefab

				entityArray.emplace_back(std::move(entityInfo));
			}

			prefabInfo["entities"] = std::move(entityArray);

			std::string content = prefabInfo.dump(1, '\t');

			Nz::File infoFile(path.toStdString(), Nz::OpenMode_WriteOnly | Nz::OpenMode_Truncate);
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

		Nz::File infoFile(prefabPath.toStdString(), Nz::OpenMode_ReadOnly);
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

		std::vector<Map::Entity> entities;
		try
		{
			nlohmann::json prefabJson = nlohmann::json::parse(content.begin(), content.end());

			for (auto&& entityInfo : prefabJson["entities"])
				entities.emplace_back(Map::UnserializeEntity(entityInfo));
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(m_parent, tr("Failed to load prefab"), tr("An error occurred while parsing the prefab file: %1").arg(e.what()));
			return;
		}

		Map& map = m_parent->GetWorkingMapMut();
		const auto& layer = map.GetLayer(layerIndex);

		Nz::Vector2f positionOffset = AlignPosition(m_parent->GetCameraCenter(), layer.positionAlignment);

		std::size_t entityIndex = layer.entities.size();
		std::vector<std::size_t> entityIndices;
		entityIndices.reserve(entities.size());

		for (auto& entityData : entities)
		{
			entityData.position += positionOffset;
			entityData.uniqueId = map.GenerateUniqueId();
			entityIndices.push_back(entityIndex++);
		}

		m_parent->PushCommand<Commands::PrefabInstantiate>(Map::EntityIndices{ layerIndex, layer.entities.size() }, std::move(entities));

		m_parent->SelectEntities(std::move(entityIndices));
	}
}
