# Changelog

## master (unreleased)

* 

## Scripting

* Gamemode events are now triggered on the table they used to register the event (this is important when using gamemode inheritance)
* Added IncomingNetworkPacket:ReadColor() and OutgoingNetworkPacket:WriteColor()
* Added match.GetPlayerByIndex()
* match.BroadcastPacket now has a second optional parameter to disable ready check

## Beta 1.1

### General

* Reworked assets and scripts downloading (assets can now be downloaded directly from server).
* Console colors are now supported on Linux
* The game is now able to load pre-beta maps
* Fixed crash on exit

### Config
* Added Debug.ShowVersion (true by default) which shows the game version in a corner
* Added GameSettings.FastDownloadURLs config
* Renamed Assets.ResourceFolder to Resources.AssetDirectory
* Renamed Assets.ScriptFolder to Resources.ScriptDirectory

### Build
* Enabled /w44062 on MSVC (switch case not handled warning)
* x86 builds are now supported

## Beta 1.0

* Initial release
