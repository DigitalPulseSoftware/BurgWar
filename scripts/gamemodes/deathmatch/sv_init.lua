local gamemode = ScriptedGamemode()

gamemode:On("PlayerDeath", function (self, player, attacker)
	self:IncreasePlayerDeath(player)
	if (attacker) then
		local attackerPlayer = attacker:GetOwner()
		if (attackerPlayer and attackerPlayer:GetPlayerIndex() ~= player:GetPlayerIndex()) then
			self:IncreasePlayerKill(attackerPlayer)
		end
	end
end)

function gamemode:OnPlayerSpawn(player)
	local entity = player:GetControlledEntity()
	entity:GiveWeapon("weapon_sword_emmentalibur")
	entity:GiveWeapon("weapon_graspain")

	local spawnImmunity = self:GetProperty("respawnimmunity")
	if (spawnImmunity > 0) then
		local conn = entity:On("TakeDamage", function (self, damage, attacker)
			if (attacker and attacker:GetOwner()) then
				return 0 -- Cancel damage
			end
		end)

		timer.Create(math.floor(spawnImmunity * 1000), function()
			if (entity:IsValid()) then
				entity:Disconnect(conn)
			end
		end)
	end
end
