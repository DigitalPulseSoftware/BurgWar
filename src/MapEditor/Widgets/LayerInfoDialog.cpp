// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/LayerInfoDialog.hpp>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <limits>

namespace bw
{
	LayerInfoDialog::LayerInfoDialog(QWidget* parent) :
	QDialog(parent)
	{
		QFormLayout* formLayout = new QFormLayout;

		m_layerName = new QLineEdit;
		formLayout->addRow(tr("Layer name"), m_layerName);

		m_layerDepth = new QDoubleSpinBox;
		m_layerDepth->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		formLayout->addRow(tr("Layer depth"), m_layerDepth);

		m_layerColorEdit = new QPushButton("Edit");
		connect(m_layerColorEdit, &QPushButton::pressed, this, &LayerInfoDialog::OnEditLayerColor);

		formLayout->addRow(tr("Layer color"), m_layerColorEdit);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &LayerInfoDialog::OnAccept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(formLayout);
		verticalLayout->addWidget(buttonBox);

		setLayout(verticalLayout);
	}

	LayerInfoDialog::LayerInfoDialog(LayerInfo entityInfo, QWidget* parent) :
	LayerInfoDialog(parent)
	{
		m_layerColor = QColor::fromRgb(entityInfo.backgroundColor.r, entityInfo.backgroundColor.g, entityInfo.backgroundColor.b);
		m_layerColorEdit->setStyleSheet("background-color: " + m_layerColor.name());

		m_layerDepth->setValue(entityInfo.depth);
		m_layerName->setText(QString::fromStdString(entityInfo.name));
	}

	LayerInfo LayerInfoDialog::GetLayerInfo() const
	{
		LayerInfo layerInfo;
		layerInfo.backgroundColor = Nz::Color(Nz::UInt8(m_layerColor.red()), Nz::UInt8(m_layerColor.green()), Nz::UInt8(m_layerColor.blue()));
		layerInfo.depth = float(m_layerDepth->value());
		layerInfo.name = m_layerName->text().toStdString();

		return layerInfo;
	}

	void LayerInfoDialog::OnAccept()
	{
		accept();
	}

	void LayerInfoDialog::OnEditLayerColor()
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
