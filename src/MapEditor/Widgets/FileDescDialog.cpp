// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/FileDescDialog.hpp>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>

namespace bw
{
	FileDescDialog::FileDescDialog(QWidget* parent) :
	QDialog(parent)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		QFormLayout* formLayout = new QFormLayout;

		m_name = new QLineEdit;
		formLayout->addRow(tr("Name"), m_name);

		m_author = new QLineEdit;
		formLayout->addRow(tr("Author"), m_author);

		m_description = new QTextEdit;
		m_description->setAcceptRichText(false);

		formLayout->addRow(tr("Description"), m_description);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &FileDescDialog::OnAccept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		verticalLayout->addLayout(formLayout);
		verticalLayout->addWidget(buttonBox);

		setLayout(verticalLayout);
	}

	FileDescInfo FileDescDialog::Getnfo() const
	{
		FileDescInfo mapInfo;
		mapInfo.author = m_author->text().toStdString();
		mapInfo.description = m_description->toPlainText().toStdString();
		mapInfo.name = m_name->text().toStdString();

		return mapInfo;
	}

	void FileDescDialog::OnAccept()
	{
		if (m_name->text().isEmpty())
		{
			QMessageBox::critical(this, "Missing informations", "Name must be set", QMessageBox::Ok);
			return;
		}

		accept();
	}
}
