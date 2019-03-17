// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP

#include <NDK/Entity.hpp>
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
			EntityInfoDialog(ClientEntityStore& clientEntityStore, ClientScriptingContext& scriptingContext, const Ndk::EntityHandle& targetEntity, EntityInfo entityInfo, QWidget* parent = nullptr);
			~EntityInfoDialog() = default;

			inline const EntityInfo& GetEntityInfo() const;
			inline const Nz::Vector2f& GetEntityPosition() const;
			inline const Nz::DegreeAnglef& GetEntityRotation() const;

			const EntityProperty& GetProperty(const std::string& propertyName) const;
			std::pair<PropertyType, bool> GetPropertyType(const std::string& propertyName) const;

			inline const Ndk::EntityHandle& GetTargetEntity() const;

			void SetEntityPosition(const Nz::Vector2f& position);
			void SetEntityRotation(const Nz::DegreeAnglef& rotation);
			void SetProperty(const std::string& propertyName, EntityProperty propertyValue);

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

			Ndk::EntityHandle m_targetEntity;
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