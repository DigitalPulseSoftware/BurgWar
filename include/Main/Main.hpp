// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAIN_HPP
#define BURGWAR_MAIN_HPP

extern int BurgMain(int argc, char* argv[], int(*mainFunc)(int argc, char* argv[]));

#define BurgWarMain(FuncName) int main(int argc, char* argv[]) \
{ \
	return BurgMain(argc, argv, &FuncName);\
}

#include <Main/Main.inl>

#endif
