// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Integer4SpinBox.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <limits>

namespace bw
{
	Integer4SpinBox::Integer4SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
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

		m_wSpinbox = new QSpinBox;
		m_wSpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_wSpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);

		QLayout* layout = nullptr;

		switch (labelMode)
		{
			case LabelMode::NoLabel:
				layout = new QBoxLayout(dir);
				layout->addWidget(m_xSpinbox);
				layout->addWidget(m_ySpinbox);
				layout->addWidget(m_zSpinbox);
				layout->addWidget(m_wSpinbox);
				break;

			case LabelMode::PositionLabel:
			case LabelMode::RectLabel:
			{
				QString xLabel = "X";
				QString yLabel = "Y";
				QString zLabel;
				QString wLabel;

				if (labelMode == LabelMode::PositionLabel)
				{
					zLabel = "Z";
					wLabel = "W";
				}
				else
				{
					zLabel = tr("Width");
					wLabel = tr("Height");
				}

				if (dir == QBoxLayout::TopToBottom)
				{
					QFormLayout* formLayout = new QFormLayout;
					formLayout->addRow(xLabel, m_xSpinbox);
					formLayout->addRow(yLabel, m_ySpinbox);
					formLayout->addRow(zLabel, m_zSpinbox);
					formLayout->addRow(wLabel, m_wSpinbox);

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

					QFormLayout* wLayout = new QFormLayout;
					wLayout->addRow(wLabel, m_wSpinbox);
					boxLayout->addLayout(wLayout);

					layout = boxLayout;
				}
				break;
			}
		}

		assert(layout);

		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
	}

	void Integer4SpinBox::setValue(const Nz::Vector4i64& value)
	{
		m_ignoreSignal = true;

		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);
		m_zSpinbox->setValue(value.z);
		m_wSpinbox->setValue(value.w);

		m_ignoreSignal = false;

		onSpinBoxValueChanged();
	}

	Nz::Vector4i64 Integer4SpinBox::value()
	{
		Nz::Vector4i64 vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();
		vec.z = m_zSpinbox->value();
		vec.w = m_wSpinbox->value();

		return vec;
	}

	void Integer4SpinBox::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}

#include "Integer4SpinBox.moc"
