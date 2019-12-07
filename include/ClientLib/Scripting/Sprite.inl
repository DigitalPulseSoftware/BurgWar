// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>

namespace bw
{
	inline Sprite::Sprite(LocalLayerEntityHandle entity, Nz::SpriteRef sprite, const Nz::Matrix4f& transformMatrix, int renderOrder) :
	m_entity(std::move(entity)),
	m_transformMatrix(transformMatrix),
	m_sprite(std::move(sprite)),
	m_renderOrder(renderOrder)
	{
	}

	inline Nz::Vector2f Sprite::GetOrigin() const
	{
		return Nz::Vector2f(m_sprite->GetOrigin());
	}

	inline const Nz::Vector2f& Sprite::GetSize() const
	{
		return m_sprite->GetSize();
	}

	inline void Sprite::Hide()
	{
		return Show(false);
	}

	inline void Sprite::SetSize(const Nz::Vector2f& newSize)
	{
		m_sprite->SetSize(newSize);
	}
}
