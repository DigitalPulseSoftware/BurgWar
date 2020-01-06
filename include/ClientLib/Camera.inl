// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Camera.hpp>

namespace bw
{
	inline float bw::Camera::GetZoomFactor() const
	{
		return m_zoomFactor;
	}

	inline bool bw::Camera::IsPerspective() const
	{
		return m_isPerspective;
	}
}
