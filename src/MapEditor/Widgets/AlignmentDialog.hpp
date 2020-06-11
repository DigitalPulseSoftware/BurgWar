// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_ALIGNMENTDIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_ALIGNMENTDIALOG_HPP

#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QDialog>

namespace bw
{
	class Float2SpinBox;

	class AlignmentDialog : public QDialog
	{
		public:
			AlignmentDialog(Nz::Vector2f alignment, QWidget* parent = nullptr);
			~AlignmentDialog() = default;

			Nz::Vector2f GetAlignment() const;

		private:
			void OnAccept();

			Float2SpinBox* m_alignmentWidget;
	};
}


#include <MapEditor/Widgets/AlignmentDialog.inl>

#endif
