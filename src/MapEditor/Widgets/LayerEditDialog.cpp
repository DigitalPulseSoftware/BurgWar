// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/LayerEditDialog.hpp>
#include <CoreLib/Map.hpp>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <limits>

namespace bw
{
	LayerEditDialog::LayerEditDialog(const Map& map, QWidget* parent) :
	QDialog(parent),
	m_map(map),
	m_layerIndex(NoLayer)
	{
		QFormLayout* formLayout = new QFormLayout;

		m_layerName = new QLineEdit;
		formLayout->addRow(tr("Layer name"), m_layerName);

		m_layerColorEdit = new QPushButton("Edit");
		connect(m_layerColorEdit, &QPushButton::pressed, this, &LayerEditDialog::OnEditLayerColor);

		formLayout->addRow(tr("Layer color"), m_layerColorEdit);

		m_visibilityList = new QListWidget;
		connect(m_visibilityList, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem* item)
		{
			if (!item)
				return;

			qulonglong index = item->data(Qt::UserRole).toULongLong();

			OnEditVisibilityLayer(static_cast<std::size_t>(index));
		});
		
		QVBoxLayout* visibleLayerLayout = new QVBoxLayout;
		visibleLayerLayout->addWidget(m_visibilityList);

		QPushButton* addLayerButton = new QPushButton(tr("Add visible layer"));
		connect(addLayerButton, &QPushButton::released, this, &LayerEditDialog::OnCreateVisibilityLayer);

		visibleLayerLayout->addWidget(addLayerButton);

		QPushButton* deleteLayerButton = new QPushButton(tr("Delete visible layer"));
		connect(deleteLayerButton, &QPushButton::released, this, [this] 
		{
			QList<QListWidgetItem*> items = m_visibilityList->selectedItems();
			if (!items.empty())
			{
				assert(items.size() == 1);

				qulonglong index = items[0]->data(Qt::UserRole).toULongLong();
				OnDeleteVisibilityLayer(static_cast<std::size_t>(index));
			}
		});

		visibleLayerLayout->addWidget(deleteLayerButton);

		formLayout->addRow(tr("Visible layers"), visibleLayerLayout);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &LayerEditDialog::OnAccept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(formLayout);
		verticalLayout->addWidget(buttonBox);

		setLayout(verticalLayout);
	}

	LayerEditDialog::LayerEditDialog(LayerIndex layerIndex, LayerInfo layerInfo, const Map& map, QWidget* parent) :
	LayerEditDialog(map, parent)
	{
		m_layerColor = QColor::fromRgb(layerInfo.backgroundColor.r, layerInfo.backgroundColor.g, layerInfo.backgroundColor.b);
		m_layerColorEdit->setStyleSheet("background-color: " + m_layerColor.name());
		m_layerIndex = layerIndex;
		m_visibilities = std::move(layerInfo.visibilities);

		m_layerName->setText(QString::fromStdString(layerInfo.name));

		RefreshVisibilityList();
	}

	LayerInfo LayerEditDialog::GetLayerInfo() const
	{
		LayerInfo layerInfo;
		layerInfo.backgroundColor = Nz::Color(Nz::UInt8(m_layerColor.red()), Nz::UInt8(m_layerColor.green()), Nz::UInt8(m_layerColor.blue()));
		layerInfo.name = m_layerName->text().toStdString();
		layerInfo.visibilities = m_visibilities;

		return layerInfo;
	}

	void LayerEditDialog::OnAccept()
	{
		accept();
	}

	void LayerEditDialog::OnCreateVisibilityLayer()
	{
		LayerVisibilityDialog* visibilityDialog = new LayerVisibilityDialog(m_layerIndex, m_map, this);
		connect(visibilityDialog, &QDialog::accepted, [this, visibilityDialog]()
		{
			m_visibilities.push_back(visibilityDialog->GetLayerVisibilityInfo());

			RefreshVisibilityList();
		});

		visibilityDialog->exec();
	}

	void LayerEditDialog::OnDeleteVisibilityLayer(std::size_t visibilityIndex)
	{
		assert(visibilityIndex < m_visibilities.size());

		int selected = QMessageBox::warning(this, tr("Delete visibility layer?"), tr("Are you sure you want to delete this visibility layer?"), QMessageBox::Yes | QMessageBox::Cancel);
		if (selected != QMessageBox::Yes)
			return;

		m_visibilities.erase(m_visibilities.begin() + visibilityIndex);

		RefreshVisibilityList();
	}

	void LayerEditDialog::OnEditVisibilityLayer(std::size_t visibilityIndex)
	{
		assert(visibilityIndex < m_visibilities.size());

		LayerVisibilityDialog* visibilityDialog = new LayerVisibilityDialog(m_layerIndex, m_visibilities[visibilityIndex], m_map, this);
		connect(visibilityDialog, &QDialog::accepted, [this, visibilityDialog, visibilityIndex]()
		{
			m_visibilities[visibilityIndex] = visibilityDialog->GetLayerVisibilityInfo();

			RefreshVisibilityList();
		});

		visibilityDialog->exec();
	}

	void LayerEditDialog::OnEditLayerColor()
	{
		QColorDialog* colorDialog = new QColorDialog(m_layerColor, this);
		colorDialog->setOption(QColorDialog::ShowAlphaChannel, false);

		connect(colorDialog, &QColorDialog::accepted, [this, colorDialog]()
		{
			m_layerColor = colorDialog->selectedColor();
			m_layerColorEdit->setStyleSheet("background-color: " + m_layerColor.name());
		});

		colorDialog->exec();
	}

	void LayerEditDialog::RefreshVisibilityList()
	{
		m_visibilityList->clear();
		m_visibilityList->clearSelection();

		std::size_t visibilityIndex = 0;
		for (const auto& visibilityInfo : m_visibilities)
		{
			auto& layer = m_map.GetLayer(visibilityInfo.layerIndex);

			QString visibilityName = tr("%1: %2 (%3)").arg(visibilityIndex).arg(QString::fromStdString(layer.name)).arg(visibilityInfo.layerIndex + 1);

			QListWidgetItem* item = new QListWidgetItem(visibilityName);
			item->setData(Qt::UserRole, qulonglong(visibilityIndex));

			m_visibilityList->addItem(item);

			visibilityIndex++;
		}
	}
}
