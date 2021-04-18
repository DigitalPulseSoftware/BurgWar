include("sv_rounds.lua")
include("sv_pcts.lua")

local gamemode = ScriptedGamemode()
gamemode.PlayerEntity = "entity_ssb_burger"

gamemode:Disconnect(gamemode.BasePlayerDeathSlot)

gamemode:On("Init", function (self)
	for i = 0, match.GetLayerCount() - 1 do
		local cameraRects = match.GetEntitiesByClass("entity_camera_rect", i)
		if (#cameraRects > 0) then
			local cameraRect = cameraRects[1]
			local creationInfo = cameraRect:DumpCreationInfo()
			creationInfo.Type = "entity_camera_zone"

			match.CreateEntity(creationInfo)
		end
	end
end)

gamemode:On("PlayerDeath", function (self, player, attacker)
	self:IncreasePlayerDeath(player)
	if (attacker) then
		local attackerPlayer = attacker:GetOwner()
		if (attackerPlayer and attackerPlayer:GetPlayerIndex() ~= player:GetPlayerIndex()) then
			self:IncreasePlayerKill(attackerPlayer)
		end
	end
end)
