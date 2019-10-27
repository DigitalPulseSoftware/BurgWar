// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_TEXTURE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_TEXTURE_HPP

#include <Nazara/Renderer/Texture.hpp>

namespace bw
{
	class Texture
	{
		public:
			inline Texture(Nz::TextureRef texture);
			Texture(const Texture&) = default;
			Texture(Texture&&) = default;
			~Texture() = default;

			inline Nz::Vector2ui GetSize() const;

			Texture& operator=(const Texture&) = default;
			Texture& operator=(Texture&&) = default;

		private:
			Nz::TextureRef m_texture;
	};
}

#include <ClientLib/Scripting/Texture.inl>

#endif
