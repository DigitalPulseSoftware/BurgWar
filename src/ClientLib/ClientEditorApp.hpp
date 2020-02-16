// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP
#define BURGWAR_CLIENTLIB_CLIENTEDITORAPP_HPP

#include <CoreLib/BurgApp.hpp>
#include <NDK/Application.hpp>

namespace bw
{
	class ClientEditorApp : public Ndk::Application, public BurgApp
	{
		public:
			ClientEditorApp(int argc, char* argv[], LogSide side);
			~ClientEditorApp();

		protected:
			void FillStores();
	};
}

#include <ClientLib/ClientEditorApp.inl>

#endif
