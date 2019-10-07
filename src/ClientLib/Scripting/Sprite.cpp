// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <iostream>

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

		auto& gfxComponent = m_entity->GetComponent<Ndk::GraphicsComponent>();
		if (show)
			gfxComponent.Attach(m_sprite, m_transformMatrix, m_renderOrder);
		else
			gfxComponent.Detach(m_sprite);
	}
}
