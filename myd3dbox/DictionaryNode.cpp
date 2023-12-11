#include "stdafx.h"
#include "DictionaryNode.h"
#include <boost/property_tree/info_parser.hpp>


void DictionaryNode::LoadFromFile(const char* path)
{
	my::IStreamBuff<wchar_t> buff(my::ResourceMgr::getSingleton().OpenIStream(path));
	std::wistream ifs(&buff);

	// ! skip utf bom, https://www.unicode.org/faq/utf_bom.html#bom1
	wchar_t header[1];
	ifs.read(header, _countof(header));
	_ASSERT(header[0] == 0xFEFF);

	boost::property_tree::read_info<DictionaryNode, wchar_t>(ifs, *this);
}
