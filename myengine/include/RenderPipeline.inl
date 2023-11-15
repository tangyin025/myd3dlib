
namespace boost
{
	static size_t hash_value(const MaterialParameter & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.m_Name);
		switch (key.GetParameterType())
		{
		case MaterialParameter::ParameterTypeFloat:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat &>(key).m_Value);
			break;
		case MaterialParameter::ParameterTypeFloat2:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat2 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat2 &>(key).m_Value.y);
			break;
		case MaterialParameter::ParameterTypeFloat3:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.y);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat3 &>(key).m_Value.z);
			break;
		case MaterialParameter::ParameterTypeFloat4:
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.x);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.y);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.z);
			boost::hash_combine(seed, static_cast<const MaterialParameterFloat4 &>(key).m_Value.w);
			break;
		case MaterialParameter::ParameterTypeTexture:
			boost::hash_combine(seed, static_cast<const MaterialParameterTexture &>(key).m_TexturePath);
			break;
		}
		return seed;
	}

	static size_t hash_value(const Material & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.m_PassMask);
		boost::hash_combine(seed, key.m_PassMask);
		boost::hash_combine(seed, key.m_CullMode);
		boost::hash_combine(seed, key.m_ZEnable);
		boost::hash_combine(seed, key.m_ZWriteEnable);
		boost::hash_combine(seed, key.m_BlendMode);
		for (unsigned int i = 0; i < key.m_ParameterList.size(); i++)
		{
			boost::hash_combine(seed, *key.m_ParameterList[i]);
		}
		return seed;
	}

	static size_t hash_value(const RenderPipeline::MeshInstanceAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<2>());
		boost::hash_combine(seed, *key.get<3>());
		return seed;
	}

	static size_t hash_value(const RenderPipeline::EmitterInstanceAtomKey & key)
	{
		size_t seed = 0;
		boost::hash_combine(seed, key.get<0>());
		boost::hash_combine(seed, key.get<1>());
		boost::hash_combine(seed, key.get<3>());
		boost::hash_combine(seed, key.get<4>());
		boost::hash_combine(seed, key.get<5>());
		boost::hash_combine(seed, *key.get<6>());
		return seed;
	}
}

inline bool RenderPipeline::MeshInstanceAtomKey::operator == (const MeshInstanceAtomKey& rhs) const
{
	return get<0>() == rhs.get<0>()
		&& get<1>() == rhs.get<1>()
		&& get<2>() == rhs.get<2>()
		&& *get<3>() == *rhs.get<3>(); // ! mtl ptr must be valid object
}

inline bool RenderPipeline::EmitterInstanceAtomKey::operator == (const EmitterInstanceAtomKey& rhs) const
{
	return get<0>() == rhs.get<0>()
		&& get<1>() == rhs.get<1>()
		&& get<3>() == rhs.get<3>()
		&& get<4>() == rhs.get<4>()
		&& get<5>() == rhs.get<5>()
		&& *get<6>() == *rhs.get<6>();
}
