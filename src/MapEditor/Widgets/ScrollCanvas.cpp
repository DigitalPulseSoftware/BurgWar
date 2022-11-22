// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/ScrollCanvas.hpp>
#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>

namespace bw
{
	ScrollCanvas::ScrollCanvas(QWidget* parent) :
	QDialog(parent),
	m_contentSize(128, 128),
	m_listenToScrollbarEvents(true)
	{
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		QGridLayout* layout = new QGridLayout;

		m_canvas = new WorldCanvas;
		m_canvas->EnableCameraControl(true);
		m_canvas->setMinimumSize(64, 64);
		m_canvas->setMaximumSize(m_contentSize.x, m_contentSize.y);
		//m_canvas->resize(std::max(m_contentSize.x, 640), std::max(m_contentSize.y, 360));
		m_canvas->OnCameraMoved.Connect(this, &ScrollCanvas::OnCameraMoved);

		m_scrollbarH = new QScrollBar(Qt::Horizontal);
		m_scrollbarH->setTracking(true);
		connect(m_scrollbarH, &QScrollBar::valueChanged, [this](int newValue)
		{
			OnScrollbarMoved(m_scrollbarH->orientation(), float(newValue) / m_scrollbarH->maximum());
		});

		m_scrollbarV = new QScrollBar(Qt::Vertical);
		m_scrollbarV->setTracking(true);
		connect(m_scrollbarV, &QScrollBar::valueChanged, [this](int newValue)
		{
			OnScrollbarMoved(m_scrollbarV->orientation(), float(newValue) / m_scrollbarV->maximum());
		});

		layout->addWidget(m_scrollbarV, 0, 1);
		layout->addWidget(m_canvas, 0, 0);
		layout->addWidget(m_scrollbarH, 1, 0);

		setLayout(layout);
	}

	void ScrollCanvas::EnableCameraControl(bool enable)
	{
		m_canvas->EnableCameraControl(enable);
	}

	Camera& ScrollCanvas::GetCamera()
	{
		return m_canvas->GetCamera();
	}

	const Camera& ScrollCanvas::GetCamera() const
	{
		return m_canvas->GetCamera();
	}

	entt::registry& ScrollCanvas::GetWorld()
	{
		return m_canvas->GetWorld();
	}

	const entt::registry& ScrollCanvas::GetWorld() const
	{
		return m_canvas->GetWorld();
	}

	void ScrollCanvas::SetContentSize(const Nz::Vector2i& contentSize)
	{
		m_contentSize = contentSize;

		m_canvas->setMaximumSize(m_contentSize.x, m_contentSize.y);
		m_canvas->resize(std::max(m_contentSize.x, m_canvas->width()), std::max(m_contentSize.y, m_canvas->height()));

		// Call OnCameraMoved to bounds camera to new size
		OnCameraMoved(m_canvas);
	}

	void ScrollCanvas::UpdateBackgroundColor(Nz::Color color)
	{
		m_canvas->UpdateBackgroundColor(color);
	}

	void ScrollCanvas::OnCameraMoved(WorldCanvas* canvas)
	{
		QSize canvasSize = m_canvas->size();
		int visibleWidth = canvasSize.width();
		int visibleHeight = canvasSize.height();

		auto& camera = canvas->GetCamera();

		// Clamp camera within tile map
		float maximumPosX = m_contentSize.x - visibleWidth;
		float maximumPosY = m_contentSize.y - visibleHeight;

		Nz::Vector2f cameraPos = camera.GetPosition();
		cameraPos.x = std::clamp(cameraPos.x, 0.f, maximumPosX);
		cameraPos.y = std::clamp(cameraPos.y, 0.f, maximumPosY);
		camera.MoveToPosition(cameraPos);

		m_listenToScrollbarEvents = false;

		m_scrollbarH->setSliderPosition(int(cameraPos.x / maximumPosX * m_scrollbarH->maximum()));
		m_scrollbarV->setSliderPosition(int(cameraPos.y / maximumPosY * m_scrollbarV->maximum()));

		m_listenToScrollbarEvents = true;
	}

	void ScrollCanvas::OnScrollbarMoved(Qt::Orientation orientation, float valuePct)
	{
		if (!m_listenToScrollbarEvents)
			return;

		auto& camera = m_canvas->GetCamera();

		QSize canvasSize = m_canvas->size();

		switch (orientation)
		{
			case Qt::Horizontal:
				camera.MoveToPosition({ valuePct * (m_contentSize.x - canvasSize.width()), camera.GetPosition().y });
				break;

			case Qt::Vertical:
				camera.MoveToPosition({ camera.GetPosition().x, valuePct * (m_contentSize.y - canvasSize.height()) });
				break;

			default:
				break;
		}
	}

	void ScrollCanvas::resizeEvent(QResizeEvent* /*event*/)
	{
		QSize canvasSize = m_canvas->size();
		int visibleWidth = canvasSize.width();
		int visibleHeight = canvasSize.height();

		if (visibleWidth < m_contentSize.x)
		{
			m_scrollbarH->show();
			m_scrollbarH->setRange(0, (10 * m_contentSize.x) / visibleWidth);
		}
		else
			m_scrollbarH->hide();

		if (visibleHeight < m_contentSize.y)
		{
			m_scrollbarV->show();
			m_scrollbarV->setRange(0, (10 * m_contentSize.y) / visibleHeight);
		}
		else
			m_scrollbarV->hide();
	}
}
