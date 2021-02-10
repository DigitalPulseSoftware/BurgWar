// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Text.hpp>
#include <Nazara/Core/Color.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <optional>
#include <stdexcept>

namespace bw
{
	/*bool Text::IsVisible() const
	{
		if (!m_entity)
			throw std::runtime_error("Invalid text");

		auto& gfxComponent = m_entity->GetComponent<Ndk::GraphicsComponent>();
		return true;
	}*/
	
	void Text::SetColor(Nz::Color color)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid text");

		m_textSprite->SetColor(color);
	}

	void Text::SetHoveringHeight(float hoveringHeight)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid text");

		if (!m_isHovering)
			throw std::runtime_error("Text is not hovering");

		m_hoveringHeight = hoveringHeight;
		if (m_isVisible)
			m_entity->UpdateHoveringRenderableHoveringHeight(m_textSprite, m_hoveringHeight);
	}

	void Text::Show(bool show)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid text");

		if (show == m_isVisible)
			return;

		if (m_isHovering)
		{
			if (show)
				m_entity->AttachHoveringRenderable(m_textSprite, m_transformMatrix, m_renderOrder, m_hoveringHeight);
			else
				m_entity->DetachHoveringRenderable(m_textSprite);
		}
		else
		{
			if (show)
				m_entity->AttachRenderable(m_textSprite, m_transformMatrix, m_renderOrder);
			else
				m_entity->DetachRenderable(m_textSprite);
		}

		m_isVisible = show;
	}
	
	void Text::UpdateTextSprite()
	{
		m_textSprite->Update(m_drawer);
	}

	void Text::UpdateTransformMatrix()
	{
		if (m_isHovering)
			m_entity->UpdateHoveringRenderableMatrix(m_textSprite, m_transformMatrix);
		else
			m_entity->UpdateRenderableMatrix(m_textSprite, m_transformMatrix);
	}
}
