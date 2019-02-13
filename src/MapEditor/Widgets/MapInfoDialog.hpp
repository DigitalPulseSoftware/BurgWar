// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_MAPINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_MAPINFODIALOG_HPP

#include <CoreLib/Map.hpp>
#include <QtWidgets/QDialog>
#include <filesystem>

class QLineEdit;
class QTextEdit;

namespace bw
{
	class MapInfoDialog : public QDialog
	{
		public:
			MapInfoDialog(QWidget* parent = nullptr);
			~MapInfoDialog() = default;

			MapInfo GetMapInfo() const;

		private:
			void OnAccept();

			QLineEdit* m_author;
			QLineEdit* m_name;
			QTextEdit* m_description;
	};
}


#include <MapEditor/Widgets/MapInfoDialog.inl>

#endif