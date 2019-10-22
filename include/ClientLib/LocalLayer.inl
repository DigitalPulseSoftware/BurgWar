// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>

namespace bw
{
	inline const Ndk::EntityHandle& LocalLayer::GetCameraEntity()
	{
		return m_camera;
	}

	inline SharedWorld& bw::LocalLayer::GetWorld()
	{
		return m_world;
	}
}
