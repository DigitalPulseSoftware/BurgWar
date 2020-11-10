RegisterClientScript()

local defaultTextureCell = Vec2(1, 1)

local function GenerateTiles(textures, textureCells)
	local tiles = {}
	for i = 1, #textures do
		local texture = textures[i]
		local textureSize = assets.GetTexture(texture):GetSize()
		local onePixelOffset = 1 / textureSize
		local cellCount = textureCells[i] or defaultTextureCell
		for y = 1, cellCount.y do
			for x = 1, cellCount.x do
				table.insert(tiles, {
					materialPath = textures[i],
					texCoords = Rect(Vec2((x - 1) / cellCount.x, (y - 1) / cellCount.y) + onePixelOffset, Vec2(x / cellCount.x, y / cellCount.y) - onePixelOffset)
				})
			end
		end
	end

	return tiles
end

local function GenerateMaterialData(textures, textureCells)
	local materialData = {}
	for i = 1, #textures do
		local cellCount = textureCells[i] or defaultTextureCell

		local group
		if (cellCount.x > 1 or cellCount.y > 1) then
			group = textures[i]
		else
			group = "single"
		end

		table.insert(materialData, {
			group = group,
			path = textures[i],
			tileCount = cellCount
		})
	end

	return materialData
end

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "mapSize", Type = PropertyType.IntegerSize, Default = Vec2(1, 1), Shared = true },
		{ Name = "cellSize", Type = PropertyType.FloatSize, Default = Vec2(64.0, 64.0), Shared = true },
		{ Name = "content", Type = PropertyType.Integer, Array = true, Default = { 0 }, Shared = true },
		{ Name = "textures", Type = PropertyType.Texture, Array = true, Default = { "" }, Shared = true },
		{ Name = "textureCells", Type = PropertyType.IntegerSize, Array = true, Default = { defaultTextureCell }, Shared = true },
		{ Name = "mass", Type = PropertyType.Float, Default = 0, Shared = true },
		{ Name = "friction", Type = PropertyType.Float, Default = 1, Shared = true },
		{ Name = "physical", Type = PropertyType.Boolean, Default = true, Shared = true },
		{ Name = "renderOrder", Type = PropertyType.Integer, Default = 0, Shared = true }
	},
	EditorActions = {
		{
			Name = "editTilemap",
			Label = "Edit Tilemap",
			OnTrigger = function (entityEditor)
				local textures = entityEditor:GetProperty("textures")
				local textureCells = entityEditor:GetProperty("textureCells")
				local tiles = GenerateTiles(textures, textureCells)
				local materials = GenerateMaterialData(textures, textureCells)

				local tileMapEditor = TileMapEditorMode.new(Editor, entityEditor:GetTargetEntity(), {
					mapSize = entityEditor:GetProperty("mapSize"),
					tileSize = entityEditor:GetProperty("cellSize"),
					origin = entityEditor:GetPosition(),
					rotation = entityEditor:GetRotation(),
					content = entityEditor:GetProperty("content")
				}, materials, tiles)

				tileMapEditor:SetFinishedCallback(function (tileMapData)
					if (tileMapData) then
						entityEditor:UpdatePosition(tileMapData.origin)
						entityEditor:UpdateRotation(tileMapData.rotation)

						entityEditor:UpdateProperty("mapSize", tileMapData.mapSize)
						entityEditor:UpdateProperty("cellSize", tileMapData.tileSize)
						entityEditor:UpdateProperty("content", tileMapData.content)
					end

					entityEditor:Show()
				end)

				Editor:SwitchToMode(tileMapEditor)
				entityEditor:Hide()
			end
		}
	}
})

entity:On("init", function (self)
	local mapSize = self:GetProperty("mapSize")
	local cellSize = self:GetProperty("cellSize")
	local content = self:GetProperty("content")

	if (self:GetProperty("physical")) then
		local colliders = {}
		local y = 0

		while (y < mapSize.y) do
			local x = 0
			while (x < mapSize.x) do
				if (content[y * mapSize.x + x + 1] ~= 0) then
					local startX = x
					x = x + 1

					while (x < mapSize.x and content[y * mapSize.x + x + 1] ~= 0) do
						x = x + 1
					end

					local mins = Vec2(startX * cellSize.x, y * cellSize.y)
					local maxs = mins + Vec2((x - startX) * cellSize.x, cellSize.y)

					table.insert(colliders, Rect(mins, maxs))
				end

				x = x + 1
			end

			y = y + 1
		end

		if (#colliders > 0) then
			self:SetCollider(colliders)
			self:InitRigidBody(self:GetProperty("mass"), self:GetProperty("friction"))
		end
	end

	if (CLIENT) then
		local textures = self:GetProperty("textures")
		local textureCells = self:GetProperty("textureCells")
		local tiles = GenerateTiles(textures, textureCells)
		local renderOrder = self:GetProperty("renderOrder")

		self.Tilemap = self:AddTilemap(mapSize, cellSize, content, tiles, renderOrder)
	end
end)
