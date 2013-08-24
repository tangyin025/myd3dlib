#define DIRECTINPUT_VERSION 0x0800
#define NOMINMAX
////Find all "#include <", Match case, Whole word, Subfolders, Keep modified files open, Find Results 1, "Current Project", "*.c;*.cpp;*.cxx;*.cc;*.tli;*.tlh;*.h;*.hpp;*.hxx;*.hh;*.inl;*.rc;*.resx;*.idl;*.asm;*.inc"
//#include <windows.h>
//#include <stdlib.h>
//#include <string>
//#include <string>
//#include <sstream>
//#include <boost/function.hpp>
//#include <boost/tuple/tuple.hpp>
//#include <boost/utility/enable_if.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/type_traits/is_void.hpp>
//#include <boost/type_traits/is_same.hpp>
//#include <limits>
//#include <boost/static_assert.hpp>
//extern "C"
//{
//#include <lua.h>
//#include <lauxlib.h>
//#include <lualib.h>
//}
//#include <vector>
//#include <d3d9.h>
//#include <ft2build.h>
//#include <atlbase.h>
//#include <hash_set>
//#include <d3dx9.h>
//#include <atlbase.h>
//#include <deque>
//#include <string>
//#include <Windows.h>
//#include <boost/shared_ptr.hpp>
//#include <d3dx9.h>
//#include <atltypes.h>
//#include <boost/unordered_map.hpp>
//#include <ft2build.h>
//#include <vector>
//#include <boost/shared_ptr.hpp>
//#include <dinput.h>
//#include <atlbase.h>
//#include <boost/shared_ptr.hpp>
//#include <crtdbg.h>
//#include <d3dx9.h>
//#include <d3d9.h>
//#include <set>
//#include <vector>
//#include <atlbase.h>
//#include <boost/shared_ptr.hpp>
//#include <boost/shared_ptr.hpp>
//#include <list>
//#include <vector>
//#include <unzip.h>
//#include <boost/weak_ptr.hpp>
//#include <boost/unordered_map.hpp>
//#include <boost/scoped_ptr.hpp>
//#include <Windows.h>
//#include <vector>
//#include <set>
//#include <boost/unordered_map.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/shared_ptr.hpp>
//#include <WTypes.h>
//#include <Mmsystem.h>
//#include <dsound.h>
//#include <boost/shared_ptr.hpp>
//#include <vector>
//#include <boost/shared_ptr.hpp>
//#include <d3d9.h>
//#include <d3dx9.h>
//#include <atlbase.h>
//#include <atltypes.h>
//#include <Windows.h>
//#include <atlbase.h>
//#include <atlwin.h>
//#include <atltypes.h>
//#include <string>
//#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
//#include <boost/weak_ptr.hpp>
//#include <set>
//#include <map>
//#include <d3d9.h>
//#include <string>
//#include <assert.h>
//#include <Windows.h>
//#include <assert.h>
//#include <math.h>
//#include <msctf.h>
//#include <malloc.h>
//#include <strsafe.h>
//#include <sstream>
//#include <d3d9.h>
//#include <dinput.h>
//#include <dsound.h>
//#include <strstream>
//#include <tchar.h>
//  //Matching lines: 91    Matching files: 28    Total files searched: 51

#define DEFINE_XML_NODE(node_v, node_p, node_s) \
	node_v = node_p->first_node(#node_s); \
	if(NULL == node_v) \
		THROW_CUSEXCEPTION("cannot find " #node_s)

#define DEFINE_XML_NODE_SIMPLE(node_s, parent_s) \
	rapidxml::xml_node<char> * node_##node_s; \
	DEFINE_XML_NODE(node_##node_s, node_##parent_s, node_s)

#define DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s) \
	attr_v = node_p->first_attribute(#attr_s); \
	if(NULL == attr_v) \
		THROW_CUSEXCEPTION("cannot find " #attr_s)

#define DEFINE_XML_ATTRIBUTE_SIMPLE(attr_s, parent_s) \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE(attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_INT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = atoi(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_INT_SIMPLE(attr_s, parent_s) \
	int attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_INT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_FLOAT(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = (float)atof(attr_v->value())

#define DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(attr_s, parent_s) \
	float attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_FLOAT(attr_s, attr_##attr_s, node_##parent_s, attr_s)

#define DEFINE_XML_ATTRIBUTE_BOOL(decl_v, attr_v, node_p, attr_s) \
	DEFINE_XML_ATTRIBUTE(attr_v, node_p, attr_s); \
	decl_v = 0 == _stricmp(attr_v->value(), "true")

#define DEFINE_XML_ATTRIBUTE_BOOL_SIMPLE(attr_s, parent_s) \
	bool attr_s; \
	rapidxml::xml_attribute<char> * attr_##attr_s; \
	DEFINE_XML_ATTRIBUTE_BOOL(attr_s, attr_##attr_s, node_##parent_s, attr_s)
