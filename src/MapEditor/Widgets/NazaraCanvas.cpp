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

		setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

		m_updateTimer.setInterval(1000 / 60);
		m_updateTimer.connect(&m_updateTimer, &QTimer::timeout, [this]()
		{
			OnUpdate(Nz::Time::FromDuration(m_updateTimer.intervalAsDuration()));
		});
	}

	NazaraCanvas::~NazaraCanvas()
	{
		m_updateTimer.stop();
	}

	QSize NazaraCanvas::minimumSizeHint() const
	{
		return QSize(640, 480);
	}

	QSize NazaraCanvas::sizeHint() const
	{
		return QSize();
	}

	void NazaraCanvas::resizeEvent(QResizeEvent* resize)
	{
		Nz::WindowEvent event;
		event.type = Nz::WindowEventType::Resized;
		event.size.width = resize->size().width();
		event.size.height = resize->size().height();

		HandleEvent(event);
	}

	void NazaraCanvas::showEvent(QShowEvent*)
	{
		if (!IsValid())
		{
			#ifdef Q_WS_X11
			XFlush(QX11Info::display());
			#endif

			Nz::WindowHandle windowHandle;
#if defined(NAZARA_PLATFORM_WINDOWS)
			windowHandle.type = Nz::WindowBackend::Windows;
			windowHandle.windows.window = reinterpret_cast<void*>(winId());
#elif defined(NAZARA_PLATFORM_LINUX)
			windowHandle.type = Nz::WindowBackend::X11;
			windowHandle.x11.display = nullptr;
			windowHandle.x11.window = winId();
#elif defined(NAZARA_PLATFORM_MACOS)
			windowHandle.type = Nz::WindowBackend::Cocoa;
			windowHandle.cocoa.window = reinterpret_cast<void*>(winId());
#endif

			Create(windowHandle);
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
		event.type = Nz::WindowEventType::LostFocus;

		HandleEvent(event);
	}

	void NazaraCanvas::OnShow()
	{
		m_updateTimer.start();

		Nz::WindowEvent event;
		event.type = Nz::WindowEventType::GainedFocus;

		HandleEvent(event);
	}

	void NazaraCanvas::OnUpdate(Nz::Time /*elapsedTime*/)
	{
		ProcessEvents();
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
				case Qt::Key_Shift:  return Nz::Keyboard::VKey::LShift;
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
					event.type = Nz::WindowEventType::KeyPressed;
					event.key.alt = modifiers & Qt::AltModifier;
					event.key.control = modifiers & Qt::ControlModifier;
					event.key.repeated = keyEvent->isAutoRepeat();
					event.key.shift = modifiers & Qt::ShiftModifier;
					event.key.system = modifiers & Qt::MetaModifier;
					event.key.virtualKey = key.value();
					event.key.scancode = Nz::Keyboard::ToScanCode(event.key.virtualKey);

					HandleEvent(event);
					ignoreEvent = true;
				}

				// This is a placeholder for a real event handling
				std::u32string u32str = keyEvent->text().toStdU32String();
				if (!u32str.empty())
				{
					Nz::WindowEvent event;
					event.type = Nz::WindowEventType::TextEntered;
					event.text.character = u32str[0];
					event.text.repeated = keyEvent->isAutoRepeat();

					HandleEvent(event);
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
					event.type = Nz::WindowEventType::KeyReleased;
					event.key.alt = modifiers & Qt::AltModifier;
					event.key.control = modifiers & Qt::ControlModifier;
					event.key.repeated = keyEvent->isAutoRepeat();
					event.key.shift = modifiers & Qt::ShiftModifier;
					event.key.system = modifiers & Qt::MetaModifier;
					event.key.virtualKey = key.value();
					event.key.scancode = Nz::Keyboard::ToScanCode(event.key.virtualKey);

					HandleEvent(event);
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
					event.type = Nz::WindowEventType::MouseButtonPressed;
					event.mouseButton.x = pos.x();
					event.mouseButton.y = pos.y();
					event.mouseButton.button = button.value();

					HandleEvent(event);
					return true;
				}

				break;
			}

			case QEvent::MouseButtonRelease:
			{
				QMouseEvent* mouseButtonEvent = static_cast<QMouseEvent*>(e);
				auto button = TranslateMouseButton(mouseButtonEvent->button());

				if (button)
				{
					auto pos = mouseButtonEvent->pos();

					Nz::WindowEvent event;
					event.type = Nz::WindowEventType::MouseButtonReleased;
					event.mouseButton.x = pos.x();
					event.mouseButton.y = pos.y();
					event.mouseButton.button = button.value();

					HandleEvent(event);
					return true;
				}

				break;
			}

			case QEvent::HoverEnter:
			{
				Nz::WindowEvent event;
				event.type = Nz::WindowEventType::MouseEntered;

				HandleEvent(event);
				return true;
			}

			case QEvent::HoverLeave:
			{
				Nz::WindowEvent event;
				event.type = Nz::WindowEventType::MouseLeft;

				HandleEvent(event);
				return true;
			}

			case QEvent::HoverMove:
			{
				QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(e);
				auto newPos = hoverEvent->pos();
				auto oldPos = hoverEvent->oldPos();

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType::MouseMoved;
				event.mouseMove.x = newPos.x();
				event.mouseMove.y = newPos.y();
				event.mouseMove.deltaX = newPos.x() - oldPos.x();
				event.mouseMove.deltaY = newPos.y() - oldPos.y();

				HandleEvent(event);
				return true;
			}

			case QEvent::Wheel:
			{
				QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);

				Nz::WindowEvent event;
				event.type = Nz::WindowEventType::MouseWheelMoved;
				event.mouseWheel.delta = wheelEvent->angleDelta().ry() / 120.f;

				HandleEvent(event);
				return true;
			}

			default:
				break;
		}
		return QWidget::event(e);
	}
}
