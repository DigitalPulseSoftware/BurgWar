// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/EntityInfoDialog.hpp>
#include <MapEditor/EntityTypeRegistry.hpp>
#include <MapEditor/Widgets/PositionEditWidget.hpp>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <iostream>
#include <limits>

namespace bw
{
	EntityInfoDialog::EntityInfoDialog(QWidget* parent) :
	QDialog(parent),
	m_entityTypeIndex(0)
	{
		m_entityInfo.position = Nz::Vector2f::Zero();
		m_entityInfo.rotation = 0.f;

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		EntityTypeRegistry* instance = EntityTypeRegistry::Instance();

		m_entityTypeWidget = new QComboBox;
		for (std::size_t i = 0; i < instance->GetTypeCount(); ++i)
			m_entityTypeWidget->addItem(QString::fromStdString(instance->GetType(i).name));

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

	EntityInfoDialog::EntityInfoDialog(EntityInfo entityInfo, QWidget* parent) :
	EntityInfoDialog(parent)
	{
		m_entityInfo = std::move(entityInfo);

		m_nameWidget->setText(QString::fromStdString(m_entityInfo.entityName));
		m_positionWidget->setValue(Nz::Vector2d(m_entityInfo.position));
		m_rotationWidget->setValue(m_entityInfo.rotation.ToDegrees());
	}

	const EntityInfo& EntityInfoDialog::GetEntityInfo() const
	{
		return m_entityInfo;
	}

	void EntityInfoDialog::OnEntityTypeUpdate()
	{
		std::string entityType = m_entityTypeWidget->currentText().toStdString();

		auto typeIndexOpt = EntityTypeRegistry::Instance()->FindTypeIndex(entityType);
		if (!typeIndexOpt.has_value())
			return;

		m_entityInfo.entityClass = std::move(entityType);

		m_entityTypeIndex = *typeIndexOpt;

		RefreshEntityType();
	}

	void EntityInfoDialog::RefreshEntityType()
	{
		EntityTypeRegistry* registry = EntityTypeRegistry::Instance();

		m_propertiesList->clearContents();

		const auto& entityTypeInfo = registry->GetType(m_entityTypeIndex);
		m_propertiesList->setRowCount(int(entityTypeInfo.properties.size()));

		int rowIndex = 0;
		for (const auto& propertyInfo : entityTypeInfo.properties)
		{
			m_propertiesList->setItem(rowIndex, 0, new QTableWidgetItem(QString::fromStdString(propertyInfo.keyName)));
			m_propertiesList->setItem(rowIndex, 1, new QTableWidgetItem("Value"));

			++rowIndex;
		}
	}

	void EntityInfoDialog::RefreshPropertyEditor(int propertyIndex)
	{
		const auto& entityTypeInfo = EntityTypeRegistry::Instance()->GetType(m_entityTypeIndex);

		assert(propertyIndex < int(entityTypeInfo.properties.size()));
		const auto& propertyInfo = entityTypeInfo.properties[propertyIndex];

		m_propertyTitle->setText(QString::fromStdString(propertyInfo.visualName));

		auto GetProperty = [&]() -> const EntityProperty&
		{
			if (auto it = m_entityInfo.properties.find(propertyInfo.keyName); it != m_entityInfo.properties.end())
				return it->second;
			else
			{
				static EntityProperty emptyProperty;
				return emptyProperty;
			}
		};

		const EntityProperty& property = GetProperty();

		QVBoxLayout* layout = new QVBoxLayout;
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