// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_NAZARACANVAS_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_NAZARACANVAS_HPP

#include <Nazara/Renderer/RenderWindow.hpp>
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

namespace bw
{
	class NazaraCanvas : public QWidget, public Nz::RenderWindow
	{
		public:
			NazaraCanvas(QWidget* parent = nullptr);
			virtual ~NazaraCanvas();

			Nz::Vector2ui GetSize() const override;

			QSize minimumSizeHint() const override;
			QSize sizeHint() const override;

		protected:
			virtual void OnHide();
			virtual void OnShow();
			virtual void OnUpdate(float elapsedTime);

			void closeEvent(QCloseEvent *event) override;
			void hideEvent(QHideEvent *event) override;
			void paintEvent(QPaintEvent*) override;
			QPaintEngine* paintEngine() const override;
			void resizeEvent(QResizeEvent*) override;
			void showEvent(QShowEvent*) override;

			bool event(QEvent* e) override;

		private:
			QTimer m_updateTimer;
	};
}

#include <MapEditor/Widgets/NazaraCanvas.inl>

#endif
