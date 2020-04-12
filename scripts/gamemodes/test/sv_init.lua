GM.PlayerEntity = "entity_burger"

GM.PlayerSeeds = {}

math.randomseed(os.time())

function GM:OnPlayerDeath(player, attacker)
	self:IncreasePlayerDeath(player)
	if (attacker) then
		local attackerPlayer = attacker:GetOwner()
		if (attackerPlayer and attackerPlayer:GetPlayerIndex() ~= player:GetPlayerIndex()) then
			self:IncreasePlayerKill(attackerPlayer)
		end
	end

	print(player:GetName() .. " died")
	timer.Sleep(2000)
	self:SpawnPlayer(player)
end

GM.OnPlayerJoin = utils.OverrideFunction(GM.OnPlayerJoin, function (self, player)
	self.PlayerSeeds[player:GetPlayerIndex()] = math.random(0, math.maxinteger)

	print(player:GetName() .. " joined")

	self:SpawnPlayer(player)
end)

GM.OnPlayerLeave = utils.OverrideFunction(GM.OnPlayerLeave, function (self, player)
	self.PlayerSeeds[player:GetPlayerIndex()] = nil
end)

function GM:OnPlayerSpawn(player)
	player:GiveWeapon("weapon_sword_emmentalibur")
	player:GiveWeapon("weapon_patator")
	player:GiveWeapon("weapon_graspain")
end

function GM:GeneratePlayerEntityProperties(player)
	return {
		seed = self.PlayerSeeds[player:GetPlayerIndex()]
	}
end

function GM:SpawnPlayer(player)
	local spawnPosition, layerIndex = self:ChoosePlayerSpawnPosition(player)
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

function GM:OnPlayerChat(player, message)
	if (message:sub(1,1) == "/") then
		local commandName, commandArgs = message:match("/(%w+)%s*(.*)")
		if (not commandName) then
			return
		end

		if (commandName == "admin") then
			if (commandArgs == password) then
				player:SetAdmin(true)
				player:PrintChatMessage("You are now admin")
			end
		elseif (commandName == "suicide") then
			local controlledEntity = player:GetControlledEntity()
			if (not controlledEntity) then
				return
			end

			controlledEntity:Kill()
		elseif (commandName == "noclip") then
			if (not player:IsAdmin()) then
				return
			end

			local controlledEntity = player:GetControlledEntity()
			if (not controlledEntity) then
				return
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
		end
	else
		return message
	end
end

GM.OnTick = utils.OverrideFunction(GM.OnTick, function (self)
	for _, burger in pairs(match.GetEntitiesByClass("entity_burger")) do
		local pos = burger:GetPosition()
		if (pos.y > 10000) then
			burger:Kill()
		end
	end
end)

function GM:OnInit()
	--[[for i = 0, 20 do
		self:CreateEntity("entity_box", Vec2(math.random(0, 10000), -200), {
			size = math.random(10, 20) / 10.0
		})
	end]]
end

function GM:ChoosePlayerSpawnPosition()
	local spawnpoints = match.GetEntitiesByClass("entity_spawnpoint")
	local spawnpointEntity = spawnpoints[math.random(1, #spawnpoints)]
	return spawnpointEntity:GetPosition(), spawnpointEntity:GetLayerIndex()
end

function GM:OnPlayerChangeLayer(player, newLayer)
	-- FIXME: This shouldn't be handled by this callback

	local oldLayer = player:GetLayerIndex()
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
end
