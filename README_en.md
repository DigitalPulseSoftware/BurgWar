# Burg'war [![GB flag](.github/images/greatbritainflag.png)](README_en.md)

![Iconic Burgwar burger](.github/images/burger.png)

Burg'war is a food-themed 2D multiplayer platform game where you play as a hamburger fighting against other hamburgers using weapons such as Emmentalibur, Tomace, Potato Thrower, and many more!

In fact, behind this seemingly simple game is a sandbox game (in the same way as Garry's Mod), the whole gameplay part of the game is programmed in Lua and is open to modification. 
So you're free to modify the game, add your weapons, your way of playing, your assets and launch a server! The game handles all the stuff to make people play following the rules of your creativity!

## How to play?

Download the latest [release](https://github.com/DigitalPulseSoftware/BurgWar/releases) is enough to play, you can also download the map editor to make your own maps.

## How to compile it?

Burg'war uses three libraries :

- [NazaraEngine](https://github.com/DigitalPulseSoftware/NazaraEngine) - used by the whole project (download the nightlies or compile it yourself) ;
- [cURL](https://curl.haxx.se/) - required by the client ([binaries for Windows](https://curl.haxx.se/windows/)) ;
- [Qt](https://www.qt.io) - required by the map editor.

Once the binaries of these libraries are installed on your system, copy the `build/config.default.lua` file to `build/config.lua` and fill it in to indicate where your dependencies are located.

This file is used by the generator ([Premake](https://premake.github.io)) to find out where the library binaries (bin) and the headers for inclusion (include) are located.
In Windows, you also need to know where the .libs for include are located (lib).

The format is as follows:
```lua
DependencyName = {
  -- You can specify each folder separately
	BinPath = [[C:\Qt\5.12.6\msvc2017_64\bin]],
	IncludePath = [[C:\Qt\5.12.6\msvc2017_64\include]],
	LibPath = [[C:\Qt\5.12.6\msvc2017_64\lib]],
  
  -- Or if, as in the example above, you have a folder containing bin/include(/lib) subfolders, you can use the following shortcut:
  PackageFolder = [[C:\Qt\5.12.6\msvc2017_64]],
}
```

If the dependency is installed on your system (which is often the case on Linux), you can indicate this with:
```lua
DependencyName = {
  PackageFolder = ":system",
}
```

The three dependencies are `cURL`, `Nazara` and `Qt`. 
Note that `Qt` also requires the `MocPath` option, path to the moc.exe utility (normally found in the bin directory), this is not mandatory but recommended if you want to make changes to the project.

In the case where a dependency is not filled, the projects that depends on it will be ignored.

Once this file is populated, you just need to run `premake5.exe` (`premake5-linux64` on Linux) followed by a spacing and the target you want to generate, commonly `vs2019` for Visual Studio 2019, `gmake` for a makefile, etc. (Call premake without arguments to get a list of supported targets).

From there, compile the project according to the generated target, and you should get executables for the Client (Burgwar), Server (BWServer) and Map Editor (BWMapEditor).
Each of these projects uses a .lua file for its configuration, make sure it is present (and properly configured) in the current directory of the executable before starting it.

You'll only miss the necessary assets (images/sounds/etc.), which you can find by downloading the GitHub releases.
