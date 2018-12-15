// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP

#include <Shared/Scripting/SharedScriptingContext.hpp>

namespace bw
{
	class LocalMatch;

	class ClientScriptingContext : public SharedScriptingContext
	{
		public:
			ClientScriptingContext(LocalMatch& match);
			~ClientScriptingContext() = default;

		private:
			//bool LoadFile(const std::filesystem::path& filePath, std::string* error) override;

			LocalMatch& m_match;
	};
}

#include <Client/Scripting/ClientScriptingContext.inl>

#endif
