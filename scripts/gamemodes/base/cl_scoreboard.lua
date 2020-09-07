local gamemode = ScriptedGamemode()
gamemode.ScoreboardColumns = { "Name", "Ping" }

gamemode:On("initscoreboard", function (self, scoreboard)
	self.Scoreboard = scoreboard
	self.ScoreboardColumnIndexes = {}

	for _, columnName in pairs(self.ScoreboardColumns) do
		self.ScoreboardColumnIndexes[columnName] = scoreboard:AddColumn(columnName)
	end

	for _, player in pairs(match.GetPlayers()) do
		self:RegisterScoreboardPlayer(player)
	end
end)

gamemode:On("playerjoined", function (self, player)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	self:RegisterScoreboardPlayer(player)
end)

function gamemode:RegisterScoreboardPlayer(player)
	local scoreboard = assert(self.Scoreboard)

	local playerIndex = player:GetPlayerIndex()
	local isLocalPlayer = false
	local localPlayerCount = engine_GetLocalPlayerCount()
	for i = 1, localPlayerCount do
		if (engine_GetLocalPlayer_PlayerIndex(i - 1) == playerIndex) then
			isLocalPlayer = true
			break
		end
	end

	scoreboard:RegisterPlayer(playerIndex, 0, {
		player:GetName(),
		tostring(player:GetPing() or ""),
	}, isLocalPlayer)
end

gamemode:On("playerleave", function (self, player)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	scoreboard:UnregisterPlayer(player:GetPlayerIndex())
end)

gamemode:On("playernameupdate", function (self, player, newName)
	print(player:GetName() .. " has changed name to " .. newName)

	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	scoreboard:UpdatePlayerValue(player:GetPlayerIndex(), self.ScoreboardColumnIndexes.Name, newName)
end)

gamemode:On("playerpingupdate", function (self, player, newName)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	for _, player in pairs(match.GetPlayers()) do
		scoreboard:UpdatePlayerValue(player:GetPlayerIndex(), self.ScoreboardColumnIndexes.Ping, tostring(player:GetPing() or ""))
	end
end)
