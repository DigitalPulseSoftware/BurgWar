// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Text.hpp>

namespace bw
{
	inline Text::Text(LayerVisualEntityHandle visualEntity, Nz::SimpleTextDrawer drawer, Nz::TextSpriteRef textSprite, const Nz::Matrix4f& transformMatrix, int renderOrder, bool isHovering) :
	m_visualEntity(std::move(visualEntity)),
	m_transformMatrix(transformMatrix),
	m_drawer(std::move(drawer)),
	m_textSprite(std::move(textSprite)),
	m_isHovering(isHovering),
	m_isVisible(false),
	m_hoveringHeight(0.f),
	m_renderOrder(renderOrder)
	{
	}

	inline Nz::Vector2f Text::GetSize() const
	{
		Nz::Boxf box = m_textSprite->GetBoundingVolume().obb.localBox;
		return { box.width, box.height };
	}

	inline std::string Text::GetText() const
	{
		return m_drawer.GetText().ToStdString();
	}

	inline void Text::Hide()
	{
		return Show(false);
	}

	inline bool Text::IsValid() const
	{
		return m_visualEntity;
	}

	inline bool Text::IsVisible() const
	{
		return m_isVisible;
	}

	inline void Text::SetOffset(const Nz::Vector2f& newOffset)
	{
		m_transformMatrix.SetTranslation(newOffset);

		UpdateTransformMatrix();
	}

	inline void Text::SetRotation(const Nz::DegreeAnglef& newRotation)
	{
		m_transformMatrix.SetRotation(newRotation);

		UpdateTransformMatrix();
	}
	
	inline void Text::SetText(const std::string& text)
	{
		if (!m_visualEntity)
			throw std::runtime_error("Invalid text");

		m_drawer.SetText(text);

		UpdateTextSprite();
	}
}
