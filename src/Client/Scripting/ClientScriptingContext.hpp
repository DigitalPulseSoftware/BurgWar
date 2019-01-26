// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_CLIENTSCRIPTINGCONTEXT_HPP

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <Shared/Utility/VirtualDirectory.hpp>

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
