// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <MapEditor/Scripting/EditorEntityStore.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <MapEditor/Widgets/Integer2SpinBox.hpp>
#include <Nazara/Core/TypeTag.hpp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <bitset>
#include <limits>

namespace bw
{
	class ComboBoxPropertyDelegate : public QStyledItemDelegate
	{
		public:
			ComboBoxPropertyDelegate(std::vector<QString> options) :
			m_options(std::move(options))
			{
			}

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, int value) const
			{
				model->setData(index, value, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				QComboBox* editor = new QComboBox(parent);
				editor->setFrame(false);

				for (const QString& option : m_options)
					editor->addItem(option);

				return editor;
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				return m_options[value.toInt()];
			}

			int RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).toInt();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QComboBox* comboBox = static_cast<QComboBox*>(editor);
				comboBox->setCurrentIndex(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QComboBox* comboBox = static_cast<QComboBox*>(editor);

				ApplyModelData(model, index, comboBox->currentIndex());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}

		private:
			std::vector<QString> m_options;
	};

	class FloatPropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, float value) const
			{
				model->setData(index, value, Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

				return editor;
			}

			float RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).toFloat();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->interpretText();

				ApplyModelData(model, index, float(spinBox->value()));
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Float2PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector2f& value) const
			{
				model->setData(index, QPointF(value.x, value.y), Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				return new Float2SpinBox(Float2SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				QPointF point = value.toPointF();
				return QString("(%1; %2)").arg(locale.toString(point.x())).arg(locale.toString(point.y()));
			}

			Nz::Vector2f RetrieveModelData(const QModelIndex& index) const
			{
				QPointF point = index.model()->data(index, Qt::EditRole).toPointF();
				return { float(point.x()), float(point.y()) };
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Float2SpinBox* spinBox = static_cast<Float2SpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Float2SpinBox* spinBox = static_cast<Float2SpinBox*>(editor);

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	class IntegerPropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, Nz::Int64 value) const
			{
				model->setData(index, int(value), Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				//TODO: Subclass for int64
				QSpinBox* editor = new QSpinBox(parent);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

				return editor;
			}

			Nz::Int64 RetrieveModelData(const QModelIndex& index) const
			{
				return index.model()->data(index, Qt::EditRole).toInt();
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->setValue(RetrieveModelData(index));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->interpretText();

				ApplyModelData(model, index, spinBox->value());
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}
	};
	
	class Integer2PropertyDelegate : public QStyledItemDelegate
	{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void ApplyModelData(QAbstractItemModel* model, const QModelIndex& index, const Nz::Vector2i64& value) const
			{
				model->setData(index, QPoint(value.x, value.y), Qt::EditRole);
			}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				return new Integer2SpinBox(Integer2SpinBox::LabelMode::NoLabel, QBoxLayout::LeftToRight, parent);
			}

			QString displayText(const QVariant& value, const QLocale& locale) const override
			{
				QPoint point = value.toPoint();
				return QString("(%1; %2)").arg(locale.toString(point.x())).arg(locale.toString(point.y()));
			}

			Nz::Vector2i64 RetrieveModelData(const QModelIndex& index) const
			{
				QPoint point = index.model()->data(index, Qt::EditRole).toPoint();
				return { Nz::Int64(point.x()), Nz::Int64(point.y()) };
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				Integer2SpinBox* spinBox = static_cast<Integer2SpinBox*>(editor);
				spinBox->setValue(Nz::Vector2i(RetrieveModelData(index)));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				Integer2SpinBox* spinBox = static_cast<Integer2SpinBox*>(editor);

				ApplyModelData(model, index, Nz::Vector2i64(spinBox->value()));
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	struct EntityInfoDialog::Delegates
	{
		std::optional<ComboBoxPropertyDelegate> comboBoxDelegate;
		FloatPropertyDelegate floatDelegate;
		Float2PropertyDelegate float2Delegate;
		IntegerPropertyDelegate intDelegate;
		Integer2PropertyDelegate int2Delegate;
	};

	EntityInfoDialog::EntityInfoDialog(const Logger& logger, const Map& map, EditorEntityStore& clientEntityStore, ScriptingContext& scriptingContext, QWidget* parent) :
	QDialog(parent),
	m_entityTypeIndex(0),
	m_propertyTypeIndex(InvalidIndex),
	m_entityStore(clientEntityStore),
	m_logger(logger),
	m_map(map),
	m_scriptingContext(scriptingContext)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		m_delegates = std::make_unique<Delegates>();

		m_entityTypeWidget = new QComboBox;
		m_entityStore.ForEachElement([this](const ScriptedEntity& entityData)
		{
			m_entityTypes.emplace_back(entityData.fullName);
		});

		std::sort(m_entityTypes.begin(), m_entityTypes.end());

		for (std::size_t i = 0; i < m_entityTypes.size(); ++i)
			m_entityTypeWidget->addItem(QString::fromStdString(m_entityTypes[i]));

		connect(m_entityTypeWidget, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) 
		{
			OnEntityTypeUpdate();
		});

		QHBoxLayout* propertyLayout = new QHBoxLayout;

		m_propertiesList = new QTableWidget(0, 3);
		m_propertiesList->setHorizontalHeaderLabels({ tr("Property"), tr("Value"), tr("Required") });
		m_propertiesList->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_propertiesList->setSelectionMode(QAbstractItemView::SingleSelection);
		m_propertiesList->setShowGrid(false);
		m_propertiesList->setEditTriggers(QAbstractItemView::NoEditTriggers);


		connect(m_propertiesList, &QTableWidget::currentCellChanged, [this](int currentRow, int currentColumn, int previousRow, int previousColumn)
		{
			if (currentRow < 0 || currentRow == previousRow)
				return;

			RefreshPropertyEditor(currentRow);
		});

		QWidget* propertyWidget = new QWidget;

		m_propertyTitle = new QLabel;
		m_propertyDescription = new QLabel;

		QVBoxLayout* propertyContentLayout = new QVBoxLayout;

		m_propertyContentWidget = new QWidget;
		m_propertyContentWidget->setMinimumSize(320, 320);

		m_resetDefaultButton = new QPushButton(tr("Restore defaults"));
		connect(m_resetDefaultButton, &QPushButton::released, [this]() { OnResetProperty(); });

		propertyContentLayout->addWidget(m_propertyTitle);
		propertyContentLayout->addWidget(m_propertyDescription);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(m_propertyContentWidget);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(m_resetDefaultButton);

		propertyLayout->addWidget(m_propertiesList);
		propertyLayout->addLayout(propertyContentLayout);

		m_nameWidget = new QLineEdit;
		connect(m_nameWidget, &QLineEdit::textEdited, [this](const QString& text)
		{
			m_entityInfo.entityName = text.toStdString();
		});

		m_positionWidget = new Float2SpinBox(Float2SpinBox::LabelMode::PositionLabel, QBoxLayout::LeftToRight);
		connect(m_positionWidget, &Float2SpinBox::valueChanged, [this](Nz::Vector2f value)
		{
			m_entityInfo.position = value;
		});

		m_rotationWidget = new QDoubleSpinBox;
		m_rotationWidget->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_rotationWidget, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double rotation)
		{
			m_entityInfo.rotation = Nz::DegreeAnglef::FromDegrees(float(rotation));
			m_entityInfo.rotation.Normalize();
		});

		QFormLayout* genericPropertyLayout = new QFormLayout;
		genericPropertyLayout->addRow(tr("Entity type"), m_entityTypeWidget);
		genericPropertyLayout->addRow(tr("Entity name"), m_nameWidget);
		genericPropertyLayout->addRow(tr("Entity position"), m_positionWidget);
		genericPropertyLayout->addRow(tr("Entity rotation"), m_rotationWidget);

		QDialogButtonBox* button = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(button, &QDialogButtonBox::accepted, this, &EntityInfoDialog::OnAccept);
		connect(button, &QDialogButtonBox::rejected, this, &QDialog::reject);

		m_editorActionWidget = new QWidget;
		m_editorActionLayout = new QHBoxLayout;
		m_editorActionWidget->setLayout(m_editorActionLayout);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(genericPropertyLayout);
		verticalLayout->addLayout(propertyLayout);
		verticalLayout->addWidget(m_editorActionWidget);
		verticalLayout->addWidget(button);

		setLayout(verticalLayout);

		connect(this, &QDialog::finished, [this](int result)
		{
			if (m_callback)
			{
				if (result == QDialog::Accepted)
					m_callback(this);

				m_callback = Callback();
			}
		});

		setWindowTitle("Entity editor");
		hide();
	}

	EntityInfoDialog::~EntityInfoDialog() = default;

	const EntityProperty& EntityInfoDialog::GetProperty(const std::string& propertyName) const
	{
		const bw::EntityProperty* property;

		auto it = m_entityInfo.properties.find(propertyName);
		if (it != m_entityInfo.properties.end())
			property = &it->second;
		else
		{
			auto propertyIt = m_propertyByName.find(propertyName);
			if (propertyIt == m_propertyByName.end())
				throw std::runtime_error("Property " + propertyName + " does not exist");

			const auto& propertyData = m_properties[propertyIt->second];
			if (!propertyData.defaultValue)
				throw std::runtime_error("Property " + propertyName + " has value nor default value");

			property = &propertyData.defaultValue.value();
		}

		return *property;
	}

	std::pair<PropertyType, bool> EntityInfoDialog::GetPropertyType(const std::string & propertyName) const
	{
		auto propertyIt = m_propertyByName.find(propertyName);
		if (propertyIt == m_propertyByName.end())
			throw std::runtime_error("Property " + propertyName + " does not exist");

		const auto& propertyData = m_properties[propertyIt->second];
		return std::make_pair(propertyData.type, propertyData.isArray);
	}

	void EntityInfoDialog::Open(std::optional<EntityInfo> info, const Ndk::EntityHandle& targetEntity, Callback callback)
	{
		m_callback = std::move(callback);
		m_targetEntity = targetEntity;

		if (info)
		{
			// Editing mode
			m_entityInfo = std::move(*info);

			m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
			m_positionWidget->setValue(m_entityInfo.position);
			m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());

			QString oldClass = m_entityTypeWidget->currentText();
			QString newClass = QString::fromStdString(m_entityInfo.entityClass);
			if (oldClass != newClass)
			{
				if (!newClass.isEmpty())
					m_entityTypeWidget->setCurrentText(newClass);
				else
					m_entityTypeWidget->setCurrentIndex(-1);
			}
			else
				OnEntityTypeUpdate();
		}
		else
		{
			// Creation mode
			m_entityInfo = EntityInfo();

			m_entityTypeWidget->setCurrentIndex(-1);
			m_propertiesList->clearSelection();
		}

		m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
		m_positionWidget->setValue(m_entityInfo.position);
		m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());

		QDialog::open();
	}

	void EntityInfoDialog::UpdatePosition(const Nz::Vector2f& position)
	{
		m_entityInfo.position = position;
		m_positionWidget->setValue(position);
	}

	void EntityInfoDialog::UpdateRotation(const Nz::DegreeAnglef& rotation)
	{
		m_entityInfo.rotation = rotation;
		m_rotationWidget->setValue(rotation.ToDegrees());
	}

	void EntityInfoDialog::UpdateProperty(const std::string& propertyName, EntityProperty propertyValue)
	{
		m_entityInfo.properties.insert_or_assign(propertyName, std::move(propertyValue));

		// Check if we should reload panel
		auto propertyIt = m_propertyByName.find(propertyName);
		assert(propertyIt != m_propertyByName.end());

		if (m_propertyTypeIndex == propertyIt->second)
			RefreshPropertyEditor(m_propertyTypeIndex);
	}

	void EntityInfoDialog::OnEntityTypeUpdate()
	{
		std::string entityType = m_entityTypeWidget->currentText().toStdString();

		m_entityTypeIndex = m_entityStore.GetElementIndex(entityType);

		std::string propertyName;
		if (m_propertyTypeIndex != InvalidIndex)
			propertyName = m_properties[m_propertyTypeIndex].keyName;

		RefreshEntityType();

		if (!propertyName.empty())
		{
			auto propertyIt = m_propertyByName.find(propertyName);
			if (propertyIt != m_propertyByName.end())
				m_propertiesList->selectRow(int(propertyIt->second));
			else
				RefreshPropertyEditor(InvalidIndex);
		}
		else
			RefreshPropertyEditor(InvalidIndex);

		m_entityInfo.entityClass = (m_entityTypeIndex != m_entityStore.InvalidIndex) ? std::move(entityType) : std::string();
	}

	void EntityInfoDialog::OnResetProperty()
	{
		if (m_propertyTypeIndex == InvalidIndex)
			return;

		assert(m_propertyTypeIndex < m_properties.size());
		const auto& propertyInfo = m_properties[m_propertyTypeIndex];
		m_entityInfo.properties.erase(propertyInfo.keyName);

		QTableWidgetItem* item = m_propertiesList->item(int(m_propertyTypeIndex), 1);
		item->setFont(QFont());
		item->setText(ToString(GetProperty(propertyInfo), propertyInfo.type));

		RefreshPropertyEditor(m_propertyTypeIndex);
	}

	void EntityInfoDialog::RefreshEntityType()
	{
		m_editorActionByName.clear();
		m_properties.clear();
		m_propertyByName.clear();
		m_propertiesList->clearContents();

		if (m_entityTypeIndex == m_entityStore.InvalidIndex)
		{
			m_entityInfo.properties.clear();
			m_propertiesList->setRowCount(0);
			return;
		}

		auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore.GetElement(m_entityTypeIndex));

		// Build property list and ensure relevant properties are stored
		EntityProperties oldProperties = std::move(m_entityInfo.properties);
		m_entityInfo.properties.clear(); // Put back in a valid state

		std::bitset<MaxPropertyCount> modifiedProperties;

		for (const auto& [propertyName, propertyInfo] : entityTypeInfo->properties)
		{
			auto& propertyData = m_properties.emplace_back();
			propertyData.defaultValue = propertyInfo.defaultValue;
			propertyData.index = propertyInfo.index;
			propertyData.isArray = propertyInfo.isArray;
			propertyData.keyName = propertyName;
			propertyData.visualName = propertyData.keyName; //< FIXME
			propertyData.type = propertyInfo.type;

			if (auto it = oldProperties.find(propertyData.keyName); it != oldProperties.end())
			{
				// Only keep old property value if types are compatibles
				if (it->second.index() == propertyInfo.defaultValue->index())
				{
					assert(propertyData.index < modifiedProperties.size());
					modifiedProperties[propertyData.index] = true;
					m_entityInfo.properties.emplace(std::move(it.key()), std::move(it.value()));
				}

				oldProperties.erase(it);
			}
		}

		std::sort(m_properties.begin(), m_properties.end(), [](auto&& first, auto&& second) { return first.index < second.index; });

		for (std::size_t i = 0; i < m_properties.size(); ++i)
			m_propertyByName.emplace(m_properties[i].keyName, i);

		m_propertiesList->setRowCount(int(m_properties.size()));

		QFont boldFont;
		boldFont.setWeight(QFont::Medium);

		int rowIndex = 0;
		for (const auto& propertyInfo : m_properties)
		{
			m_propertiesList->setItem(rowIndex, 0, new QTableWidgetItem(QString::fromStdString(propertyInfo.visualName)));

			QTableWidgetItem* valueItem = new QTableWidgetItem(ToString(GetProperty(propertyInfo), propertyInfo.type));
			if (modifiedProperties.test(propertyInfo.index))
				valueItem->setFont(boldFont);
			
			m_propertiesList->setItem(rowIndex, 1, valueItem);
			m_propertiesList->setItem(rowIndex, 2, new QTableWidgetItem( propertyInfo.defaultValue.has_value() ? "" : "*" ));

			++rowIndex;
		}


		while (QWidget* w = m_editorActionWidget->findChild<QWidget*>())
			delete w;

		std::size_t actionIndex = 0;
		for (auto&& editorAction : entityTypeInfo->editorActions)
		{
			m_editorActionByName.emplace(editorAction.name, actionIndex++);

			QPushButton* button = new QPushButton(QString::fromStdString(editorAction.label));
			connect(button, &QPushButton::released, [this, name = editorAction.name]()
			{
				auto it = m_editorActionByName.find(name);
				assert(it != m_editorActionByName.end());

				auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore.GetElement(m_entityTypeIndex));

				const auto& action = entityTypeInfo->editorActions[it->second];

				auto result = action.onTrigger(this);
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(m_logger, LogLevel::Error, "Editor action {0}::OnTrigger failed: {1}", name, err.what());
				}
			});

			m_editorActionLayout->addWidget(button);

			actionIndex++;
		}
	}

	void EntityInfoDialog::RefreshPropertyEditor(std::size_t propertyIndex)
	{
		while (QWidget* w = m_propertyContentWidget->findChild<QWidget*>())
			delete w;

		delete m_propertyContentWidget->layout();

		m_propertyTypeIndex = propertyIndex;
		if (m_propertyTypeIndex == InvalidIndex)
			return;

		const auto& entityTypeInfo = m_entityStore.GetElement(m_entityTypeIndex);

		assert(propertyIndex < m_properties.size());
		const auto& propertyInfo = m_properties[propertyIndex];
		EntityPropertyConstRefOpt propertyValue = GetProperty(propertyInfo);

		m_propertyTitle->setText(QString::fromStdString(propertyInfo.visualName));

		// Only allow to reset if a default value exists
		m_resetDefaultButton->setEnabled(propertyInfo.defaultValue.has_value());

		QVBoxLayout* layout = new QVBoxLayout;

		bool isArray = propertyInfo.isArray;
		int arraySize = 0;

		if (propertyValue)
		{
			std::visit([&](auto&& propertyValue)
			{
				constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, std::decay_t<decltype(propertyValue)>>;

				isArray = IsArray;
				if constexpr (IsArray)
					arraySize = int(propertyValue.size());

			}, propertyValue->get());
		}

		auto OnPropertyOverride = [this, propertyIndex]()
		{
			QFont boldFont;
			boldFont.setWeight(QFont::Medium);

			m_propertiesList->item(int(propertyIndex), 1)->setFont(boldFont);
		};

		auto UpdatePropertyPreview = [this, propertyIndex](const QString& preview)
		{
			m_propertiesList->item(int(propertyIndex), 1)->setText(preview);
		};

		if (isArray)
		{
			QSpinBox* spinbox = new QSpinBox;
			spinbox->setRange(0, std::numeric_limits<int>::max());
			spinbox->setValue(arraySize);

			QPushButton* updateButton = new QPushButton(tr("Update"));
			connect(updateButton, &QPushButton::released, [this, &propertyInfo, spinbox, propertyIndex, UpdatePropertyPreview]()
			{
				std::size_t newSize = spinbox->value();

				// Ensure we have a custom value for this
				EntityProperty* property;
				if (auto it = m_entityInfo.properties.find(propertyInfo.keyName); it != m_entityInfo.properties.end())
					property = &it.value();
				else
				{
					if (propertyInfo.defaultValue)
					{
						const EntityProperty& defaultValue = propertyInfo.defaultValue.value();

						auto it = m_entityInfo.properties.emplace(propertyInfo.keyName, defaultValue);
						property = &it.first.value();
					}
					else
						property = nullptr;
				}

				if (property)
				{
					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
						using PropertyType = std::conditional_t<IsArray, typename IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

						// We have to use if constexpr here because the compiler will instantiate this lambda even for single types
						assert(IsArray);
						if constexpr (IsArray) //< always true
						{
							EntityPropertyArray<PropertyType> newArray(newSize);

							// Copy old values
							std::size_t size = std::min(newArray.size(), propertyValue.size());
							for (std::size_t i = 0; i < size; ++i)
								newArray[i] = propertyValue[i];

							propertyValue = std::move(newArray);
						}

					}, *property);
				}

				RefreshPropertyEditor(propertyIndex);
				UpdatePropertyPreview(ToString(*property, propertyInfo.type));
			});

			QHBoxLayout* arraySizeLayout = new QHBoxLayout;
			arraySizeLayout->addWidget(spinbox);
			arraySizeLayout->addWidget(updateButton);

			layout->addLayout(arraySizeLayout);
			
			// Waiting for template lambda in C++20
			auto SetProperty = [this, keyName = propertyInfo.keyName, arraySize, OnPropertyOverride](int rowIndex, auto&& value)
			{
				using T = std::decay_t<decltype(value)>;
				using ArrayType = EntityPropertyArray<T>;

				auto it = m_entityInfo.properties.find(keyName);
				if (it == m_entityInfo.properties.end())
				{
					it = m_entityInfo.properties.emplace(keyName, ArrayType(arraySize)).first;
					OnPropertyOverride();
				}

				ArrayType& propertyArray = std::get<ArrayType>(it.value());
				propertyArray[rowIndex] = std::forward<decltype(value)>(value);
			};

			switch (propertyInfo.type)
			{
				case PropertyType::Bool:
				{
					using T = EntityPropertyArray<bool>;

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Enabled") });

					for (int i = 0; i < arraySize; ++i)
					{
						QStandardItem* item = new QStandardItem(1);
						item->setCheckable(true);

						model->setItem(i, 0, item);
					}

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());
						for (int i = 0; i < arraySize; ++i)
							model->item(i)->setCheckState((propertyArray[i]) ? Qt::Checked : Qt::Unchecked);
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						SetProperty(item->index().row(), item->checkState() == Qt::Checked);
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Float:
				{
					using T = EntityPropertyArray<float>;

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&m_delegates->floatDelegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());
						for (int i = 0; i < arraySize; ++i)
							m_delegates->floatDelegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						SetProperty(item->index().row(), m_delegates->floatDelegate.RetrieveModelData(item->index()));
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
				{
					using T = EntityPropertyArray<Nz::Vector2f>;
					
					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&m_delegates->float2Delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ tr("Value") });

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());
						for (int i = 0; i < arraySize; ++i)
							m_delegates->float2Delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						SetProperty(item->index().row(), m_delegates->float2Delegate.RetrieveModelData(item->index()));
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Integer:
				{
					using T = EntityPropertyArray<Nz::Int64>;

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&m_delegates->intDelegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());
						for (int i = 0; i < arraySize; ++i)
							m_delegates->intDelegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						SetProperty(item->index().row(), m_delegates->intDelegate.RetrieveModelData(item->index()));
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Layer:
				{
					std::vector<QString> options;
					options.push_back(tr("<No layer>"));

					for (std::size_t i = 0; i < m_map.GetLayerCount(); ++i)
					{
						auto& layer = m_map.GetLayer(i);

						options.push_back(tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(i + 1));
					}

					m_delegates->comboBoxDelegate.emplace(std::move(options));

					using T = EntityPropertyArray<Nz::Int64>;

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&m_delegates->comboBoxDelegate.value());
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());

						for (int i = 0; i < arraySize; ++i)
						{
							assert(propertyArray[i] <= 0xFFFF);
							int layerIndex = int(propertyArray[i]);
							if (layerIndex == NoLayer)
								layerIndex = 0;
							else
								layerIndex++;

							m_delegates->comboBoxDelegate->ApplyModelData(model, model->index(i, 0), layerIndex);
						}
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						Nz::Int64 layerIndex = m_delegates->comboBoxDelegate->RetrieveModelData(item->index());
						if (layerIndex == 0)
							layerIndex = NoLayer;
						else
							layerIndex--;

						SetProperty(item->index().row(), layerIndex);
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
				{
					using T = EntityPropertyArray<Nz::Vector2i64>;


					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&m_delegates->int2Delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ tr("Value") });

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());

						for (int i = 0; i < arraySize; ++i)
							m_delegates->int2Delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);
					}

					connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
					{
						SetProperty(item->index().row(), m_delegates->int2Delegate.RetrieveModelData(item->index()));
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::String:
				case PropertyType::Texture:
				{
					using T = EntityPropertyArray<std::string>;

					QTableWidget* table = new QTableWidget(arraySize, 1);
					table->setHorizontalHeaderLabels({ tr("Value") });
					table->setSelectionMode(QAbstractItemView::NoSelection);
					table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

					if (propertyValue)
					{
						auto& propertyArray = std::get<T>(propertyValue->get());

						QAbstractItemModel* model = table->model();
						for (int i = 0; i < arraySize; ++i)
							model->setData(model->index(i, 0), QString::fromStdString(propertyArray[i]));
					}

					connect(table, &QTableWidget::cellChanged, [=](int row, int column)
					{
						SetProperty(row, table->item(row, column)->text().toStdString());
					});

					layout->addWidget(table);
					break;
				}

				default:
					break;
			}
		}
		else
		{
			auto SetProperty = [this, keyName = propertyInfo.keyName, type = propertyInfo.type, OnPropertyOverride, UpdatePropertyPreview](auto&& value)
			{
				auto it = m_entityInfo.properties.find(keyName);
				if (it == m_entityInfo.properties.end())
				{
					it = m_entityInfo.properties.emplace(keyName, EntityProperty{}).first;
					OnPropertyOverride();
				}

				it.value() = std::forward<decltype(value)>(value);
				UpdatePropertyPreview(ToString(it.value(), type));
			};

			switch (propertyInfo.type)
			{
				case PropertyType::Bool:
				{
					QCheckBox* checkBox = new QCheckBox;
					if (propertyValue && std::holds_alternative<bool>(propertyValue->get()))
						checkBox->setChecked(std::get<bool>(propertyValue->get()));

					connect(checkBox, &QCheckBox::toggled, [=](bool checked)
					{
						SetProperty(checked);
					});

					layout->addWidget(checkBox);
					break;
				}

				case PropertyType::Float:
				{
					QDoubleSpinBox* spinbox = new QDoubleSpinBox;
					spinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
					if (propertyValue && std::holds_alternative<float>(propertyValue->get()))
						spinbox->setValue(std::get<float>(propertyValue->get()));

					connect(spinbox, &QDoubleSpinBox::editingFinished, [=]()
					{
						SetProperty(float(spinbox->value()));
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
				{
					Float2SpinBox::LabelMode labelMode = (propertyInfo.type == PropertyType::FloatPosition) ? Float2SpinBox::LabelMode::PositionLabel : Float2SpinBox::LabelMode::SizeLabel;
					Float2SpinBox* spinbox = new Float2SpinBox(labelMode, QBoxLayout::TopToBottom);
					if (propertyValue && std::holds_alternative<Nz::Vector2f>(propertyValue->get()))
						spinbox->setValue(std::get<Nz::Vector2f>(propertyValue->get()));

					connect(spinbox, &Float2SpinBox::valueChanged, [=]()
					{
						SetProperty(spinbox->value());
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::Integer:
				{
					// TODO: Handle properly int64
					QSpinBox* spinbox = new QSpinBox;
					spinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

					if (propertyValue && std::holds_alternative<Nz::Int64>(propertyValue->get()))
						spinbox->setValue(std::get<Nz::Int64>(propertyValue->get()));

					connect(spinbox, &QSpinBox::editingFinished, [=]()
					{
						SetProperty(Nz::Int64(spinbox->value()));
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::Layer:
				{
					QComboBox* comboBox = new QComboBox;
					comboBox->addItem(tr("<No layer>"));

					for (std::size_t i = 0; i < m_map.GetLayerCount(); ++i)
					{
						auto& layer = m_map.GetLayer(i);

						comboBox->addItem(tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(i + 1));
					}


					if (propertyValue && std::holds_alternative<Nz::Int64>(propertyValue->get()))
					{
						int index = int(std::get<Nz::Int64>(propertyValue->get()));
						if (index == NoLayer)
							index = 0;
						else
							index++;

						comboBox->setCurrentIndex(index);
					}

					connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index)
					{
						if (index <= 0)
							index = NoLayer;
						else
							index--;

						SetProperty(Nz::Int64(index));
					});

					layout->addWidget(comboBox);
					break;
				}

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
				{
					// TODO: Handle properly int64
					Integer2SpinBox::LabelMode labelMode = (propertyInfo.type == PropertyType::FloatPosition) ? Integer2SpinBox::LabelMode::PositionLabel : Integer2SpinBox::LabelMode::SizeLabel;
					Integer2SpinBox* spinbox = new Integer2SpinBox(labelMode, QBoxLayout::TopToBottom);
					if (propertyValue && std::holds_alternative<Nz::Vector2i64>(propertyValue->get()))
						spinbox->setValue(Nz::Vector2i(std::get<Nz::Vector2i64>(propertyValue->get())));

					connect(spinbox, &Integer2SpinBox::valueChanged, [=]()
					{
						SetProperty(Nz::Vector2i64(spinbox->value()));
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::String:
				case PropertyType::Texture:
				{
					QLineEdit* lineEdit = new QLineEdit;
					if (propertyValue && std::holds_alternative<std::string>(propertyValue->get()))
						lineEdit->setText(QString::fromStdString(std::get<std::string>(propertyValue->get())));

					connect(lineEdit, &QLineEdit::editingFinished, [=]()
					{
						SetProperty(lineEdit->text().toStdString());
					});

					layout->addWidget(lineEdit);
					break;
				}

				default:
					break;
			}
		}

		m_propertyContentWidget->setLayout(layout);
	}

	QString EntityInfoDialog::ToString(bool value, PropertyType type)
	{
		return (value) ? "true" : "false";
	}

	QString EntityInfoDialog::ToString(float value, PropertyType type)
	{
		return QString::number(value);
	}

	QString EntityInfoDialog::ToString(Nz::Int64 value, PropertyType type)
	{
		assert(type == PropertyType::Integer || type == PropertyType::Layer);
		switch (type)
		{
			case PropertyType::Integer:
				return QString::number(value);

			case PropertyType::Layer:
			{
				if (value == NoLayer)
					return tr("<No layer>");
				else
				{
					auto& layer = m_map.GetLayer(value);
					return tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(value + 1);
				}
			}

			default:
				assert(false);
				return "<error>";
		}
	}

	QString EntityInfoDialog::ToString(const Nz::Vector2f& value, PropertyType type)
	{
		return QString("(%1; %2)").arg(value.x).arg(value.y);
	}

	QString EntityInfoDialog::ToString(const Nz::Vector2i64& value, PropertyType type)
	{
		return QString("(%1; %2)").arg(value.x).arg(value.y);
	}

	QString EntityInfoDialog::ToString(const std::string& value, PropertyType type)
	{
		return QString::fromStdString(value);
	}

	QString EntityInfoDialog::ToString(EntityPropertyConstRefOpt property, PropertyType type)
	{
		if (!property)
			return QString("<No value>");

		return std::visit([=](const auto& value) -> QString
		{
			using T = std::decay_t<decltype(value)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;

			// We have to use if constexpr here because the compiler will instantiate this lambda even for single types
			if constexpr (IsArray)
			{
				return "Array of " + QString::number(value.size());
			}
			else
			{
				return ToString(value, type);
			}
		}, property->get());
	}

	auto EntityInfoDialog::GetProperty(const PropertyData& property) const -> EntityPropertyConstRefOpt
	{
		if (auto it = m_entityInfo.properties.find(property.keyName); it != m_entityInfo.properties.end())
			return it->second;
		else
			return property.defaultValue;
	}

	void EntityInfoDialog::OnAccept()
	{
		if (m_entityTypeWidget->currentIndex() < 0)
		{
			QMessageBox::critical(this, tr("Invalid entity type"), tr("You must select a valid entity type"), QMessageBox::Ok);
			return;
		}

		for (const auto& propertyInfo : m_properties)
		{
			if (!propertyInfo.defaultValue)
			{
				if (m_entityInfo.properties.find(propertyInfo.keyName) == m_entityInfo.properties.end())
				{
					QMessageBox::critical(this, tr("Missing required property"), tr("Property %1 has no value (mandatory field)").arg(QString::fromStdString(propertyInfo.visualName)), QMessageBox::Ok);
					return;
				}
			}
		}

		accept();
	}
}