GM.ScoreDeaths = {}
GM.ScoreKills = {}

function GM:GetPlayerDeaths(player)
	return self.ScoreDeaths[player:GetPlayerIndex()] or 0
end

function GM:GetPlayerKills(player)
	return self.ScoreKills[player:GetPlayerIndex()] or 0
end

GM.OnPlayerLeave = utils.OverrideFunction(GM.OnPlayerLeave, function (self, player)
	local playerIndex = player:GetPlayerIndex()
	self.ScoreDeaths[playerIndex] = nil
	self.ScoreKills[playerIndex] = nil
end)

if (SERVER) then
	local function BuildScorePacket()
		local packet = network.NewPacket("ScoreUpdate")

		packet:WriteCompressedUnsigned(table.count(GM.ScoreDeaths))
		packet:WriteCompressedUnsigned(table.count(GM.ScoreKills))

		for playerIndex, death in pairs(GM.ScoreDeaths) do
			packet:WriteCompressedUnsigned(playerIndex)
			packet:WriteCompressedUnsigned(death)
		end

		for playerIndex, kill in pairs(GM.ScoreKills) do
			packet:WriteCompressedUnsigned(playerIndex)
			packet:WriteCompressedUnsigned(kill)
		end

		return packet
	end

	GM.ScoreUpdated = false

	GM.OnPlayerJoin = utils.OverrideFunction(GM.OnPlayerJoin, function (self, player)
		player:SendPacket(BuildScorePacket())
	end)

	function GM:IncreasePlayerDeath(player, deathCount)
		local playerIndex = player:GetPlayerIndex()
		self.ScoreDeaths[playerIndex] = (self.ScoreDeaths[playerIndex] or 0) + (deathCount or 1)
		self.ScoreUpdated = false
	end

	function GM:IncreasePlayerKill(player, killCount)
		local playerIndex = player:GetPlayerIndex()
		self.ScoreKills[playerIndex] = (self.ScoreKills[playerIndex] or 0) + (killCount or 1)
		self.ScoreUpdated = false
	end

	function GM:UpdatePlayerDeath(player, deathCount)
		self.ScoreDeaths[player:GetPlayerIndex()] = deathCount
		self.ScoreUpdated = false
	end

	function GM:UpdatePlayerKill(player, killCount)
		self.ScoreKills[player:GetPlayerIndex()] = killCount
		self.ScoreUpdated = false
	end

	GM.OnTick = utils.OverrideFunction(GM.OnTick, function (self)
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
			GM.ScoreDeaths[playerIndex] = death
		end

		for i = 1, killCount do
			local playerIndex = packet:ReadCompressedUnsigned()
			local kill = packet:ReadCompressedUnsigned()

			kills[playerIndex] = kill
			GM.ScoreKills[playerIndex] = kill
		end

		GM:UpdateScoreboard(deaths, kills)
	end)
end
