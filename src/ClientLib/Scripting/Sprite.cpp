// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>
#include <stdexcept>

namespace bw
{
	/*bool Sprite::IsVisible() const
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		auto& gfxComponent = m_entity->GetComponent<Ndk::GraphicsComponent>();
		return true;
	}*/

	void Sprite::Show(bool show)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		if (show)
			m_entity->AttachRenderable(m_sprite, m_transformMatrix, m_renderOrder);
		else
			m_entity->DetachRenderable(m_sprite);
	}
	
	void Sprite::UpdateTransformMatrix()
	{
		m_entity->UpdateRenderableMatrix(m_sprite, m_transformMatrix);
	}
}
