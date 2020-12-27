local gamemode = ScriptedGamemode()
gamemode.ScoreboardColumns = { "Name", "Ping" }

gamemode:On("InitScoreboard", function (self, scoreboard)
	self.Scoreboard = scoreboard
	self.ScoreboardColumnIndexes = {}
	self.ScoreboardTeamIndexes = {}

	-- Retrieve active gamemode instead of base gamemode
	local bottomGamemode = match.GetGamemode()

	for _, columnName in pairs(bottomGamemode.ScoreboardColumns) do
		self.ScoreboardColumnIndexes[columnName] = scoreboard:AddColumn(columnName)
	end

	for _, team in pairs(self:GetTeams()) do
		local teamName = team:GetName()
		self.ScoreboardTeamIndexes[teamName] = scoreboard:AddTeam(teamName, team:GetColor())
	end

	for _, player in pairs(match.GetPlayers()) do
		bottomGamemode:RegisterScoreboardPlayer(player)
	end
end)

gamemode:On("PlayerJoined", function (self, player)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	-- Retrieve active gamemode instead of base gamemode
	match.GetGamemode():RegisterScoreboardPlayer(player)
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

	local playerTeam = self:GetPlayerTeam(player)
	scoreboard:RegisterPlayer(playerIndex, playerTeam and self.ScoreboardTeamIndexes[playerTeam:GetName()] or -1, {
		player:GetName(),
		tostring(player:GetPing() or ""),
	}, isLocalPlayer)
end

gamemode:On("PlayerLeave", function (self, player)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	scoreboard:UnregisterPlayer(player:GetPlayerIndex())
end)

gamemode:On("PlayerNameUpdate", function (self, player, newName)
	print(player:GetName() .. " has changed name to " .. newName)

	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	scoreboard:UpdatePlayerValue(player:GetPlayerIndex(), self.ScoreboardColumnIndexes.Name, newName)
end)

gamemode:On("PlayerPingUpdate", function (self, player, newName)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	for _, player in pairs(match.GetPlayers()) do
		scoreboard:UpdatePlayerValue(player:GetPlayerIndex(), self.ScoreboardColumnIndexes.Ping, tostring(player:GetPing() or ""))
	end
end)

gamemode:On("PlayerTeamUpdate", function (self, player, team)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	scoreboard:UpdatePlayerTeam(player:GetPlayerIndex(), team and self.ScoreboardTeamIndexes[team:GetName()] or -1)
end)
