// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Text.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Graphics/MaterialInstance.hpp>
#include <sol/sol.hpp>
#include <optional>
#include <stdexcept>

namespace bw
{
	void Text::SetColor(Nz::Color color)
	{
		if (!m_visualEntity)
			throw std::runtime_error("Invalid text");

		m_textSprite->GetMaterial()->SetValueProperty("BaseColor", color);
	}

	void Text::SetHoveringHeight(float hoveringHeight)
	{
		if (!m_visualEntity)
			throw std::runtime_error("Invalid text");

		if (!m_isHovering)
			throw std::runtime_error("Text is not hovering");

		m_hoveringHeight = hoveringHeight;
		if (m_isVisible)
			m_visualEntity->UpdateHoveringRenderableHoveringHeight(m_textSprite, m_hoveringHeight);
	}

	void Text::Show(bool show)
	{
		if (!m_visualEntity)
			throw std::runtime_error("Invalid text");

		if (show == m_isVisible)
			return;

		if (m_isHovering)
		{
			if (show)
				m_visualEntity->AttachHoveringRenderable(m_textSprite, m_offset, m_rotation, m_hoveringHeight);
			else
				m_visualEntity->DetachHoveringRenderable(m_textSprite);
		}
		else
		{
			if (show)
				m_visualEntity->AttachRenderable(m_textSprite, m_offset, m_rotation);
			else
				m_visualEntity->DetachRenderable(m_textSprite);
		}

		m_isVisible = show;
	}
	
	void Text::UpdateTextSprite()
	{
		m_textSprite->Update(m_drawer);
	}

	void Text::UpdateTransform()
	{
		if (m_isHovering)
			m_visualEntity->UpdateHoveringRenderableTransform(m_textSprite, m_offset, m_rotation);
		else
			m_visualEntity->UpdateRenderableTransform(m_textSprite, m_offset, m_rotation);
	}
}
