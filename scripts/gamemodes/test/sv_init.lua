function GM:OnPlayerDeath(player, attacker)
	print(player:GetName() .. " died")
	timer.Sleep(2000)
	player:Spawn()
end

function GM:OnPlayerJoin(player)
	print(player:GetName() .. " joined")
	player:UpdateLayer(0)
	player:Spawn()
	player:GiveWeapon("weapon_sword_emmentalibur")
	player:GiveWeapon("weapon_grenade")
end

function GM:OnTick()
	for _, burger in pairs(match.GetEntitiesByClass("entity_burger")) do
		local pos = burger:GetPosition()
		if (pos.y > 2000) then
			burger:Kill()
		end
	end
end

function GM:OnInit()
	--[[for i = 0, 20 do
		self:CreateEntity("entity_box", Vec2(math.random(0, 10000), -200), {
			size = math.random(10, 20) / 10.0
		})
	end]]
end

function GM:ChoosePlayerSpawnPosition()
	local spawnpoints = match.GetEntitiesByClass("entity_spawnpoint")
	local spawnpointIndex = math.random(1, #spawnpoints)

	return spawnpoints[spawnpointIndex]:GetPosition()
end

function GM:OnPlayerChangeLayer(player, newLayer)
	-- FIXME: This shouldn't be handled by this callback

	local oldLayer = player:GetLayerIndex()
	if (oldLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", oldLayer)) do
			ent:OnPlayerLeaveLayer(player)
		end
	end

	if (newLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", newLayer)) do
			ent:OnPlayerEnterLayer(player)
		end
	end
end
