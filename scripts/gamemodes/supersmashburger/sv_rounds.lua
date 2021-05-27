local gamemode = ScriptedGamemode()

gamemode.PlayerRoundCount = {}
gamemode.State = "waiting"

gamemode:Disconnect(gamemode.BasePlayerJoinedSlot)

gamemode:OnAsync("PlayerJoined", function (self, player)
	player:MoveToLayer(0)

	local playerIndex = player:GetPlayerIndex()
	self.PlayerRoundCount[playerIndex] = 0

	if (self.State == "waiting") then
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
	end

	match.BroadcastChatMessage("Active players: " .. table.concat(activePlayerNames, ", "))
	match.BroadcastChatMessage("Fight!")

	self.State = "playing"

	for _, player in pairs(activePlayers) do
		self:SpawnPlayer(player)
	end
end

function gamemode:PrepareNextRound()
	self.State = "starting"
	match.BroadcastChatMessage("Starting round in 5s")
	timer.Sleep(5000)

	-- Recheck player count
	local playerCount = #match.GetPlayers()
	local requiredPlayerCount = self:GetProperty("minplayercount")

	if (playerCount >= requiredPlayerCount) then
		self:StartRound()
	else
		match.BroadcastChatMessage("Not enough players, aborted.")
		self.State = "waiting"
	end
end

gamemode:OnAsync("Tick", function (self)
	if (self.State ~= "playing") then
		return
	end

	local potentialWinningPlayer
	for _, player in pairs(match.GetPlayers()) do
		local entity = player:GetControlledEntity()
		if (entity and entity:IsValid()) then
			if (potentialWinningPlayer) then
				-- Another player is still there, dismiss
				return
			end

			potentialWinningPlayer = player
		end
	end

	if (potentialWinningPlayer) then
		self.State = "won"

		match.BroadcastChatMessage(potentialWinningPlayer:GetName() .. " won!")
		timer.Sleep(5000)
		match.ResetTerrain()
		self:PrepareNextRound()
	else
		-- All players disconnected, reset game
		self.State = "waiting"
		match.ResetTerrain()
	end
end)
