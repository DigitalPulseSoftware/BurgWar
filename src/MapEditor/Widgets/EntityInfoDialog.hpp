// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Map.hpp>
#include <QtWidgets/QDialog>
#include <tsl/hopscotch_map.h>
#include <filesystem>

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLayout;
class QLineEdit;
class QTableWidget;
class QTextEdit;
class QWidget;

namespace bw
{
	class ClientEntityStore;
	class ClientScriptingContext;
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
			EntityInfoDialog(ClientEntityStore& clientEntityStore, ClientScriptingContext& scriptingContext, QWidget* parent = nullptr);
			EntityInfoDialog(ClientEntityStore& clientEntityStore, ClientScriptingContext& scriptingContext, EntityInfo entityInfo, QWidget* parent = nullptr);
			~EntityInfoDialog() = default;

			const EntityInfo& GetEntityInfo() const;

		private:
			void OnAccept();
			void OnEntityTypeUpdate();

			void RefreshEntityType();
			void RefreshPropertyEditor(std::size_t propertyIndex);

			static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

			struct PropertyData
			{
				std::string keyName;
				std::string visualName;
				EntityProperty defaultValue;
				PropertyType type;
				bool isArray;
			};

			std::size_t m_entityTypeIndex;
			std::size_t m_propertyTypeIndex;
			std::vector<PropertyData> m_properties;
			std::vector<std::string> m_entityTypes;
			tsl::hopscotch_map<std::string, std::size_t> m_editorActionByName;
			tsl::hopscotch_map<std::string, std::size_t> m_propertyByName;
			ClientEntityStore& m_entityStore;
			ClientScriptingContext& m_scriptingContext;
			EntityInfo m_entityInfo;
			QComboBox* m_entityTypeWidget;
			PositionEditWidget* m_positionWidget;
			QDoubleSpinBox* m_rotationWidget;
			QLabel* m_propertyTitle;
			QLabel* m_propertyDescription;
			QLayout* m_editorActionLayout;
			QLineEdit* m_nameWidget;
			QWidget* m_editorActionWidget;
			QWidget* m_propertyContentWidget;
			QTableWidget* m_propertiesList;
	};
}


#include <MapEditor/Widgets/EntityInfoDialog.inl>

#endif