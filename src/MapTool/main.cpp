// Copyright(C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Map.hpp>
#include <Main/Main.hpp>
#include <cxxopts.hpp>
#include <filesystem>
#include <stdexcept>

int BurgWarMapTool(int argc, char* argv[])
{
	cxxopts::Options options("BurgWarMapTool", "Tool for compiling BurgWar maps in CLI");
	options.add_options()
		("c,compile", "Compilation output", cxxopts::value<std::string>())
		("i,input", "Input file", cxxopts::value<std::string>())
		("h,help", "Print usage")
	;

	options.parse_positional("input");

	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("help") > 0)
		{
			std::cout << options.help() << std::endl;
			return EXIT_SUCCESS;
		}

		if (result.count("input") == 0)
		{
			std::cout << "No input file\n" << options.help() << std::endl;
			return EXIT_SUCCESS;
		}

		std::string input = result["input"].as<std::string>();

		bw::Map map;
		if (std::filesystem::is_directory(input))
			map = bw::Map::LoadFromDirectory(input);
		else if (std::filesystem::is_regular_file(input))
			map = bw::Map::LoadFromBinary(input);
		else if (std::filesystem::exists(input))
			throw std::runtime_error("input map is neither a directory nor a binary");
		else
			throw std::runtime_error("input map doesn't exist");

		if (result.count("compile") > 0)
		{
			std::filesystem::path outputPath = result["compile"].as<std::string>();
			if (!outputPath.has_extension())
				outputPath.replace_extension("bmap");

			if (!map.Compile(outputPath))
				throw std::runtime_error("failed to compile map: failed to open " + outputPath.generic_u8string());

			std::cout << "Successfully compiled map to " << outputPath.generic_u8string() << std::endl;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << e.what() << "\n";
		std::cout << options.help() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

BurgWarMain(BurgWarMapTool)
