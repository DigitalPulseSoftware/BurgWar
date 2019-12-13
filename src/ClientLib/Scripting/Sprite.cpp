// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>

namespace bw
{
	/*bool Sprite::IsVisible() const
	{
		assert(m_entity);

		auto& gfxComponent = m_entity->GetComponent<Ndk::GraphicsComponent>();
		return true;
	}*/

	void Sprite::Show(bool show)
	{
		assert(m_entity);

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
