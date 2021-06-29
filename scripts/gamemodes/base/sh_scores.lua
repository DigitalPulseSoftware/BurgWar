local gamemode = ScriptedGamemode()

gamemode.Scores = {}
gamemode.ScoreTypes = {}

function gamemode:GetPlayerScore(player, scoreType)
	local playerScores = self.Scores[player:GetPlayerIndex()]
	if not playerScores then
		return nil
	end

	return playerScores[scoreType]
end

gamemode:On("PlayerJoined", function (self, player)
	local playerIndex = player:GetPlayerIndex()
	self.Scores[playerIndex] = {}
end)

gamemode:On("PlayerLeave", function (self, player)
	local playerIndex = player:GetPlayerIndex()
	self.Scores[playerIndex] = nil
end)

if (SERVER) then
	local function BuildScorePacket()
		local packet = network.NewPacket("ScoreUpdate")

		local derivedGamemode = match.GetGamemode()
		packet:WriteCompressedUnsigned(table.count(derivedGamemode.Scores))

		for playerIndex, scores in pairs(derivedGamemode.Scores) do
			packet:WriteCompressedUnsigned(playerIndex)
			for _, scoreType in pairs(derivedGamemode.ScoreTypes) do
				packet:WriteCompressedUnsigned(scores[scoreType] or 0)
			end
		end

		return packet
	end

	gamemode.ScoreUpdated = false

	function gamemode:IncreasePlayerScore(player, scoreType, increment)
		local playerIndex = player:GetPlayerIndex()
		local playerScores = assert(self.Scores[playerIndex])
		playerScores[scoreType] = (playerScores[scoreType] or 0) + (increment or 1)

		local derivedGamemode = match.GetGamemode()
		derivedGamemode.ScoreUpdated = false
	end

	function gamemode:UpdatePlayerScore(player, scoreType, value)
		local playerIndex = player:GetPlayerIndex()
		local playerScores = assert(self.Scores[playerIndex])
		playerScores[scoreType] = value

		local derivedGamemode = match.GetGamemode()
		derivedGamemode.ScoreUpdated = false
	end

	gamemode:On("PlayerJoined", function (self, player)
		local derivedGamemode = match.GetGamemode()
		if #derivedGamemode.ScoreTypes > 0 then
			player:SendPacket(BuildScorePacket())
		end
	end)

	gamemode:On("Tick", function (self)
		local derivedGamemode = match.GetGamemode()
		if #derivedGamemode.ScoreTypes == 0 then
			return
		end

		if (not derivedGamemode.ScoreUpdated) then
			-- Send score to players
			match.BroadcastPacket(BuildScorePacket())

			derivedGamemode.ScoreUpdated = true
		end
	end)

	network.RegisterPacket("ScoreUpdate")
else
	network.SetHandler("ScoreUpdate", function (packet)	
		local derivedGamemode = match.GetGamemode()

		local scores = {}

		local playerCount = packet:ReadCompressedUnsigned()
		for i=1, playerCount do
			local playerIndex = packet:ReadCompressedUnsigned()

			local playerScores = {}
			for _, scoreType in pairs(derivedGamemode.ScoreTypes) do
				playerScores[scoreType] = packet:ReadCompressedUnsigned()
			end

			scores[playerIndex] = playerScores
		end

		gamemode:UpdateScoreboard(scores)
	end)
end
