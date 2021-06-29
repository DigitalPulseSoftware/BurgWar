local gamemode = ScriptedGamemode()
gamemode.ScoreTypes = { "Kill", "Death" }

function gamemode:GetPlayerDeaths(player)
	return gamemode:GetPlayerScore(player, "Death")
end

function gamemode:GetPlayerKills(player)
	return gamemode:GetPlayerScore(player, "Kill")
end

if (SERVER) then
	function gamemode:IncreasePlayerDeath(player, deathCount)
		return gamemode:IncreasePlayerScore(player, "Death", deathCount)
	end

	function gamemode:IncreasePlayerKill(player, killCount)
		return gamemode:IncreasePlayerScore(player, "Kill", killCount)
	end

	function gamemode:UpdatePlayerDeath(player, deathCount)
		return gamemode:UpdatePlayerScore(player, "Death", deathCount)
	end

	function gamemode:UpdatePlayerKill(player, killCount)
		return gamemode:UpdatePlayerScore(player, "Kill", killCount)
	end
end
