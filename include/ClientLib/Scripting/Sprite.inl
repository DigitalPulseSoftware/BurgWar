// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/Sprite.hpp>

namespace bw
{
	inline Sprite::Sprite(LayerVisualEntityHandle visualEntity, std::shared_ptr<Nz::Sprite> sprite, const Nz::Matrix4f& transformMatrix, int renderOrder) :
	m_visualEntity(std::move(visualEntity)),
	m_transformMatrix(transformMatrix),
	m_sprite(std::move(sprite)),
	m_renderOrder(renderOrder),
	m_isVisible(false)
	{
	}

	inline Nz::Vector2f Sprite::GetOrigin() const
	{
		return Nz::Vector2f(m_sprite->GetOrigin()) / m_sprite->GetSize();
	}

	inline const Nz::Vector2f& Sprite::GetSize() const
	{
		return m_sprite->GetSize();
	}

	inline void Sprite::Hide()
	{
		return Show(false);
	}

	inline bool Sprite::IsValid() const
	{
		return m_visualEntity.IsValid();
	}

	inline bool Sprite::IsVisible() const
	{
		return m_isVisible;
	}

	inline void Sprite::SetOffset(const Nz::Vector2f& newOffset)
	{
		m_transformMatrix.SetTranslation(newOffset);

		UpdateTransformMatrix();
	}

	inline void Sprite::SetRotation(const Nz::DegreeAnglef& newRotation)
	{
		m_transformMatrix.SetRotation(newRotation);

		UpdateTransformMatrix();
	}

	inline void Sprite::SetSize(const Nz::Vector2f& newSize)
	{
		m_sprite->SetSize(newSize);
	}
}
