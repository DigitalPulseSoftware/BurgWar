local gamemode = ScriptedGamemode()
gamemode.ScoreboardColumns = { "Name", "Kill", "Death", "Ping" }

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
		tostring(self:GetPlayerKills(player)),
		tostring(self:GetPlayerDeaths(player)),
		tostring(player:GetPing() or ""),
	}, isLocalPlayer)
end

function gamemode:UpdateScoreboard(deaths, kills)
	local scoreboard = self.Scoreboard
	if (not scoreboard) then
		return
	end

	for playerIndex, death in pairs(deaths) do
		scoreboard:UpdatePlayerValue(playerIndex, self.ScoreboardColumnIndexes.Death, tostring(death))
	end

	for playerIndex, kill in pairs(kills) do
		scoreboard:UpdatePlayerValue(playerIndex, self.ScoreboardColumnIndexes.Kill, tostring(kill))
	end
end
