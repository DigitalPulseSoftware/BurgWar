// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP

#include <GameLibShared/Scripting/SharedScriptingContext.hpp>
#include <GameLibShared/Utility/VirtualDirectory.hpp>

namespace bw
{
	class LocalMatch;

	class ClientScriptingContext : public SharedScriptingContext
	{
		public:
			ClientScriptingContext(LocalMatch& match, std::shared_ptr<VirtualDirectory> scriptDir);
			~ClientScriptingContext() = default;

			bool Load(const std::filesystem::path& folderOrFile) override;

		private:
			inline LocalMatch& GetMatch();

			std::shared_ptr<VirtualDirectory> m_scriptDirectory;
	};
}

#include <Client/Scripting/ClientScriptingContext.inl>

#endif
