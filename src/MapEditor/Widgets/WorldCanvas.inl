// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/WorldCanvas.hpp>

namespace bw
{
	inline const CameraMovement& WorldCanvas::GetCameraController() const
	{
		return *m_cameraMovement;
	}

	inline Camera& WorldCanvas::GetCamera()
	{
		return m_camera;
	}

	inline const Camera& WorldCanvas::GetCamera() const
	{
		return m_camera;
	}

	inline Ndk::World& WorldCanvas::GetWorld()
	{
		return m_world;
	}

	inline const Ndk::World& WorldCanvas::GetWorld() const
	{
		return m_world;
	}
}