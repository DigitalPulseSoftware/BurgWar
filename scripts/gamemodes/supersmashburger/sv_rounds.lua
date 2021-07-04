include("sh_rounds.lua")

local stuckInputController = CustomInputController.new(function (entity)
	local inputs = entity:GetOwner():GetInputs()

	inputs.isAttacking = false
	inputs.isMovingLeft = false
	inputs.isMovingRight = false

	return inputs
end)

local gamemode = ScriptedGamemode()

gamemode.PlayerRoundCount = {}
gamemode.State = RoundState.Waiting

gamemode:Disconnect(gamemode.BasePlayerJoinedSlot)

gamemode:OnAsync("PlayerJoined", function (self, player)
	player:MoveToLayer(0)

	local playerIndex = player:GetPlayerIndex()

	self.PlayerRoundCount[playerIndex] = 0

	if (self.State == RoundState.Waiting) then
		local playerCount = #match.GetPlayers()
		local requiredPlayerCount = self:GetProperty("minplayercount")

		if (playerCount >= requiredPlayerCount) then
			self:PrepareNextRound()
		else
			match.BroadcastChatMessage(string.format("%d/%d players", playerCount, requiredPlayerCount))
		end
	end
end)

gamemode:On("PlayerLeave", function (self, player)
	local playerIndex = player:GetPlayerIndex()
	print("PlayerLeave - " .. playerIndex .. " - " .. player:GetName())
	self.PlayerRoundCount[playerIndex] = nil
end)

function gamemode:StartRound()
	local maxPlayerPerRound = self:GetProperty("maxplayercount")
	local players = match.GetPlayers()
	table.sort(players, function (playerA, playerB)
		return (self.PlayerRoundCount[playerA:GetPlayerIndex()] or 0) < (self.PlayerRoundCount[playerB:GetPlayerIndex()] or 0)
	end)

	local activePlayers = {}
	local activePlayerNames = {}
	for i = 1, maxPlayerPerRound do
		if (i > #players) then
			break
		end

		local player = players[i]
		table.insert(activePlayers, player)
		table.insert(activePlayerNames, player:GetName())

		self.PlayerRoundCount[player:GetPlayerIndex()] = self.PlayerRoundCount[player:GetPlayerIndex()] + 1
		self:IncreasePlayerScore(player, "Played")
	end

	match.BroadcastChatMessage("Active players: " .. table.concat(activePlayerNames, ", "))

	self:UpdateState(RoundState.Countdown)

	for _, player in pairs(activePlayers) do
		self:SpawnPlayer(player)

		local entity = player:GetControlledEntity()
		if (entity) then
			entity.previousController = entity:GetInputController()
			entity:UpdateInputController(stuckInputController)
		end
	end

	timer.Sleep(4000)

	self:UpdateState(RoundState.Playing)
	for _, player in pairs(match.GetPlayers()) do
		local entity = player:GetControlledEntity()
		if (entity and entity.previousController) then
			entity:UpdateInputController(entity.previousController)
			entity.previousController = nil
		end
	end

	match.BroadcastChatMessage("Fight!")
end

function gamemode:PrepareNextRound()
	self:UpdateState(RoundState.Starting)
	match.BroadcastChatMessage("Starting round in 5s")
	timer.Sleep(5000)

	-- Recheck player count
	local playerCount = #match.GetPlayers()
	local requiredPlayerCount = self:GetProperty("minplayercount")

	print("Checking player count (again) - " .. playerCount .. "/" .. requiredPlayerCount)

	if (playerCount >= requiredPlayerCount) then
		self:StartRound()
	else
		match.BroadcastChatMessage("Not enough players, aborted.")
		self:UpdateState(RoundState.Waiting)
	end
end

gamemode:OnAsync("Tick", function (self)
	if (self.State ~= RoundState.Playing) then
		return
	end
	
	local hasPlayer = false
	local potentialWinningPlayer
	for _, player in pairs(match.GetPlayers()) do
		hasPlayer = true

		local entity = player:GetControlledEntity()
		if (entity and entity:IsValid()) then
			if (potentialWinningPlayer) then
				-- Another player is still there, dismiss
				return
			end

			potentialWinningPlayer = player
		end
	end

	if (potentialWinningPlayer or hasPlayer) then
		self:UpdateState(RoundState.Finished)

		if (potentialWinningPlayer) then
			match.BroadcastChatMessage(potentialWinningPlayer:GetName() .. " won!")
			self:IncreasePlayerScore(potentialWinningPlayer, "Wins")
		else
			match.BroadcastChatMessage("Draw!")
		end

		timer.Sleep(5000)
		match.ResetTerrain()
		self:PrepareNextRound()
	else
		-- All players disconnected, reset game
		self:UpdateState(RoundState.Waiting)
		match.ResetTerrain()
	end
end)
