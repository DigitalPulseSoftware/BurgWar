local gamemode = ScriptedGamemode()

gamemode.PlayerEntity = "entity_burger"

gamemode.PlayerSeeds = {}

math.randomseed(os.time())

gamemode.BasePlayerDeathSlot = gamemode:OnAsync("PlayerDeath", function (self, player, attacker)
	print(player:GetName() .. " died")
	timer.Sleep(self:GetProperty("respawntime") * 1000)

	match.GetGamemode():SpawnPlayer(player)
end)

gamemode:On("PlayerJoined", function (self, player)
	self.PlayerSeeds[player:GetPlayerIndex()] = math.random(0, math.maxinteger)
	print(player:GetName() .. " joined")
end)

gamemode.BasePlayerJoinedSlot = gamemode:On("PlayerJoined", function(self, player)
	local team = match.GetGamemode():ChoosePlayerTeam(player)
	if (team) then
		team:AddPlayer(player)
	end

	match.GetGamemode():SpawnPlayer(player)
end)

gamemode:On("PlayerLeave", function (self, player)
	self.PlayerSeeds[player:GetPlayerIndex()] = nil
end)

function gamemode:ChoosePlayerTeam(player)
end

function gamemode:OnPlayerSpawn(player)
	local entity = player:GetControlledEntity()
	entity:GiveWeapon("weapon_graspain")
end

function gamemode:GeneratePlayerEntityProperties(player)
	return {
		seed = self.PlayerSeeds[player:GetPlayerIndex()]
	}
end

function gamemode:SpawnPlayer(player)
	local spawnPosition, layerIndex = self:ChoosePlayerSpawnPosition()
	local properties = self:GeneratePlayerEntityProperties(player)
	local entity = match.CreateEntity({
		Type = self.PlayerEntity,
		LayerIndex = layerIndex,
		Position = spawnPosition,
		Properties = properties
	})
	player:UpdateControlledEntity(entity)

	self:OnPlayerSpawn(player)
end

local noclipController = NoclipPlayerMovementController.new()

local possibleCharacters = "0123456789abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWYZ"
local password = {}
for i = 1, 8 do
	local index = math.random(1, #possibleCharacters)
	table.insert(password, possibleCharacters:sub(index, index))
end
password = table.concat(password)

print("Admin password: " .. password)

gamemode:On("PlayerChat", function (self, player, message)
	if (message:sub(1,1) == "/") then
		local commandName, commandArgs = message:match("/(%w+)%s*(.*)")
		if (not commandName) then
			return false
		end

		if (commandName == "admin") then
			if (commandArgs == password) then
				player:SetAdmin(true)
				player:PrintChatMessage("You are now admin")
			end
		elseif (commandName == "suicide") then
			local controlledEntity = player:GetControlledEntity()
			if (not controlledEntity) then
				return false
			end

			controlledEntity:Kill()
		elseif (commandName == "noclip") then
			if (not player:IsAdmin()) then
				return false
			end

			local controlledEntity = player:GetControlledEntity()
			if (not controlledEntity) then
				return false
			end

			if (controlledEntity.previousController) then
				controlledEntity:UpdatePlayerMovementController(controlledEntity.previousController)
				controlledEntity.previousController = nil
				player:PrintChatMessage("Noclip disabled")
			else
				controlledEntity.previousController = controlledEntity:GetPlayerMovementController()
				controlledEntity:UpdatePlayerMovementController(noclipController)
				player:PrintChatMessage("Noclip enabled")
			end
		elseif (commandName == "teleport") then
			if (not player:IsAdmin()) then
				return false
			end

			local controlledEntity = player:GetControlledEntity()
			if (not controlledEntity) then
				return false
			end

			controlledEntity:SetPosition(Vec2(0, 0))
		end

		return false
	end
end)

function gamemode:ChoosePlayerSpawnPosition()
	local spawnpoints = match.GetEntitiesByClass("entity_spawnpoint")
	local spawnpointEntity = spawnpoints[math.random(1, #spawnpoints)]
	return spawnpointEntity:GetPosition(), spawnpointEntity:GetLayerIndex()
end

gamemode:On("PlayerLayerUpdate", function (self, player, oldLayer, newLayer)
	print("Player " .. player:GetName() .. " change layer (", oldLayer .. " => " ..  newLayer .. ")")
	if (oldLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", oldLayer)) do
			ent:OnPlayerLeaveLayer(player, oldLayer)
		end
	end

	if (newLayer ~= NoLayer) then
		for _, ent in pairs(match.GetEntitiesByClass("entity_visible_layer", newLayer)) do
			ent:OnPlayerEnterLayer(player, newLayer)
		end
	end
end)
