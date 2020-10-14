// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_FILEDESCDIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_FILEDESCDIALOG_HPP

#include <QtWidgets/QDialog>

class QLineEdit;
class QTextEdit;

namespace bw
{
	struct FileDescInfo
	{
		std::string author;
		std::string description;
		std::string name;
	};

	class FileDescDialog : public QDialog
	{
		public:
			FileDescDialog(QWidget* parent = nullptr);
			~FileDescDialog() = default;

			FileDescInfo GetInfo() const;

		private:
			void OnAccept();

			QLineEdit* m_author;
			QLineEdit* m_name;
			QTextEdit* m_description;
	};
}


#include <MapEditor/Widgets/FileDescDialog.inl>

#endif
