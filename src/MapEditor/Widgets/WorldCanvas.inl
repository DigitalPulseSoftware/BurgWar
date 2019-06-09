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

	inline const Ndk::EntityHandle& WorldCanvas::GetCameraEntity()
	{
		return m_cameraEntity;
	}

	inline const Ndk::EntityHandle& WorldCanvas::GetCameraEntity() const
	{
		return m_cameraEntity;
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