local gamemode = ScriptedGamemode()

gamemode.ScoreDeaths = {}
gamemode.ScoreKills = {}

function gamemode:GetPlayerDeaths(player)
	return self.ScoreDeaths[player:GetPlayerIndex()] or 0
end

function gamemode:GetPlayerKills(player)
	return self.ScoreKills[player:GetPlayerIndex()] or 0
end

gamemode:On("playerleave", function (self, player)
	local playerIndex = player:GetPlayerIndex()
	self.ScoreDeaths[playerIndex] = nil
	self.ScoreKills[playerIndex] = nil
end)

if (SERVER) then
	local function BuildScorePacket()
		local packet = network.NewPacket("ScoreUpdate")

		packet:WriteCompressedUnsigned(table.count(gamemode.ScoreDeaths))
		packet:WriteCompressedUnsigned(table.count(gamemode.ScoreKills))

		for playerIndex, death in pairs(gamemode.ScoreDeaths) do
			packet:WriteCompressedUnsigned(playerIndex)
			packet:WriteCompressedUnsigned(death)
		end

		for playerIndex, kill in pairs(gamemode.ScoreKills) do
			packet:WriteCompressedUnsigned(playerIndex)
			packet:WriteCompressedUnsigned(kill)
		end

		return packet
	end

	gamemode.ScoreUpdated = false

	gamemode:On("playerjoined", function (self, player)
		player:SendPacket(BuildScorePacket())
	end)

	function gamemode:IncreasePlayerDeath(player, deathCount)
		local playerIndex = player:GetPlayerIndex()
		self.ScoreDeaths[playerIndex] = (self.ScoreDeaths[playerIndex] or 0) + (deathCount or 1)
		self.ScoreUpdated = false
	end

	function gamemode:IncreasePlayerKill(player, killCount)
		local playerIndex = player:GetPlayerIndex()
		self.ScoreKills[playerIndex] = (self.ScoreKills[playerIndex] or 0) + (killCount or 1)
		self.ScoreUpdated = false
	end

	function gamemode:UpdatePlayerDeath(player, deathCount)
		self.ScoreDeaths[player:GetPlayerIndex()] = deathCount
		self.ScoreUpdated = false
	end

	function gamemode:UpdatePlayerKill(player, killCount)
		self.ScoreKills[player:GetPlayerIndex()] = killCount
		self.ScoreUpdated = false
	end

	gamemode:On("tick", function (self)
		if (not self.ScoreUpdated) then
			-- Send score to players
			match.BroadcastPacket(BuildScorePacket())

			self.ScoreUpdated = true
		end
	end)

	network.RegisterPacket("ScoreUpdate")
else
	network.SetHandler("ScoreUpdate", function (packet)	
		local deaths = {}
		local kills = {}

		local deathCount = packet:ReadCompressedUnsigned()
		local killCount = packet:ReadCompressedUnsigned()

		for i = 1, deathCount do
			local playerIndex = packet:ReadCompressedUnsigned()
			local death = packet:ReadCompressedUnsigned()

			deaths[playerIndex] = death
			gamemode.ScoreDeaths[playerIndex] = death
		end

		for i = 1, killCount do
			local playerIndex = packet:ReadCompressedUnsigned()
			local kill = packet:ReadCompressedUnsigned()

			kills[playerIndex] = kill
			gamemode.ScoreKills[playerIndex] = kill
		end

		gamemode:UpdateScoreboard(deaths, kills)
	end)
end
