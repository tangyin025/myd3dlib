
#include <tchar.h>
#include <atlbase.h>
#include <myResource.h>
#include <myException.h>
#include <D3DX9Effect.h>
#include <libc.h>
#include <crtdbg.h>

// ------------------------------------------------------------------------------------------
// _tmain
// ------------------------------------------------------------------------------------------

int _tmain(int argc, _TCHAR ** argv)
{
#if defined(DEBUG) | defined(_DEBUG)
	// 设置crtdbg监视内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		// 注册搜索路径
		//my::ResourceMgr::getSingleton().RegisterFileDir(_T("."));
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("..\\..\\common\\medias"));
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("..\\demo2_3"));

		// 打开指定文件
		my::ArchiveCachePtr cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("SimpleSample.fx")));

		// 编译d3dx effect
		CComPtr<ID3DXEffectCompiler> d3dxcompiler;
		CComPtr<ID3DXBuffer> d3dxbuffer;
		HRESULT hres = D3DXCreateEffectCompiler(
			(LPCSTR)&(*cache)[0], cache->size(), NULL, NULL, 0, &d3dxcompiler, &d3dxbuffer);

		// 若编译错误，则输出错误信息
		if(FAILED(hres))
		{
			_tprintf_s(_T("compiler error: \n%s\n"), mstringToTString((LPCSTR)d3dxbuffer->GetBufferPointer()).c_str());
		}
	}
	catch(const my::Exception & e)
	{
		_tprintf_s(_T("exception: \n%s\n"), e.GetFullDescription().c_str());
	}

	return 0;
}
