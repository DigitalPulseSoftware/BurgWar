// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Integer3SpinBox.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <limits>

namespace bw
{
	Integer3SpinBox::Integer3SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	QWidget(parent),
	m_ignoreSignal(false)
	{
		auto onValueChanged = [this](double) 
		{ 
			if (!m_ignoreSignal)
				onSpinBoxValueChanged();
		};

		m_xSpinbox = new QSpinBox;
		m_xSpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_xSpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);

		m_ySpinbox = new QSpinBox;
		m_ySpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_ySpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);

		m_zSpinbox = new QSpinBox;
		m_zSpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_zSpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);

		QLayout* layout = nullptr;

		switch (labelMode)
		{
			case LabelMode::NoLabel:
				layout = new QBoxLayout(dir);
				layout->addWidget(m_xSpinbox);
				layout->addWidget(m_ySpinbox);
				layout->addWidget(m_zSpinbox);
				break;

			case LabelMode::EulerAngles:
			case LabelMode::PositionLabel:
			case LabelMode::ScaleLabel:
			case LabelMode::SizeLabel:
			{
				QString xLabel;
				QString yLabel;
				QString zLabel;

				if (labelMode == LabelMode::EulerAngles)
				{
					xLabel = tr("Pitch");
					yLabel = tr("Yaw");
					zLabel = tr("Roll");
				}
				else if (labelMode == LabelMode::SizeLabel)
				{
					xLabel = tr("Width");
					yLabel = tr("Height");
					zLabel = tr("Depth");
				}
				else
				{
					xLabel = "X";
					yLabel = "Y";
					zLabel = "Z";
				}

				if (dir == QBoxLayout::TopToBottom)
				{
					QFormLayout* formLayout = new QFormLayout;
					formLayout->addRow(xLabel, m_xSpinbox);
					formLayout->addRow(yLabel, m_ySpinbox);
					formLayout->addRow(zLabel, m_zSpinbox);

					layout = formLayout;
				}
				else
				{
					QBoxLayout* boxLayout = new QBoxLayout(dir);

					QFormLayout* xLayout = new QFormLayout;
					xLayout->addRow(xLabel, m_xSpinbox);
					boxLayout->addLayout(xLayout);

					QFormLayout* yLayout = new QFormLayout;
					yLayout->addRow(yLabel, m_ySpinbox);
					boxLayout->addLayout(yLayout);

					QFormLayout* zLayout = new QFormLayout;
					zLayout->addRow(zLabel, m_zSpinbox);
					boxLayout->addLayout(zLayout);

					layout = boxLayout;
				}
				break;
			}
		}

		assert(layout);

		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
	}

	void Integer3SpinBox::setValue(const Nz::Vector3i64& value)
	{
		m_ignoreSignal = true;

		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);
		m_zSpinbox->setValue(value.z);

		m_ignoreSignal = false;

		onSpinBoxValueChanged();
	}

	Nz::Vector3i64 Integer3SpinBox::value()
	{
		Nz::Vector3i64 vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();
		vec.z = m_zSpinbox->value();

		return vec;
	}

	void Integer3SpinBox::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}

#include "Integer3SpinBox.moc"
