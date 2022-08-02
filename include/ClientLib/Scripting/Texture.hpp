// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_TEXTURE_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_TEXTURE_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Renderer/Texture.hpp>

namespace bw
{
	class BURGWAR_CLIENTLIB_API Texture
	{
		public:
			inline Texture(std::shared_ptr<Nz::Texture> texture);
			Texture(const Texture&) = default;
			Texture(Texture&&) = default;
			~Texture() = default;

			inline Nz::Vector2ui GetSize() const;

			Texture& operator=(const Texture&) = default;
			Texture& operator=(Texture&&) = default;

		private:
			std::shared_ptr<Nz::Texture> m_texture;
	};
}

#include <ClientLib/Scripting/Texture.inl>

#endif
