// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_ENTITYINFODIALOG_HPP

#include <Nazara/Core/Flags.hpp>
#include <NDK/Entity.hpp>
#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Enums.hpp>
#include <QtWidgets/QDialog>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <filesystem>
#include <functional>
#include <optional>

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLayout;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextEdit;
class QWidget;

namespace bw
{
	class EditorEntityStore;
	class ScriptingContext;
	class Float2SpinBox;
	class Logger;

	struct EntityInfo
	{
		std::string entityClass;
		std::string entityName;
		Nz::DegreeAnglef rotation = Nz::DegreeAnglef::Zero();
		Nz::Vector2f position = Nz::Vector2f::Zero();
		PropertyValueMap properties;
	};

	class EntityInfoDialog : public QDialog
	{
		public:
			using Callback = std::function<void(EntityInfoDialog* dialog, EntityInfo&& entityInfo, EntityInfoUpdateFlags updateFlags)>;

			EntityInfoDialog(const Logger& logger, const Map& map, EditorEntityStore& clientEntityStore, ScriptingContext& scriptingContext, QWidget* parent = nullptr);
			~EntityInfoDialog();

			inline const Nz::Vector2f& GetPosition() const;
			inline const Nz::DegreeAnglef& GetRotation() const;

			const PropertyValue& GetProperty(const std::string& propertyName) const;
			std::pair<PropertyType, bool> GetPropertyType(const std::string& propertyName) const;
			inline const Ndk::EntityHandle& GetTargetEntity() const;

			void Open(EntityId uniqueId, std::optional<EntityInfo> info, const Ndk::EntityHandle& targetEntity, Callback callback);

			void UpdatePosition(const Nz::Vector2f& position);
			void UpdateRotation(const Nz::DegreeAnglef& rotation);
			void UpdateProperty(const std::string& propertyName, PropertyValue propertyValue);

		private:
			struct Delegates;
			struct PropertyData;

			template<PropertyType P> friend struct PropertyWidgets;
			template<bool IsArray> friend struct PropertyOverrider;

			using EntityPropertyConstRefOpt = std::optional<std::reference_wrapper<const PropertyValue>>;

			std::vector<std::pair<QString, QVariant>> BuildEntityComboBoxOptions();
			std::vector<std::pair<QString, QVariant>> BuildLayerComboBoxOptions();

			EntityPropertyConstRefOpt GetProperty(const PropertyData& property) const;

			void OnAccept();
			void OnEntityTypeUpdate();
			void OnResetProperty();

			void RefreshEntityType();
			void RefreshPropertyEditor(std::size_t propertyIndex);

			QString ToString(bool value, PropertyType type);
			QString ToString(float value, PropertyType type);
			QString ToString(Nz::UInt16 value, PropertyType type);
			QString ToString(Nz::Int64 value, PropertyType type);
			QString ToString(const Nz::Vector2f& value, PropertyType type);
			QString ToString(const Nz::Vector2i64& value, PropertyType type);
			QString ToString(const Nz::Vector3f& value, PropertyType type);
			QString ToString(const Nz::Vector3i64& value, PropertyType type);
			QString ToString(const Nz::Vector4f& value, PropertyType type);
			QString ToString(const Nz::Vector4i64& value, PropertyType type);
			QString ToString(const std::string& value, PropertyType type);
			QString ToString(EntityPropertyConstRefOpt property, PropertyType type);

			static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

			struct PropertyData
			{
				std::size_t index;
				std::string keyName;
				std::string visualName;
				std::optional<PropertyValue> defaultValue;
				PropertyType type;
				bool isArray;
			};

			Ndk::EntityHandle m_targetEntity;
			EntityId m_entityUniqueId;
			std::size_t m_entityTypeIndex;
			std::size_t m_propertyTypeIndex;
			std::vector<PropertyData> m_properties;
			std::vector<std::string> m_entityTypes;
			std::unique_ptr<Delegates> m_delegates;
			tsl::hopscotch_map<std::string, std::size_t> m_editorActionByName;
			tsl::hopscotch_map<std::string, std::size_t> m_propertyByName;
			Callback m_callback;
			EditorEntityStore& m_entityStore;
			EntityInfoUpdateFlags m_updateFlags;
			const Logger& m_logger;
			const Map& m_map;
			ScriptingContext& m_scriptingContext;
			EntityInfo m_entityInfo;
			QComboBox* m_entityTypeWidget;
			Float2SpinBox* m_positionWidget;
			QDoubleSpinBox* m_rotationWidget;
			QLabel* m_entityIndexLabel;
			QLabel* m_propertyTitle;
			QLabel* m_propertyDescription;
			QLayout* m_editorActionLayout;
			QLineEdit* m_nameWidget;
			QPushButton* m_resetDefaultButton;
			QWidget* m_editorActionWidget;
			QWidget* m_propertyContentWidget;
			QTableWidget* m_propertiesList;
	};
}


#include <MapEditor/Widgets/EntityInfoDialog.inl>

#endif
