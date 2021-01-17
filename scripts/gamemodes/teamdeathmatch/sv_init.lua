local gamemode = ScriptedGamemode()

gamemode.PlayerEntity = "entity_team_burger"

gamemode:AddTeam("Brioche", { r = 255, g = 165, b = 0 })
gamemode:AddTeam("Rustic", { r = 166, g = 166, b = 166 })

function gamemode:GeneratePlayerEntityProperties(player)
	local properties = self.Base:GeneratePlayerEntityProperties(player)
	properties.bread = (self:GetPlayerTeam(player).index == 1) and 1 or 3

	return properties
end

function gamemode:ChoosePlayerTeam()
	local teams = self:GetTeams()

	-- Find the most empty team
	local bestTeam = teams[1]
	for i = 2, #teams do
		local team = teams[i]
		if (team:GetPlayerCount() < bestTeam:GetPlayerCount()) then
			bestTeam = team
		end
	end

	return bestTeam
end

function gamemode:OnPlayerSpawn(player)
	self.Base:OnPlayerSpawn(player)

	local entity = player:GetControlledEntity()
	if (not self:GetProperty("friendlyfire")) then
		entity:On("TakeDamage", function (self, damage, attacker)
			if (not attacker) then
				return
			end

			local owner = self:GetOwner()
			if (not owner) then
				return
			end

			local attackerPlayer = attacker:GetOwner()
			if (not attackerPlayer or attackerPlayer:GetPlayerIndex() == owner:GetPlayerIndex()) then
				return
			end

			local gamemode = match.GetGamemode()
			if (gamemode:GetPlayerTeam(owner) ~= gamemode:GetPlayerTeam(attackerPlayer)) then
				return
			end

			return 0 -- Cancel damage for friendly fire
		end)
	end
end
