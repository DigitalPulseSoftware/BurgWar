include("sh_scoreboard.lua")
include("sv_rounds.lua")
include("sv_pcts.lua")

local gamemode = ScriptedGamemode()
gamemode.PlayerEntity = "entity_ssb_burger"

gamemode:Disconnect(gamemode.BasePlayerDeathSlot)

gamemode:On("MapInit", function (self)
	local cameraRects = match.GetEntitiesByClass("entity_camera_rect", i)
	if (#cameraRects > 0) then
		local cameraRect = cameraRects[1]
		local creationInfo = cameraRect:DumpCreationInfo()
		creationInfo.Type = "entity_camera_zone"

		match.CreateEntity(creationInfo)
	end
end)

function gamemode:OnPlayerSpawn(player)
	local entity = player:GetControlledEntity()
	entity:GiveWeapon("weapon_sword_emmentalibur")
	entity:GiveWeapon("weapon_graspain")
end
