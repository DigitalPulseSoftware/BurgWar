// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Map.hpp>
#include <QtWidgets/QDialog>
#include <hopscotch/hopscotch_map.h>
#include <filesystem>

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QTableWidget;
class QTextEdit;
class QWidget;

namespace bw
{
	class ClientEntityStore;
	class PositionEditWidget;

	struct EntityInfo
	{
		std::string entityClass;
		std::string entityName;
		Nz::DegreeAnglef rotation;
		Nz::Vector2f position;
		EntityProperties properties;
	};

	class EntityInfoDialog : public QDialog
	{
		public:
			EntityInfoDialog(ClientEntityStore& clientEntityStore, QWidget* parent = nullptr);
			EntityInfoDialog(ClientEntityStore& clientEntityStore, EntityInfo entityInfo, QWidget* parent = nullptr);
			~EntityInfoDialog() = default;

			const EntityInfo& GetEntityInfo() const;

		private:
			void OnEntityTypeUpdate();

			void RefreshEntityType();
			void RefreshPropertyEditor(int propertyIndex);

			void OnAccept();

			struct PropertyData
			{
				std::string keyName;
				std::string visualName;
				PropertyType type;
			};

			std::size_t m_entityTypeIndex;
			std::vector<PropertyData> m_properties;
			std::vector<std::string> m_entityTypes;
			ClientEntityStore& m_entityStore;
			EntityInfo m_entityInfo;
			QComboBox* m_entityTypeWidget;
			PositionEditWidget* m_positionWidget;
			QDoubleSpinBox* m_rotationWidget;
			QLabel* m_propertyTitle;
			QLabel* m_propertyDescription;
			QLineEdit* m_nameWidget;
			QWidget* m_propertyContentWidget;
			QTableWidget* m_propertiesList;
	};
}


#include <MapEditor/Widgets/EntityInfoDialog.inl>

#endif