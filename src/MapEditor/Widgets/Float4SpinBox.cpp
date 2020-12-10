// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Float4SpinBox.hpp>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <limits>

namespace bw
{
	Float4SpinBox::Float4SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	QWidget(parent),
	m_ignoreSignal(false)
	{
		auto onValueChanged = [this](double) 
		{ 
			if (!m_ignoreSignal)
				onSpinBoxValueChanged();
		};

		m_xSpinbox = new QDoubleSpinBox;
		m_xSpinbox->setDecimals(6);
		m_xSpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_xSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		m_ySpinbox = new QDoubleSpinBox;
		m_ySpinbox->setDecimals(6);
		m_ySpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_ySpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		m_zSpinbox = new QDoubleSpinBox;
		m_zSpinbox->setDecimals(6);
		m_zSpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_zSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		m_wSpinbox = new QDoubleSpinBox;
		m_wSpinbox->setDecimals(6);
		m_wSpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_wSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

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

	void Float4SpinBox::setValue(const Nz::Vector4f& value)
	{
		m_ignoreSignal = true;

		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);
		m_zSpinbox->setValue(value.z);
		m_wSpinbox->setValue(value.w);

		m_ignoreSignal = false;

		onSpinBoxValueChanged();
	}

	Nz::Vector4f Float4SpinBox::value()
	{
		Nz::Vector4f vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();
		vec.z = m_zSpinbox->value();
		vec.w = m_wSpinbox->value();

		return vec;
	}

	void Float4SpinBox::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}
