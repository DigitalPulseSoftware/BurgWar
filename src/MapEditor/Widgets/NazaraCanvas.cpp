// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Nazara Development Kit Qt Layer"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <QtGui/QKeyEvent>
#include <iostream>
#include <optional>

namespace bw
{
	NazaraCanvas::NazaraCanvas(QWidget* parent) :
	QWidget(parent)
	{
		std::cout << this << std::endl;

		// Setup some states to allow direct rendering into the widget
		setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_Hover);

		// Set strong focus to enable keyboard events to be received
		setFocusPolicy(Qt::StrongFocus);

		//setMouseTracking(true);

		auto mousePos = QWidget::mapFromGlobal(QCursor::pos());

		setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

		m_updateTimer.setInterval(1000 / 60);
		m_updateTimer.connect(&m_updateTimer, &QTimer::timeout, [this]()
		{
			OnUpdate(m_updateTimer.intervalAsDuration().count() / 1000.f);
		});
	}

	NazaraCanvas::~NazaraCanvas()
	{
		m_updateTimer.stop();
	}

	Nz::Vector2ui NazaraCanvas::GetSize() const
	{
		return Nz::Vector2ui(Nz::Vector2i(width(), height()));
	}

	QSize NazaraCanvas::minimumSizeHint() const
	{
		return QSize(640, 480);
	}

	QSize NazaraCanvas::sizeHint() const
	{
		return QSize();
	}

	void NazaraCanvas::resizeEvent(QResizeEvent*)
	{
		OnWindowResized();
	}

	void NazaraCanvas::showEvent(QShowEvent*)
	{
		if (!IsValid())
		{
			#ifdef Q_WS_X11
			XFlush(QX11Info::display());
			#endif

			Nz::RenderWindow::Create(reinterpret_cast<Nz::WindowHandle>(winId()));
		}

		m_updateTimer.start();
	}

	QPaintEngine* NazaraCanvas::paintEngine() const
	{
		return nullptr;
	}

	void NazaraCanvas::OnUpdate(float elapsedTime)
	{
		ProcessEvents();
		Display();
	}

	void NazaraCanvas::closeEvent(QCloseEvent* event)
	{
		m_updateTimer.stop();
	}

	void NazaraCanvas::hideEvent(QHideEvent* event)
	{
		m_updateTimer.stop();
	}

	void NazaraCanvas::paintEvent(QPaintEvent*)
	{
	}

	bool NazaraCanvas::event(QEvent* e)
	{
		auto TranslateMouseButton = [](Qt::MouseButton mouseButton) -> std::optional<Nz::Mouse::Button>
		{
			switch (mouseButton)
			{
				case Qt::LeftButton:   return Nz::Mouse::Left;
				case Qt::MiddleButton: return Nz::Mouse::Middle;
				case Qt::RightButton:  return Nz::Mouse::Right;
				case Qt::XButton1:     return Nz::Mouse::XButton1;
				case Qt::XButton2:     return Nz::Mouse::XButton2;

				default:
					return std::nullopt;
			}
		};

		switch (e->type())
		{
			case QEvent::MouseButtonPress:
			{
				QMouseEvent* mouseButtonEvent = static_cast<QMouseEvent*>(e);
				auto button = TranslateMouseButton(mouseButtonEvent->button());

				if (button)
				{
					auto pos = mouseButtonEvent->pos();

					Nz::WindowEvent event;
					event.type = Nz::WindowEventType_MouseButtonPressed;
					event.mouseButton.x = pos.x();
					event.mouseButton.y = pos.y();
					event.mouseButton.button = button.value();

					PushEvent(event);
					return true;
				}
			}

			case QEvent::MouseButtonRelease:
			{
				QMouseEvent* mouseButtonEvent = static_cast<QMouseEvent*>(e);
				auto button = TranslateMouseButton(mouseButtonEvent->button());

				if (button)
				{
					auto pos = mouseButtonEvent->pos();

					Nz::WindowEvent event;
					event.type = Nz::WindowEventType_MouseButtonReleased;
					event.mouseButton.x = pos.x();
					event.mouseButton.y = pos.y();
					event.mouseButton.button = button.value();

					PushEvent(event);
					return true;
				}
			}

			case QEvent::HoverEnter:
			{
				QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(e);

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType_MouseEntered;

				PushEvent(event);
				return true;
			}

			case QEvent::HoverLeave:
			{
				QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(e);

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType_MouseLeft;

				PushEvent(event);
				return true;
			}

			case QEvent::HoverMove:
			{
				QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(e);
				auto newPos = hoverEvent->pos();
				auto oldPos = hoverEvent->oldPos();

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType_MouseMoved;
				event.mouseMove.x = newPos.x();
				event.mouseMove.y = newPos.y();
				event.mouseMove.deltaX = newPos.x() - oldPos.x();
				event.mouseMove.deltaY = newPos.y() - oldPos.y();

				PushEvent(event);
				return true;
			}

			default:
				break;
		}
		return QWidget::event(e);
	}

	void NazaraCanvas::keyPressEvent(QKeyEvent* key)
	{
	}
}
