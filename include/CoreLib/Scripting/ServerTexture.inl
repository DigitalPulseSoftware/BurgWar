// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerTexture.hpp>

namespace bw
{
	inline ServerTexture::ServerTexture(Nz::ImageRef texture) :
	m_image(std::move(texture))
	{
	}

	inline Nz::Vector2ui ServerTexture::GetSize() const
	{
		return Nz::Vector2ui(m_image->GetSize());
	}
}
