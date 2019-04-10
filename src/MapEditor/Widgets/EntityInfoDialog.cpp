// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientScriptingContext.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <MapEditor/Widgets/Integer2SpinBox.hpp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <iostream>
#include <limits>

namespace bw
{
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

	EntityInfoDialog::EntityInfoDialog(ClientEntityStore& clientEntityStore, ClientScriptingContext& scriptingContext, QWidget* parent) :
	QDialog(parent),
	m_entityTypeIndex(0),
	m_propertyTypeIndex(InvalidIndex),
	m_entityStore(clientEntityStore),
	m_scriptingContext(scriptingContext)
	{
		m_entityInfo.position = Nz::Vector2f::Zero();
		m_entityInfo.rotation = 0.f;

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

		m_propertiesList = new QTableWidget(0, 2);
		m_propertiesList->setHorizontalHeaderLabels({ QString("Property"), QString("Value") });
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

		QDialogButtonBox* propertyButtons = new QDialogButtonBox(QDialogButtonBox::Reset);

		propertyContentLayout->addWidget(m_propertyTitle);
		propertyContentLayout->addWidget(m_propertyDescription);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(m_propertyContentWidget);
		propertyContentLayout->addStretch();
		propertyContentLayout->addWidget(propertyButtons);

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
		genericPropertyLayout->addRow("Entity type", m_entityTypeWidget);
		genericPropertyLayout->addRow("Entity name", m_nameWidget);
		genericPropertyLayout->addRow("Entity position", m_positionWidget);
		genericPropertyLayout->addRow("Entity rotation", m_rotationWidget);

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

		OnEntityTypeUpdate();
	}

	EntityInfoDialog::EntityInfoDialog(ClientEntityStore& clientEntityStore, ClientScriptingContext& scriptingContext, const Ndk::EntityHandle& targetEntity, EntityInfo entityInfo, QWidget* parent) :
	EntityInfoDialog(clientEntityStore, scriptingContext, parent)
	{
		m_entityInfo = std::move(entityInfo);
		m_targetEntity = targetEntity;

		m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
		m_positionWidget->setValue(m_entityInfo.position);
		m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());

		m_entityTypeWidget->setCurrentText(QString::fromStdString(m_entityInfo.entityClass));
	}

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
			property = &propertyData.defaultValue;
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

	void EntityInfoDialog::SetEntityPosition(const Nz::Vector2f& position)
	{
		m_entityInfo.position = position;
		m_positionWidget->setValue(position);
	}

	void EntityInfoDialog::SetEntityRotation(const Nz::DegreeAnglef& rotation)
	{
		m_entityInfo.rotation = rotation;
		m_rotationWidget->setValue(rotation.ToDegrees());
	}

	void EntityInfoDialog::SetProperty(const std::string& propertyName, EntityProperty propertyValue)
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

		std::size_t elementIndex = m_entityStore.GetElementIndex(entityType);
		if (elementIndex == m_entityStore.InvalidIndex)
			return;

		m_entityInfo.entityClass = std::move(entityType);

		m_entityTypeIndex = elementIndex;

		RefreshEntityType();
		RefreshPropertyEditor(InvalidIndex);
	}

	void EntityInfoDialog::RefreshEntityType()
	{
		m_propertiesList->clearContents();

		auto entityTypeInfo = std::static_pointer_cast<EditorScriptedEntity, ScriptedEntity>(m_entityStore.GetElement(m_entityTypeIndex));

		// Build property list and ensure relevant properties are stored
		EntityProperties oldProperties = std::move(m_entityInfo.properties);
		m_entityInfo.properties.clear(); // Put back in a valid state

		m_properties.clear();
		for (const auto& [propertyName, propertyInfo] : entityTypeInfo->properties)
		{
			auto& propertyData = m_properties.emplace_back();
			propertyData.index = propertyInfo.index;
			propertyData.isArray = propertyInfo.isArray;
			propertyData.defaultValue = *propertyInfo.defaultValue;
			propertyData.keyName = propertyName;
			propertyData.visualName = propertyData.keyName; //< FIXME
			propertyData.type = propertyInfo.type;

			if (auto it = oldProperties.find(propertyData.keyName); it != oldProperties.end())
			{
				// Only keep old property value if types are compatibles
				if (it->second.index() == propertyInfo.defaultValue->index())
					m_entityInfo.properties.emplace(std::move(it.key()), std::move(it.value()));

				oldProperties.erase(it);
			}
		}

		std::sort(m_properties.begin(), m_properties.end(), [](auto&& first, auto&& second) { return first.index < second.index; });

		m_propertyByName.clear();
		for (std::size_t i = 0; i < m_properties.size(); ++i)
			m_propertyByName.emplace(m_properties[i].keyName, i);

		m_propertiesList->setRowCount(int(m_properties.size()));

		int rowIndex = 0;
		for (const auto& propertyInfo : m_properties)
		{
			m_propertiesList->setItem(rowIndex, 0, new QTableWidgetItem(QString::fromStdString(propertyInfo.visualName)));
			m_propertiesList->setItem(rowIndex, 1, new QTableWidgetItem("Value"));

			++rowIndex;
		}


		while (QWidget* w = m_editorActionWidget->findChild<QWidget*>())
			delete w;

		m_editorActionByName.clear();

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
					std::cerr << "Editor action " << name << "::OnTrigger failed: " << err.what() << std::endl;
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

		m_propertyTitle->setText(QString::fromStdString(propertyInfo.visualName));

		auto GetProperty = [&]() -> const EntityProperty&
		{
			if (auto it = m_entityInfo.properties.find(propertyInfo.keyName); it != m_entityInfo.properties.end())
				return it->second;
			else
				return propertyInfo.defaultValue;
		};

		const EntityProperty& property = GetProperty();

		QVBoxLayout* layout = new QVBoxLayout;

		bool isArray;
		int arraySize;

		std::visit([&](auto&& propertyValue)
		{
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, std::decay_t<decltype(propertyValue)>>;

			isArray = IsArray;
			if constexpr (IsArray)
				arraySize = int(propertyValue.size());

		}, property);

		if (isArray)
		{
			QSpinBox* spinbox = new QSpinBox;
			spinbox->setRange(0, std::numeric_limits<int>::max());
			spinbox->setValue(arraySize);

			QPushButton* updateButton = new QPushButton(tr("Update"));
			connect(updateButton, &QPushButton::released, [this, &propertyInfo, spinbox, propertyIndex]()
			{
				std::size_t newSize = spinbox->value();

				// Ensure we have a custom value for this
				EntityProperty* property;
				if (auto it = m_entityInfo.properties.find(propertyInfo.keyName); it != m_entityInfo.properties.end())
					property = &it.value();
				else
					property = &m_entityInfo.properties.emplace(propertyInfo.keyName, propertyInfo.defaultValue).first.value(); //< This is ugly

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

				RefreshPropertyEditor(propertyIndex);
			});

			QHBoxLayout* arraySizeLayout = new QHBoxLayout;
			arraySizeLayout->addWidget(spinbox);
			arraySizeLayout->addWidget(updateButton);

			layout->addLayout(arraySizeLayout);

			switch (propertyInfo.type)
			{
				case PropertyType::Bool:
				{
					using T = EntityPropertyArray<bool>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Enabled") });

					for (int i = 0; i < arraySize; ++i)
					{
						QStandardItem* item = new QStandardItem(1);
						item->setCheckable(true);
						item->setCheckState((propertyArray[i]) ? Qt::Checked : Qt::Unchecked);

						model->setItem(i, 0, item);
					}

					connect(model, &QStandardItemModel::itemChanged, [this, keyName = propertyInfo.keyName, arraySize](QStandardItem* item)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());

						int rowIndex = item->index().row();
						propertyArray[rowIndex] = (item->checkState() == Qt::Checked);
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Float:
				{
					static FloatPropertyDelegate delegate; //FIXME?

					using T = EntityPropertyArray<float>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					for (int i = 0; i < arraySize; ++i)
						delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);

					connect(model, &QStandardItemModel::itemChanged, [this, keyName = propertyInfo.keyName, arraySize](QStandardItem* item)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());

						int rowIndex = item->index().row();
						propertyArray[rowIndex] = delegate.RetrieveModelData(item->index());
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
				{
					static Float2PropertyDelegate delegate; //< FIXME?

					using T = EntityPropertyArray<Nz::Vector2f>;

					auto& propertyArray = std::get<T>(property);
					
					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					for (int i = 0; i < arraySize; ++i)
						delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);

					connect(model, &QStandardItemModel::itemChanged, [this, keyName = propertyInfo.keyName, arraySize](QStandardItem* item)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());

						int rowIndex = item->index().row();
						propertyArray[rowIndex] = delegate.RetrieveModelData(item->index());
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Integer:
				{
					static IntegerPropertyDelegate delegate; //FIXME?
					
					using T = EntityPropertyArray<Nz::Int64>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					for (int i = 0; i < arraySize; ++i)
						delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);

					connect(model, &QStandardItemModel::itemChanged, [this, keyName = propertyInfo.keyName, arraySize](QStandardItem* item)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());

						int rowIndex = item->index().row();
						propertyArray[rowIndex] = delegate.RetrieveModelData(item->index());
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
				{
					static Integer2PropertyDelegate delegate; //< FIXME?

					using T = EntityPropertyArray<Nz::Vector2i64>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setItemDelegate(&delegate);
					tableView->setModel(model);

					model->setHorizontalHeaderLabels({ QString("Value") });

					for (int i = 0; i < arraySize; ++i)
						delegate.ApplyModelData(model, model->index(i, 0), propertyArray[i]);

					connect(model, &QStandardItemModel::itemChanged, [this, keyName = propertyInfo.keyName, arraySize](QStandardItem* item)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());

						int rowIndex = item->index().row();
						propertyArray[rowIndex] = delegate.RetrieveModelData(item->index());
					});

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::String:
				case PropertyType::Texture:
				{
					using T = EntityPropertyArray<std::string>;

					auto& propertyArray = std::get<T>(property);

					QTableWidget* table = new QTableWidget(arraySize, 1);
					table->setHorizontalHeaderLabels({ QString("Value") });
					table->setSelectionMode(QAbstractItemView::NoSelection);
					table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

					QAbstractItemModel* model = table->model();
					for (int i = 0; i < arraySize; ++i)
						model->setData(model->index(i, 0), QString::fromStdString(propertyArray[i]));

					connect(table, &QTableWidget::cellChanged, [this, keyName = propertyInfo.keyName, table, arraySize](int row, int column)
					{
						auto it = m_entityInfo.properties.find(keyName);
						if (it == m_entityInfo.properties.end())
							it = m_entityInfo.properties.emplace(keyName, T(arraySize)).first;

						auto& propertyArray = std::get<T>(it.value());
						propertyArray[row] = table->item(row, column)->text().toStdString();
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
			switch (propertyInfo.type)
			{
				case PropertyType::Bool:
				{
					QCheckBox* checkBox = new QCheckBox;
					if (std::holds_alternative<bool>(property))
						checkBox->setChecked(std::get<bool>(property));

					connect(checkBox, &QCheckBox::toggled, [this, keyName = propertyInfo.keyName](bool checked)
					{
						m_entityInfo.properties[keyName] = checked;
					});

					layout->addWidget(checkBox);
					break;
				}

				case PropertyType::Float:
				{
					QDoubleSpinBox* spinbox = new QDoubleSpinBox;
					spinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
					if (std::holds_alternative<float>(property))
						spinbox->setValue(std::get<float>(property));

					connect(spinbox, &QDoubleSpinBox::editingFinished, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = float(spinbox->value());
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
				{
					Float2SpinBox::LabelMode labelMode = (propertyInfo.type == PropertyType::FloatPosition) ? Float2SpinBox::LabelMode::PositionLabel : Float2SpinBox::LabelMode::SizeLabel;
					Float2SpinBox* spinbox = new Float2SpinBox(labelMode, QBoxLayout::TopToBottom);
					if (std::holds_alternative<Nz::Vector2f>(property))
						spinbox->setValue(std::get<Nz::Vector2f>(property));

					connect(spinbox, &Float2SpinBox::valueChanged, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = Nz::Vector2f(spinbox->value());
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::Integer:
				{
					// TODO: Handle properly int64
					QSpinBox* spinbox = new QSpinBox;
					if (std::holds_alternative<Nz::Int64>(property))
						spinbox->setValue(std::get<Nz::Int64>(property));

					connect(spinbox, &QSpinBox::editingFinished, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = Nz::Int64(spinbox->value());
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
				{
					// TODO: Handle properly int64
					Integer2SpinBox::LabelMode labelMode = (propertyInfo.type == PropertyType::FloatPosition) ? Integer2SpinBox::LabelMode::PositionLabel : Integer2SpinBox::LabelMode::SizeLabel;
					Integer2SpinBox* spinbox = new Integer2SpinBox(labelMode, QBoxLayout::TopToBottom);
					if (std::holds_alternative<Nz::Vector2i64>(property))
						spinbox->setValue(Nz::Vector2i(std::get<Nz::Vector2i64>(property)));

					connect(spinbox, &Integer2SpinBox::valueChanged, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = Nz::Vector2i64(spinbox->value());
					});

					layout->addWidget(spinbox);
					break;
				}

				case PropertyType::String:
				case PropertyType::Texture:
				{
					QLineEdit* lineEdit = new QLineEdit;
					if (std::holds_alternative<std::string>(property))
						lineEdit->setText(QString::fromStdString(std::get<std::string>(property)));

					connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = lineEdit->text().toStdString();
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

	void EntityInfoDialog::OnAccept()
	{
		/*if (m_name->text().isEmpty())
		{
			QMessageBox::critical(this, "Missing informations", "Map name must be set", QMessageBox::Ok);
			return;
		}
*/

		accept();
	}
}