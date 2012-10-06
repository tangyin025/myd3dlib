module("Loader", package.seeall)

-- 防止garbage collect时清理掉还在使用的资源
resource_pool={}
function LoadTexture(name)
	if not resource_pool[name] then
		resource_pool[name]=game:LoadTexture(name, false)
	end
	return resource_pool[name]
end

function LoadCubeTexture(name)
	if not resource_pool[name] then
		resource_pool[name]=game:LoadCubeTexture(name, false)
	end
	return resource_pool[name]
end

function LoadMesh(name)
	if not resource_pool[name] then
		resource_pool[name]=game:LoadMesh(name, false)
	end
	return resource_pool[name]
end

function LoadSkeleton(name)
	if not resource_pool[name] then
		resource_pool[name]=game:LoadSkeleton(name, false)
	end
	return resource_pool[name]
end

function LoadEffect(name)
	if not resource_pool[name] then
		resource_pool[name]=game:LoadEffect(name, false)
	end
	return resource_pool[name]
end

function LoadFont(name, height)
	local res_name = name..height
	if not resource_pool[res_name] then
		resource_pool[res_name]=game:LoadFont(name, height, false)
	end
	return resource_pool[res_name]
end

function LoadMaterial(name)
	dofile("material/"..name..".lua")
	local mat = Material()
	SetupMaterial(mat)
	return mat
end

function LoadEffectMesh(name)
	local effectMesh = EffectMesh()
	effectMesh.Mesh = LoadMesh(name)
	for i = 0,effectMesh.Mesh:GetMaterialNum()-1 do
		effectMesh:InsertMaterial(LoadMaterial(effectMesh.Mesh:GetMaterialName(i)))
	end
	return effectMesh
end