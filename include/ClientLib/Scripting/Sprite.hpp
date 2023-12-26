// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_SPRITE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_SPRITE_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Matrix4.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <sol/forward.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API Sprite
	{
		public:
			inline Sprite(LayerVisualEntityHandle visualEntity, std::shared_ptr<Nz::Sprite> sprite, const Nz::Vector2f& offset, const Nz::DegreeAnglef& rotation);
			Sprite(const Sprite&) = delete;
			Sprite(Sprite&&) noexcept = default;
			~Sprite() = default;

			inline Nz::Vector2f GetOrigin() const;
			inline const Nz::Vector2f& GetSize() const;

			inline void Hide();

			inline bool IsValid() const;
			inline bool IsVisible() const;

			void SetColor(const Nz::Color& color);
			void SetCornerColor(const std::string_view& corner, const Nz::Color& cornerColor);
			void SetCornerColors(const sol::table& cornerTable);
			inline void SetOffset(const Nz::Vector2f& newOffset);
			inline void SetRotation(const Nz::DegreeAnglef& newRotation);
			inline void SetSize(const Nz::Vector2f& newSize);
			void Show(bool show = true);

			Sprite& operator=(const Sprite&) = delete;
			Sprite& operator=(Sprite&&) noexcept = default;

		private:
			void UpdateTransform();

			LayerVisualEntityHandle m_visualEntity;
			Nz::DegreeAnglef m_rotation;
			Nz::Vector2f m_offset;
			std::shared_ptr<Nz::Sprite> m_sprite;
			bool m_isVisible;
	};
}

#include <ClientLib/Scripting/Sprite.inl>

#endif
