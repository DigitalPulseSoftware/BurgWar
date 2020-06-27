// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>
#include <Nazara/Core/Color.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <optional>
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
	
	void Sprite::SetColor(Nz::Color color)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		m_sprite->SetColor(color);
	}

	void Sprite::SetCornerColor(const std::string_view& corner, Nz::Color cornerColor)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		Nz::RectCorner rectCorner;
		if (corner == "TopLeft")
			rectCorner = Nz::RectCorner_LeftTop;
		else if (corner == "TopRight")
			rectCorner = Nz::RectCorner_RightTop;
		else if (corner == "BottomLeft")
			rectCorner = Nz::RectCorner_LeftBottom;
		else if (corner == "BottomRight")
			rectCorner = Nz::RectCorner_RightBottom;
		else
			throw std::runtime_error("Invalid corner " + std::string(corner));

		m_sprite->SetCornerColor(rectCorner, cornerColor);
	}

	void Sprite::SetCornerColors(const sol::table& cornerTable)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		struct Corner
		{
			const char* varName;
			Nz::RectCorner rectCorner;
		};

		std::array<Corner, 4> corners = {
			{
				{
					"TopLeft",
					Nz::RectCorner_LeftTop
				},
				{
					"TopRight",
					Nz::RectCorner_RightTop
				},
				{
					"BottomLeft",
					Nz::RectCorner_LeftBottom
				},
				{
					"BottomRight",
					Nz::RectCorner_RightBottom
				}
			}
		};

		for (const auto& corner : corners)
		{
			if (std::optional<Nz::Color> colorParameter = cornerTable.get_or<std::optional<Nz::Color>>(corner.varName, std::nullopt); colorParameter)
				m_sprite->SetCornerColor(corner.rectCorner, colorParameter.value());
		}
	}

	void Sprite::Show(bool show)
	{
		if (!m_entity)
			throw std::runtime_error("Invalid sprite");

		if (show == m_isVisible)
			return;

		if (show)
			m_entity->AttachRenderable(m_sprite, m_transformMatrix, m_renderOrder);
		else
			m_entity->DetachRenderable(m_sprite);

		m_isVisible = show;
	}
	
	void Sprite::UpdateTransformMatrix()
	{
		m_entity->UpdateRenderableMatrix(m_sprite, m_transformMatrix);
	}
}
