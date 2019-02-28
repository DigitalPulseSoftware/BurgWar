// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <MapEditor/Widgets/PositionEditWidget.hpp>
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

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

				return editor;
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->setValue(index.model()->data(index, Qt::EditRole).toDouble());
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
				spinBox->interpretText();

				model->setData(index, spinBox->value(), Qt::EditRole);
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

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				//TODO: Subclass for int64
				QSpinBox* editor = new QSpinBox(parent);
				editor->setFrame(false);
				editor->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

				return editor;
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->setValue(index.model()->data(index, Qt::EditRole).toInt());
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
				spinBox->interpretText();

				model->setData(index, spinBox->value(), Qt::EditRole);
			}

			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				editor->setGeometry(option.rect);
			}
	};

	EntityInfoDialog::EntityInfoDialog(ClientEntityStore& clientEntityStore, QWidget* parent) :
	QDialog(parent),
	m_entityTypeIndex(0),
	m_entityStore(clientEntityStore)
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

		m_positionWidget = new PositionEditWidget;
		connect(m_positionWidget, &PositionEditWidget::valueChanged, [this](Nz::Vector2d value)
		{
			m_entityInfo.position = Nz::Vector2f(value);
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

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(genericPropertyLayout);
		verticalLayout->addLayout(propertyLayout);
		verticalLayout->addWidget(button);

		setLayout(verticalLayout);

		OnEntityTypeUpdate();
	}

	EntityInfoDialog::EntityInfoDialog(ClientEntityStore& clientEntityStore, EntityInfo entityInfo, QWidget* parent) :
	EntityInfoDialog(clientEntityStore, parent)
	{
		m_entityInfo = std::move(entityInfo);

		m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
		m_positionWidget->setValue(Nz::Vector2d(m_entityInfo.position));
		m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());

		m_entityTypeWidget->setCurrentText(QString::fromStdString(m_entityInfo.entityClass));
	}

	const EntityInfo& EntityInfoDialog::GetEntityInfo() const
	{
		return m_entityInfo;
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
	}

	void EntityInfoDialog::RefreshEntityType()
	{
		m_propertiesList->clearContents();

		const auto& entityTypeInfo = m_entityStore.GetElement(m_entityTypeIndex);

		// Build property list and ensure relevant properties are stored
		EntityProperties oldProperties = std::move(m_entityInfo.properties);
		m_entityInfo.properties.clear(); // Put back in a valid state

		m_properties.clear();
		for (const auto& propertyInfo : entityTypeInfo->properties)
		{
			auto& propertyData = m_properties.emplace_back();
			propertyData.defaultValue = *propertyInfo.second.defaultValue;
			propertyData.keyName = propertyInfo.first;
			propertyData.visualName = propertyData.keyName; //< FIXME
			propertyData.type = propertyInfo.second.type;

			if (auto it = oldProperties.find(propertyData.keyName); it != oldProperties.end())
			{
				// Only keep old property value if types are compatibles
				if (it->second.index() == propertyInfo.second.defaultValue->index())
					m_entityInfo.properties.emplace(std::move(it.key()), std::move(it.value()));

				oldProperties.erase(it);
			}
		}

		std::sort(m_properties.begin(), m_properties.end(), [](auto&& first, auto&& second) { return first.keyName < second.keyName; });

		m_propertiesList->setRowCount(int(m_properties.size()));

		int rowIndex = 0;
		for (const auto& propertyInfo : m_properties)
		{
			m_propertiesList->setItem(rowIndex, 0, new QTableWidgetItem(QString::fromStdString(propertyInfo.visualName)));
			m_propertiesList->setItem(rowIndex, 1, new QTableWidgetItem("Value"));

			++rowIndex;
		}
	}

	void EntityInfoDialog::RefreshPropertyEditor(int propertyIndex)
	{
		const auto& entityTypeInfo = m_entityStore.GetElement(m_entityTypeIndex);

		assert(propertyIndex < int(m_properties.size()));
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

		/*std::visit([&](auto&& propertyValue)
		{
			using T = std::decay_t<decltype(propertyValue)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
			using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

			if constexpr (!IsArray) //< FIXME
			{
				if constexpr (std::is_same_v<PropertyType, bool>)
				{
					QCheckBox* checkBox = new QCheckBox;
					checkBox->setChecked(propertyValue);

					connect(checkBox, &QCheckBox::toggled, [this, keyName = propertyInfo.keyName](bool checked)
					{
						m_entityInfo.properties[keyName] = checked;
					});

					layout->addWidget(checkBox);
				}
				else if constexpr (std::is_same_v<PropertyType, float>)
				{
					QDoubleSpinBox* spinbox = new QDoubleSpinBox;
					spinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
					spinbox->setValue(propertyValue);

					connect(spinbox, &QDoubleSpinBox::editingFinished, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = float(spinbox->value());
					});

					layout->addWidget(spinbox);
				}
				else if constexpr (std::is_same_v<PropertyType, Nz::Int64>)
				{
					// TODO: Handle properly int64
					QSpinBox* spinbox = new QSpinBox;
					spinbox->setValue(propertyValue);

					connect(spinbox, &QSpinBox::editingFinished, [this, spinbox, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = Nz::Int64(spinbox->value());
					});

					layout->addWidget(spinbox);
				}
				else if constexpr (std::is_same_v<PropertyType, std::string>)
				{
					QLineEdit* lineEdit = new QLineEdit;
					if (std::holds_alternative<EntityPropertyContainer<std::string>>(property))
						lineEdit->setText(QString::fromStdString(std::get<EntityPropertyContainer<std::string>>(property).GetElement(0)));

					connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit, keyName = propertyInfo.keyName]()
					{
						m_entityInfo.properties[keyName] = EntityPropertyContainer(lineEdit->text().toStdString());
					});
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
			}

		}, property);*/

		bool isArray;
		std::size_t arraySize;

		std::visit([&](auto&& propertyValue)
		{
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, std::decay_t<decltype(propertyValue)>>;

			isArray = IsArray;
			if constexpr (IsArray)
				arraySize = propertyValue.size();

		}, property);

		if (isArray)
		{
			QSpinBox* spinbox = new QSpinBox;
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
					using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

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

					model->setRowCount(arraySize);

					QStringList headerLabels;
					headerLabels << "Enabled";
					model->setHorizontalHeaderLabels(headerLabels);

					for (std::size_t i = 0; i < arraySize; ++i)
					{
						QStandardItem* item = new QStandardItem(1);
						item->setCheckable(true);
						item->setCheckState((propertyArray[i]) ? Qt::Checked : Qt::Unchecked);

						model->setItem(i, 0, item);
					}

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Float:
				{
					using T = EntityPropertyArray<float>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setModel(model);
					tableView->setItemDelegate(new FloatPropertyDelegate); //FIXME

					model->setRowCount(arraySize);

					QStringList headerLabels;
					headerLabels << "Value";
					model->setHorizontalHeaderLabels(headerLabels);

					for (std::size_t i = 0; i < arraySize; ++i)
					{
						QModelIndex index = model->index(i, 0, QModelIndex());
						model->setData(index, double(propertyArray[i]), Qt::EditRole);
					}

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::Integer:
				{
					using T = EntityPropertyArray<Nz::Int64>;

					auto& propertyArray = std::get<T>(property);

					QTableView* tableView = new QTableView;
					QStandardItemModel* model = new QStandardItemModel(arraySize, 1, tableView);
					tableView->setModel(model);

					tableView->setItemDelegate(new IntegerPropertyDelegate); //FIXME

					model->setRowCount(arraySize);

					QStringList headerLabels;
					headerLabels << "Value";
					model->setHorizontalHeaderLabels(headerLabels);

					for (std::size_t i = 0; i < arraySize; ++i)
						model->setData(model->index(i, 0), int(propertyArray[i]), Qt::EditRole); //< FIXME

					layout->addWidget(tableView);
					break;
				}

				case PropertyType::String:
				case PropertyType::Texture:
				{
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

		while (QWidget* w = m_propertyContentWidget->findChild<QWidget*>())
			delete w;

		delete m_propertyContentWidget->layout();

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