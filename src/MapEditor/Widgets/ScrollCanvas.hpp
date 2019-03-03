// Copyright (C) 2019 J�r�me Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_SCROLLCANVAS_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_SCROLLCANVAS_HPP

#include <Nazara/Core/Color.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/World.hpp>
#include <QtWidgets/QDialog>
#include <vector>

class QScrollBar;
class QWidget;

namespace bw
{
	class WorldCanvas;

	class ScrollCanvas : public QDialog
	{
		public:
			ScrollCanvas(QWidget* parent = nullptr);
			~ScrollCanvas() = default;

			void EnableCameraControl(bool enable);

			const Ndk::EntityHandle& GetCameraEntity();
			Ndk::World& GetWorld();
			const Ndk::World& GetWorld() const;

			void SetContentSize(const Nz::Vector2i& contentSize);

			void UpdateBackgroundColor(Nz::Color color);

		private:
			void OnCameraMoved(WorldCanvas* canvas);
			void OnScrollbarMoved(Qt::Orientation orientation, float valuePct);

			void resizeEvent(QResizeEvent* event) override;

			QScrollBar* m_scrollbarH;
			QScrollBar* m_scrollbarV;
			Nz::Vector2i m_contentSize;
			WorldCanvas* m_canvas;
			bool m_listenToScrollbarEvents;
	};
}


#include <MapEditor/Widgets/TileMapEditorDialog.inl>

#endif