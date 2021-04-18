# Changelog

## master (unreleased)

## General

* Added teams handling in scoreboard and base gamemode
* Added teamdeatchmatch gamemode
* Improved Windows and Linux builds
* Fixed hovering visuals (nickname, health bar) position when the player entity gets scaled up or down
* Added maptool (command-line software which can compile maps from json to binary format)
* The client will now try other fast download urls in case of failure
* Switched from static to dynamic linking for common code to decrease binary size

## Map editor

* **The map editor is now able to show other layers (as they would be seen in the game)**
* Added a button to explicitly rebuild the asset list
* Maps asset lists are now sorted by path
* Fix: the entity list is now properly cleared when closing a map

## Scripting

* Gamemode events are now triggered on the table they used to register the event (this is important when using gamemode inheritance)
* Added "PlayerControlledEntityUpdate" client gamemode event.
* Added Text renderable (which is now using for showing player nicknames)
* Added IncomingNetworkPacket:ReadColor() and OutgoingNetworkPacket:WriteColor()
* Added match.GetPlayerByIndex()
* Added Scoreboard:UpdatePlayerTeam()
* Added base gamemode event 
* Added match.BroadcastChatMessage(message)
* match.BroadcastPacket now has a second optional parameter to disable ready check
* Elements and gamemode events can now be disconnected using the Disconnect method
* Player entities are now detected using a `IsPlayerEntity` boolean on their table
* Fixed gamemode overriding of the `Musics` table

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
