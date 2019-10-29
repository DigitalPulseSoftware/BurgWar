// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/LayerVisibilityDialog.hpp>
#include <CoreLib/Map.hpp>
#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <limits>

namespace bw
{
	LayerVisibilityDialog::LayerVisibilityDialog(LayerIndex excludedLayerIndex, const Map& map, QWidget* parent) :
	QDialog(parent)
	{
		QFormLayout* formLayout = new QFormLayout;

		m_layer = new QComboBox;
		for (std::size_t i = 0; i < map.GetLayerCount(); ++i)
		{
			if (i == excludedLayerIndex)
				continue;

			auto& layer = map.GetLayer(i);

			m_layer->addItem(tr("%1 (%2)").arg(QString::fromStdString(layer.name)).arg(i + 1), qulonglong(i));
		}

		formLayout->addRow(tr("Layer"), m_layer);

		m_renderOrder = new QSpinBox;
		m_renderOrder->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

		formLayout->addRow(tr("Render order"), m_renderOrder);

		m_offset = new Float2SpinBox(Float2SpinBox::LabelMode::PositionLabel, QBoxLayout::LeftToRight);
		formLayout->addRow(tr("Offset"), m_offset);

		m_scale = new Float2SpinBox(Float2SpinBox::LabelMode::ScaleLabel, QBoxLayout::LeftToRight);
		m_scale->setValue(Nz::Vector2f::Unit());
		formLayout->addRow(tr("Scale"), m_scale);

		m_parallaxFactor = new Float2SpinBox(Float2SpinBox::LabelMode::ScaleLabel, QBoxLayout::LeftToRight);
		m_parallaxFactor->setValue(Nz::Vector2f::Unit());
		
		formLayout->addRow(tr("Parallax Factor"), m_parallaxFactor);

		m_rotation = new QDoubleSpinBox;
		m_rotation->setRange(-180.0, 180.0);

		formLayout->addRow(tr("Rotation"), m_rotation);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &LayerVisibilityDialog::OnAccept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(formLayout);
		verticalLayout->addWidget(buttonBox);

		setLayout(verticalLayout);
	}

	LayerVisibilityDialog::LayerVisibilityDialog(LayerIndex excludedLayerIndex, LayerVisibilityInfo visibilityInfo, const Map& map, QWidget* parent) :
	LayerVisibilityDialog(excludedLayerIndex, map, parent)
	{
		LayerIndex visibilityLayerIndex = visibilityInfo.layerIndex;
		if (visibilityLayerIndex >= excludedLayerIndex)
			--visibilityLayerIndex;

		m_layer->setCurrentIndex(int(visibilityLayerIndex));
		m_renderOrder->setValue(visibilityInfo.renderOrder);
		m_offset->setValue(visibilityInfo.offset);
		m_scale->setValue(visibilityInfo.scale);
		m_parallaxFactor->setValue(visibilityInfo.parallaxFactor);
		m_rotation->setValue(visibilityInfo.rotation.ToDegrees());
	}

	LayerVisibilityInfo LayerVisibilityDialog::GetLayerVisibilityInfo() const
	{
		LayerVisibilityInfo visibilityInfo;
		visibilityInfo.layerIndex = m_layer->itemData(m_layer->currentIndex()).value<qulonglong>();
		visibilityInfo.offset = m_offset->value();
		visibilityInfo.parallaxFactor = m_parallaxFactor->value();
		visibilityInfo.renderOrder = m_renderOrder->value();
		visibilityInfo.rotation = Nz::DegreeAnglef(float(m_rotation->value()));
		visibilityInfo.scale = m_scale->value();

		return visibilityInfo;
	}

	void LayerVisibilityDialog::OnAccept()
	{
		accept();
	}
}
