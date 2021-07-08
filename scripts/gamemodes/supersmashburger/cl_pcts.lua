
local gamemode = ScriptedGamemode()

local function hsvToRgb(h, s, v)
	local r, g, b

	local i = math.floor(h * 6)
	local f = h * 6 - i
	local p = v * (1 - s)
	local q = v * (1 - f * s)
	local t = v * (1 - (1 - f) * s)

	i = i % 6

	if i == 0 then r, g, b = v, t, p
	elseif i == 1 then r, g, b = q, v, p
	elseif i == 2 then r, g, b = p, v, t
	elseif i == 3 then r, g, b = p, q, v
	elseif i == 4 then r, g, b = t, p, v
	elseif i == 5 then r, g, b = v, p, q
	end

	return r * 255, g * 255, b * 255
end

local goldenRatio = 0.618033988749895
local initialValue = 0.73177468828361

local function GetPlayerColor(playerId)
	local r,g,b = hsvToRgb((initialValue + playerId * goldenRatio) % 1, 0.5, 0.99)
	return {r = math.floor(r), g = math.floor(g), b = math.floor(b)}
end

function gamemode:GetPlayerColor(player)
	return GetPlayerColor(player:GetPlayerIndex())
end

gamemode.PlayerPcts = {}
gamemode.PlayerPctTexts = {}

gamemode:On("PlayerControlledEntityUpdate", function (self, player, oldEntity, newEntity)
	local id = player:GetPlayerIndex()

	local text = self.PlayerPctTexts[id]
	if (text and text:IsValid()) then
		text:Hide()
	end

	if (newEntity) then
		local pct = self.PlayerPcts[id] or 0

		local text = newEntity:AddText({
			Color = self:GetPlayerColor(player),
			Hovering = true,
			OutlineThickness = 2,
			RenderOrder = 10,
			Text = pct .. "%"
		})
		text:SetHoveringHeight(50)

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))

		self.PlayerPctTexts[id] = text
	else
		self.PlayerPctTexts[id] = nil
	end
end)

gamemode:On("PlayerLeave", function (self, player)
	local id = player:GetPlayerIndex()

	local text = self.PlayerPctTexts[id]
	if (text and text:IsValid()) then
		text:Hide()
	end

	self.PlayerPctTexts[id] = nil
	self.PlayerPcts[id] = nil
end)

function gamemode:UpdatePlayerPct(playerId, pct)
	gamemode.PlayerPcts[playerId] = pct
	local text = gamemode.PlayerPctTexts[playerId]
	if (text and text:IsValid()) then
		text:SetText(pct .. "%")

		local size = text:GetSize()
		text:SetOffset(Vec2(-size.x / 2, -size.y))
	end
end

gamemode:On("RoundStateUpdate", function (self, oldState, newState)
	if (newState == RoundState.Finished) then
		for _, player in pairs(match.GetPlayers()) do
			local id = player:GetPlayerIndex()
			gamemode:UpdatePlayerPct(id, 0)
		end
	end
end)

network.SetHandler("PctUpdate", function (packet)
	local id = packet:ReadCompressedUnsigned()
	local pct = packet:ReadCompressedUnsigned()

	gamemode:UpdatePlayerPct(id, pct)
end)
