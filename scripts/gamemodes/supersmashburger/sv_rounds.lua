local gamemode = ScriptedGamemode()

gamemode.State = "waiting"

gamemode:Disconnect(gamemode.BasePlayerJoinedSlot)

gamemode:OnAsync("PlayerJoined", function (self, player)
	player:MoveToLayer(0)

	local playerCount = #match.GetPlayers()
	local requiredPlayerCount = self:GetProperty("minplayercount")

	if (playerCount >= requiredPlayerCount) then
		if (self.State == "waiting") then
			self:PrepareNextRound()
		end
	else
		match.BroadcastChatMessage(string.format("%d/%d players", playerCount, requiredPlayerCount))
	end
end)

function gamemode:StartRound()
	match.BroadcastChatMessage("Fight!")

	self.State = "playing"

	for _, player in pairs(match.GetPlayers()) do
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

	if (not potentialWinningPlayer) then
		return
	end

	match.BroadcastChatMessage(potentialWinningPlayer:GetName() .. " won!")
	self:PrepareNextRound()
end)
