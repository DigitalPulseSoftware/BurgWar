// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERTEXTURE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERTEXTURE_HPP

#include <Nazara/Utility/Image.hpp>

namespace bw
{
	class ServerTexture
	{
		public:
			inline ServerTexture(Nz::ImageRef image);
			ServerTexture(const ServerTexture&) = default;
			ServerTexture(ServerTexture&&) = default;
			~ServerTexture() = default;

			inline Nz::Vector2ui GetSize() const;

			ServerTexture& operator=(const ServerTexture&) = default;
			ServerTexture& operator=(ServerTexture&&) = default;

		private:
			Nz::ImageRef m_image;
	};
}

#include <CoreLib/Scripting/ServerTexture.inl>

#endif
