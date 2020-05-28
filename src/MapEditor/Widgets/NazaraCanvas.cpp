// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Nazara Development Kit Qt Layer"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <MapEditor/Widgets/NazaraCanvas.hpp>
#include <QtGui/QKeyEvent>
#include <optional>

namespace bw
{
	NazaraCanvas::NazaraCanvas(QWidget* parent) :
	QWidget(parent)
	{
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

			Nz::RenderWindow::Create(reinterpret_cast<void*>(winId()));
		}

		OnShow();
	}

	QPaintEngine* NazaraCanvas::paintEngine() const
	{
		return nullptr;
	}

	void NazaraCanvas::OnHide()
	{
		m_updateTimer.stop();

		Nz::WindowEvent event;
		event.type = Nz::WindowEventType_LostFocus;

		PushEvent(event);
	}

	void NazaraCanvas::OnShow()
	{
		m_updateTimer.start();

		Nz::WindowEvent event;
		event.type = Nz::WindowEventType_GainedFocus;

		PushEvent(event);
	}

	void NazaraCanvas::OnUpdate(float /*elapsedTime*/)
	{
		ProcessEvents();
		Display();
	}

	void NazaraCanvas::closeEvent(QCloseEvent* /*event*/)
	{
		OnHide();
	}

	void NazaraCanvas::hideEvent(QHideEvent* /*event*/)
	{
		OnHide();
	}

	void NazaraCanvas::paintEvent(QPaintEvent*)
	{
	}

	bool NazaraCanvas::event(QEvent* e)
	{
		auto TranslateKey = [](Qt::Key key) -> std::optional<Nz::Keyboard::VKey>
		{
			switch (key)
			{
				case Qt::Key_Delete: return Nz::Keyboard::VKey::Delete;
				case Qt::Key_Escape: return Nz::Keyboard::VKey::Escape;
				case Qt::Key_Return: return Nz::Keyboard::VKey::Return;
				case Qt::Key_Tab:    return Nz::Keyboard::VKey::Tab;

				default:
					return std::nullopt;
			}
		};

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
			case QEvent::KeyPress:
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
				auto key = TranslateKey(static_cast<Qt::Key>(keyEvent->key()));

				bool ignoreEvent = false;
				if (key)
				{
					Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

					Nz::WindowEvent event;
					event.type = Nz::WindowEventType_KeyPressed;
					event.key.alt = modifiers & Qt::AltModifier;
					event.key.control = modifiers & Qt::ControlModifier;
					event.key.repeated = keyEvent->isAutoRepeat();
					event.key.shift = modifiers & Qt::ShiftModifier;
					event.key.system = modifiers & Qt::MetaModifier;
					event.key.virtualKey = key.value();
					event.key.scancode = Nz::Keyboard::ToScanCode(event.key.virtualKey);

					PushEvent(event);
					ignoreEvent = true;
				}

				// This is a placeholder for a real event handling
				std::u32string u32str = keyEvent->text().toStdU32String();
				if (!u32str.empty())
				{
					Nz::WindowEvent event;
					event.type = Nz::WindowEventType_TextEntered;
					event.text.character = u32str[0];
					event.text.repeated = keyEvent->isAutoRepeat();

					PushEvent(event);
				}

				if (ignoreEvent)
					return true;

				break;
			}

			case QEvent::KeyRelease:
			{
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
				auto key = TranslateKey(static_cast<Qt::Key>(keyEvent->key()));

				if (key)
				{
					Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

					Nz::WindowEvent event;
					event.type = Nz::WindowEventType_KeyReleased;
					event.key.alt = modifiers & Qt::AltModifier;
					event.key.control = modifiers & Qt::ControlModifier;
					event.key.repeated = keyEvent->isAutoRepeat();
					event.key.shift = modifiers & Qt::ShiftModifier;
					event.key.system = modifiers & Qt::MetaModifier;
					event.key.virtualKey = key.value();
					event.key.scancode = Nz::Keyboard::ToScanCode(event.key.virtualKey);

					PushEvent(event);
					return true;
				}
				break;
			}

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
				Nz::WindowEvent event;
				event.type = Nz::WindowEventType_MouseEntered;

				PushEvent(event);
				return true;
			}

			case QEvent::HoverLeave:
			{
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

			case QEvent::Wheel:
			{
				QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType_MouseWheelMoved;
				event.mouseWheel.delta = wheelEvent->angleDelta().ry() / 120.f;

				PushEvent(event);
				return true;
			}

			default:
				break;
		}
		return QWidget::event(e);
	}
}
