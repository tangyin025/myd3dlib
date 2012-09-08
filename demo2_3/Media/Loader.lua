-- 防止garbage collect时清理掉还在使用的资源
texture_pool={}
function LoadTexture(name)
	if not texture_pool[name] then
		texture_pool[name]=game:LoadTexture(name)
	end
	return texture_pool[name]
end

effect_pool={}
function LoadEffect(name)
	if not effect_pool[name] then
		effect_pool[name]=game:LoadEffect(name)
	end
	return effect_pool[name]
end

effect_mesh_pool={}
function LoadEffectMesh(name)
	if not effect_mesh_pool[name] then
		effect_mesh_pool[name]=game:LoadEffectMesh(name)
	end
	return effect_mesh_pool[name]
end

skeleton_pool={}
function LoadSkeleton(name)
	if not skeleton_pool[name] then
		skeleton_pool[name]=game:LoadSkeleton(name)
	end
	return skeleton_pool[name]
end

cube_texture_pool={}
function LoadCubeTexture(name)
	if not cube_texture_pool[name] then
		cube_texture_pool[name]=game:LoadCubeTexture(name)
	end
	return cube_texture_pool[name]
end