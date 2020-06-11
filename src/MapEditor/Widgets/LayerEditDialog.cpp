// Copyright (C) 2020 Jérôme Leclercq
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

		m_layerName->setText(QString::fromStdString(layerInfo.name));
	}

	LayerInfo LayerEditDialog::GetLayerInfo() const
	{
		LayerInfo layerInfo;
		layerInfo.backgroundColor = Nz::Color(Nz::UInt8(m_layerColor.red()), Nz::UInt8(m_layerColor.green()), Nz::UInt8(m_layerColor.blue()));
		layerInfo.name = m_layerName->text().toStdString();

		return layerInfo;
	}

	void LayerEditDialog::OnAccept()
	{
		accept();
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
}
