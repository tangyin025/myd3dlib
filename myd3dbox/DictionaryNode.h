// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <boost/unordered_map.hpp>
#include <string>

class DictionaryNode
{
public:
	std::wstring m_data;

	typedef boost::unordered_map<std::wstring, DictionaryNode> childmap;
	
	boost::shared_ptr<childmap> m_children;

	DictionaryNode(void)
		: m_children(new childmap())
	{
	}

	childmap::iterator push_back(const childmap::value_type& value)
	{
		std::pair<childmap::iterator, bool> res = m_children->insert(value);
		return res.first;
	}

	void swap(DictionaryNode& rhs)
	{
		boost::swap(m_data, rhs.m_data);
		// Void pointers, no ADL necessary
		m_children.swap(rhs.m_children);
	}

	std::wstring & data()
	{
		return m_data;
	}

	template <typename T>
	T get_value(void) const
	{
		return (T)m_data;
	}

	void put_value(const std::wstring& value)
	{
		m_data = value;
	}

	void LoadFromFile(const char* path);
};

