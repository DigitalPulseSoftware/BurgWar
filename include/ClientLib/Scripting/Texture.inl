// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>

namespace bw
{
	inline Texture::Texture(Nz::TextureRef texture) :
	m_texture(std::move(texture))
	{
	}

	inline Nz::Vector2ui Texture::GetSize() const
	{
		return Nz::Vector2ui(m_texture->GetSize());
	}
}
