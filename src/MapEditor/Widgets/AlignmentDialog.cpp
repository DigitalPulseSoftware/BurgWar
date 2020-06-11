// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/AlignmentDialog.hpp>
#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

namespace bw
{
	AlignmentDialog::AlignmentDialog(Nz::Vector2f alignment, QWidget* parent) :
	QDialog(parent)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		QFormLayout* formLayout = new QFormLayout;

		m_alignmentWidget = new Float2SpinBox(Float2SpinBox::LabelMode::PositionLabel, QBoxLayout::TopToBottom);
		m_alignmentWidget->setValue(alignment);

		formLayout->addRow("Alignment value", m_alignmentWidget);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset | QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &AlignmentDialog::OnAccept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
		connect(buttonBox, &QDialogButtonBox::clicked, [=](QAbstractButton* button)
		{
			if (button == buttonBox->button(QDialogButtonBox::Reset))
				m_alignmentWidget->setValue(alignment);
			else if (button == buttonBox->button(QDialogButtonBox::RestoreDefaults))
				m_alignmentWidget->setValue(Nz::Vector2f::Unit());
		});

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(formLayout);
		verticalLayout->addWidget(buttonBox);

		setLayout(verticalLayout);
	}

	Nz::Vector2f AlignmentDialog::GetAlignment() const
	{
		return m_alignmentWidget->value();
	}

	void AlignmentDialog::OnAccept()
	{
		Nz::Vector2f alignment = GetAlignment();
		if (alignment.x <= 0.f || alignment.y <= 0.f)
		{
			QMessageBox::critical(this, tr("Incorrect alignment"), tr("Alignment must be positive and non-zero"), QMessageBox::Ok);
			return;
		}

		accept();
	}
}
