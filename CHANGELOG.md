# Changelog

## Unreleased

### **Highlights**

* Player heal/damage values are now shown as floating text

### General

* Ctrl+C/SIGINT/SIGTERM signals are now handled and the server will close properly
* Added ServerSettings.SleepWhenEmpty config for servers (disable ticks if no player is on the server)
* Log time is now more precise and will always reflect elapsed time (before that change, log time have been the same during each update loop)
* Decreased server usage (it will now sleep according to its tick rate)
* Config file loading is now much more verbose (especially with debug logs) and will print all errors/warnings instead of the first one, it will also print ignored options
* Added weapon categories which can be used to quickly change weapon

### Fixes
* Fixed in-game console staying open after exiting a match

## Beta 2.1

### General

* Gravity and jump power are now 50% stronger
* Handled servers registering using IPv6, they will now register a second time using an IPv4-only route which will allows master server to know both addresses (which will then be used by the game)
* Increased max distance of weapon_graspain
* Added icons to game and map editor executables on Windows

### Fixes

* Improved visibility check of entity_tilemap_fade
* Fixed default serverconfig.lua

### Scripting

* Added match.Quit() function (server-only)

## Beta 2.0

### **Highlights**

* The map editor is now able to show other layers (as they would be seen in the game)
* Added bundled map scripts
* Added initial mods support
* Added server listing (with server name, description and more!)
* Added supersmashburger gamemode
* Added teamdeatchmatch gamemode
* Added teams handling
* Added new ssb_island map (with scripted events)
* Added /resetmap admin command
* The grapple will now break with too much force applied

### General

* x86 arch is no longer supported
* Official fast-download and master server URLs have been added to the default configuration files
* Improved Windows and Linux builds
* Fixed hovering visuals (nickname, health bar) position when the player entity gets scaled up or down
* Added maptool (command-line software which can compile maps from json to binary format)
* The client will now try other fast download urls in case of failure
* Nicknames are now limited to 32 characters
* Switched from static to dynamic linking for common code to decrease binary size
* Updated beta_map
* entity_powerup and entity_powerup2 have been replaced by entity_spawner
* `GameSettings` configuration table has been renamed to `ServerSettings`
* Server config `GameSettings.MapFile` has been replaced by `ServerSettings.MapPath` (which can point to a non-compiled map folder as well as a compiled .bmap file)
* Added `ServerSettings.Name`, `ServerSettings.Description`, `ServerSettings.Port` and `ServerSettings.MaxPlayerCount` configs

### Map editor

* Added a button to explicitly rebuild the asset list
* Maps asset lists are now sorted by path
* Fix: the entity list is now properly cleared when closing a map
* Fixed entity selection when creating a new one (previous selection is now discarded)
* When reloading scripts, all entities are now recreated (in case their init callback changed).

### Scripting

* Added InputController (a script can now override the way entities inputs are generated)
* Gamemode events are now triggered on the table they used to register the event (this is important when using gamemode inheritance)
* Added `PlayerControlledEntityUpdate` client gamemode event.
* Added Text renderable (which is now used for showing player nicknames)
* Added IncomingNetworkPacket:ReadBoolean() and OutgoingNetworkPacket:WriteBoolean()
* Added IncomingNetworkPacket:ReadColor() and OutgoingNetworkPacket:WriteColor()
* Added Element:GetGlobalBounds() and Element:GetLocalBounds() (client-side only for now)
* Base gamemode has now BasePlayerDeathSlot and BasePlayerJoinedSlot to allow derived gamemode to disconnect them
* The Destroyed event is no longer called client-side when a layer is disabled
* Added Constraint:GetLastForce and Constraint:GetLastImpulse
* Added Element:GetOwner() on client
* Added Element:GetInputController()
* Added Element:UpdateInputController()
* Added Player:IsLocalPlayer() on client
* Added Player:GetInputs (for every player on server but only local players on client)
* Added Rect:ExtendToPosition(pos)
* Added Rect:Scale(scale)
* Added match.GetEntityByUniqueId(id)
* Added match.GetLayerCount()
* Added match.GetPlayerByIndex(id)
* Added match.ResetTerrain()
* Added math.approach(current, target, incr)
* Added table.join(...)
* Added Scoreboard:UpdatePlayerTeam()
* Added base gamemode event `PlayerTeamUpdate`
* Added base gamemode callback `GetPlayerColor`
* Added match.BroadcastChatMessage(message)
* Element:AddSprite now accepts a `repeatTexture` boolean
* Scoreboard:RegisterPlayer now takes a table parameter
* match.BroadcastPacket now has a second optional parameter to disable ready check
* match.GetPlayers now longer returns connecting players
* Replaced Entity:Kill() by Element:Kill()
* Elements and gamemode events can now be disconnected using the Disconnect method
* Player entities are now detected using a `IsPlayerEntity` boolean on their table
* Fixed gamemode overriding of the `Musics` table
* Fixed entity_burger_ai only targeting entity_burger (will now target any entity having .IsPlayerEntity = true)
* Fixed weapon_graspain grapple force (is now based on entity mass)
* Fixed static entities movement not being sent to clients
* Fixed a lot of small issues

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
