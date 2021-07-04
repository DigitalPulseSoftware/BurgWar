
local gamemode = ScriptedGamemode()

gamemode.PlayerPcts = {}

local massLossPerPct = 0.2
local startingMass = 50

function gamemode:OnPlayerSpawn(player)
	self.Base:OnPlayerSpawn(player)

	local entity = player:GetControlledEntity()
	entity:GiveWeapon("weapon_sword_emmentalibur")
	entity:GiveWeapon("weapon_graspain")

	local id = player:GetPlayerIndex()
	self.PlayerPcts[id] = 0

	local entity = player:GetControlledEntity()
	entity:SetMass(startingMass)
	entity:On("TakeDamage", function (entity, damage, attacker)
		if (not attacker or attacker == entity) then
			return
		end

		local owner = entity:GetOwner()
		if (not owner) then
			return
		end

		self:UpdatePct(id, damage // 5)
		return 0 -- Prevent players from hurting each other
	end)
end

function gamemode:UpdatePct(playerId, pct)
	local newPct = self.PlayerPcts[playerId] + pct
	self.PlayerPcts[playerId] = newPct

	local player = assert(match.GetPlayerByIndex(playerId))
	local entity = player:GetControlledEntity()
	if entity then
		local newMass = math.max(math.floor(startingMass - newPct * massLossPerPct), 5)
		entity:SetMass(newMass)
	end

	local packet = network.NewPacket("PctUpdate")
	packet:WriteCompressedUnsigned(playerId)
	packet:WriteCompressedUnsigned(self.PlayerPcts[playerId])

	match.BroadcastPacket(packet)
end

network.RegisterPacket("PctUpdate")
