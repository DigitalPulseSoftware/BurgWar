// Copyright(C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Map.hpp>
#include <Main/Main.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/std.h>
#include <filesystem>
#include <stdexcept>

int BurgWarMapTool(int argc, char* argv[])
{
	cxxopts::Options options("BurgWarMapTool", "Tool for compiling BurgWar maps in CLI");
	options.add_options()
		("c,compile", "Compile input maps to binary map format")
		("i,input", "Input file(s)", cxxopts::value<std::vector<std::string>>())
		("o,output", "Output folder", cxxopts::value<std::string>()->default_value("."), "path")
		("s,show", "Show informations about the map (default)")
		("h,help", "Print usage")
	;

	options.parse_positional("input");
	options.positional_help("MAPS");

	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("help") > 0)
		{
			fmt::print("{}\n", options.help());
			return EXIT_SUCCESS;
		}

		if (result.count("input") == 0)
		{
			fmt::print("no input files\n{}\n", options.help());
			return EXIT_SUCCESS;
		}

		std::vector<std::string> inputMaps = result["input"].as<std::vector<std::string>>();
		for (const std::string& inputMap : inputMaps)
		{
			std::filesystem::path inputPath = inputMap;
			std::filesystem::path mapName;

			if (inputMaps.size() > 1)
				fmt::print("--- {0} ---\n", inputPath);

			bw::Map map;

			try
			{
				if (std::filesystem::is_directory(inputPath))
				{
					map = bw::Map::LoadFromDirectory(inputPath);
					if (inputPath.has_filename())
						mapName = inputPath.filename(); //< foo/bar => bar
					else
						mapName = inputPath.parent_path().filename(); //< foo/bar/ => bar
				}
				else if (std::filesystem::is_regular_file(inputPath))
				{
					map = bw::Map::LoadFromBinary(inputPath);
					mapName = inputPath.stem(); //< foo/bar.bmap => bar
				}
				else if (std::filesystem::exists(inputPath))
					throw std::runtime_error(inputMap + " is neither a directory nor a binary");
				else
					throw std::runtime_error(inputMap + " doesn't exist");
			}
			catch (const std::exception& e)
			{
				fmt::print(stderr, "{0}: {1}\n", inputMap, e.what());
				continue;
			}

			if (result.count("compile") > 0)
			{
				std::filesystem::path outputPath = result["output"].as<std::string>();
				if (!std::filesystem::is_directory(outputPath))
					std::filesystem::create_directories(outputPath);

				outputPath /= mapName;
				if (outputPath.extension() != "bmap")
					outputPath += ".bmap";

				if (!map.Compile(outputPath))
					throw std::runtime_error("failed to compile map: failed to open " + Nz::PathToString(outputPath));

				fmt::print("successfully compiled map {0} to {1}\n", inputMap, outputPath);
			}
			else
			{
				// Show info about the map
				const auto& mapInfo = map.GetMapInfo();

				fmt::print("{input_map} info:\n- Name: {name}\n- Description: {desc}\n- Author: {author}\n", 
					fmt::arg("input_map", inputMap), 
					fmt::arg("name", mapInfo.name), 
					fmt::arg("desc", mapInfo.description), 
					fmt::arg("author", mapInfo.author));

				std::size_t layerCount = map.GetLayerCount();
				fmt::print("\nThis map has {} layer(s):\n", layerCount);
				for (std::size_t i = 0; i < layerCount; ++i)
				{
					const auto& layer = map.GetLayer(static_cast<bw::LayerIndex>(i));
					fmt::print("- layer #{} ({}) has {} entities\n", i, layer.name, layer.entities.size());
				}
			}
		}
	}
	catch (const cxxopts::exceptions::exception& e)
	{
		fmt::print(stderr, "{}\n{}\n", e.what(), options.help());
	}
	catch (const std::exception& e)
	{
		fmt::print(stderr, "{}\n", e.what());
	}

	return EXIT_SUCCESS;
}

BurgWarMain(BurgWarMapTool)
