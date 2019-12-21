// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Integer2SpinBox.hpp>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <limits>

namespace bw
{
	Integer2SpinBox::Integer2SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	QWidget(parent)
	{
		auto onValueChanged = [this](int) { onSpinBoxValueChanged(); };

		// TODO: Handle properly int64

		m_xSpinbox = new QSpinBox;
		m_xSpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_xSpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);

		m_ySpinbox = new QSpinBox;
		m_ySpinbox->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		connect(m_ySpinbox, qOverload<int>(&QSpinBox::valueChanged), onValueChanged);
		
		QLayout* layout = nullptr;

		switch (labelMode)
		{
			case LabelMode::NoLabel:
				layout = new QBoxLayout(dir);
				layout->addWidget(m_xSpinbox);
				layout->addWidget(m_ySpinbox);
				break;
				
			case LabelMode::PositionLabel:
			case LabelMode::SizeLabel:
			{
				QString xLabel;
				QString yLabel;

				if (labelMode == LabelMode::SizeLabel)
				{
					xLabel = tr("Width");
					yLabel = tr("Height");
				}
				else
				{
					xLabel = "X";
					yLabel = "Y";
				}

				if (dir == QBoxLayout::TopToBottom)
				{
					QFormLayout* formLayout = new QFormLayout;
					formLayout->addRow(xLabel, m_xSpinbox);
					formLayout->addRow(yLabel, m_ySpinbox);

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

					layout = boxLayout;
				}
				break;
			}
		}

		assert(layout);

		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
	}

	void Integer2SpinBox::setValue(const Nz::Vector2i64& value)
	{
		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);
	}

	Nz::Vector2i64 Integer2SpinBox::value()
	{
		Nz::Vector2i64 vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();

		return vec;
	}

	void Integer2SpinBox::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}

#include "Integer2SpinBox.moc"
