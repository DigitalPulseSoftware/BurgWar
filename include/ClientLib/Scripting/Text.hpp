// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_TEXT_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_TEXT_HPP

#include <ClientLib/ClientLayerEntity.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Matrix4.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <entt/entt.hpp>
#include <sol/forward.hpp>

namespace bw
{
	class Text
	{
		public:
			inline Text(LayerVisualEntityHandle visualEntity, Nz::SimpleTextDrawer drawer, std::shared_ptr<Nz::TextSprite> textSprite, const Nz::Matrix4f& transformMatrix, int renderOrder, bool isHovering);
			Text(const Text&) = delete;
			Text(Text&&) = default;
			~Text() = default;

			inline Nz::Vector2f GetSize() const;
			inline std::string GetText() const;

			inline void Hide();

			inline bool IsValid() const;
			inline bool IsVisible() const;

			void SetColor(Nz::Color color);
			void SetHoveringHeight(float hoveringHeight);
			inline void SetOffset(const Nz::Vector2f& newOffset);
			inline void SetRotation(const Nz::DegreeAnglef& newRotation);
			inline void SetText(const std::string& text);
			void Show(bool show = true);

			Text& operator=(const Text&) = delete;
			Text& operator=(Text&&) = default;

		private:
			void UpdateTextSprite();
			void UpdateTransformMatrix();

			LayerVisualEntityHandle m_visualEntity;
			Nz::Matrix4f m_transformMatrix;
			Nz::SimpleTextDrawer m_drawer;
			std::shared_ptr<Nz::TextSprite> m_textSprite;
			bool m_isHovering;
			bool m_isVisible;
			float m_hoveringHeight;
			int m_renderOrder;
	};
}

#include <ClientLib/Scripting/Text.inl>

#endif
