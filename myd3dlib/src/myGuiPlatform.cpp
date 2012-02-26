#include "StdAfx.h"
#include "myd3dlib.h"

#include <d3dx9.h>
//#include "MyGUI_DirectXRenderManager.h"
//#include "MyGUI_DirectXTexture.h"
//#include "MyGUI_DirectXVertexBuffer.h"
//#include "MyGUI_DirectXDiagnostic.h"
#include "MyGUI_Gui.h"
#include "MyGUI_Timer.h"

using namespace my;
using namespace MyGUI;

DirectXRenderManager::DirectXRenderManager() :
	mIsInitialise(false),
	mpD3DDevice(nullptr),
	mUpdate(false)
{
}

void DirectXRenderManager::initialise(IDirect3DDevice9* _device)
{
	////MYGUI_PLATFORM_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
	////MYGUI_PLATFORM_LOG(Info, "* Initialise: " << getClassTypeName());

	mpD3DDevice = _device;

	mVertexFormat = VertexColourType::ColourARGB;

	memset(&mInfo, 0, sizeof(mInfo));
	if (mpD3DDevice != nullptr)
	{
		D3DVIEWPORT9 vp;
		mpD3DDevice->GetViewport(&vp);
		setViewSize(vp.Width, vp.Height);
	}

	mUpdate = false;

	if (mpD3DDevice != nullptr)
	{
		D3DCAPS9 caps;
		mpD3DDevice->GetDeviceCaps(&caps);
		if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
		{
			//MYGUI_PLATFORM_LOG(Warning, "Non-squared textures not supported.");
		}
	}

	//MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully initialized");
	mIsInitialise = true;
}

void DirectXRenderManager::shutdown()
{
	//MYGUI_PLATFORM_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
	//MYGUI_PLATFORM_LOG(Info, "* Shutdown: " << getClassTypeName());

	destroyAllResources();
	mpD3DDevice = nullptr;

	//MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully shutdown");
	mIsInitialise = false;
}

IVertexBuffer* DirectXRenderManager::createVertexBuffer()
{
	return new DirectXVertexBuffer(mpD3DDevice, this);
}

void DirectXRenderManager::destroyVertexBuffer(IVertexBuffer* _buffer)
{
	delete _buffer;
}

void DirectXRenderManager::doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count)
{
	DirectXTexture* dxTex = static_cast<DirectXTexture*>(_texture);
	mpD3DDevice->SetTexture(0, dxTex->getDirectXTexture());
	DirectXVertexBuffer* dxVB = static_cast<DirectXVertexBuffer*>(_buffer);
	dxVB->setToStream(0);
	// count in vertexes, triangle_list = vertexes / 3
	mpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, _count / 3); // Microsoft C++ exception: long at memory location 0x0014d36c..
}

void DirectXRenderManager::drawOneFrame()
{
	Gui* gui = Gui::getInstancePtr();
	if (gui == nullptr)
		return;

	static Timer timer;
	static unsigned long last_time = timer.getMilliseconds();
	unsigned long now_time = timer.getMilliseconds();
	unsigned long time = now_time - last_time;

	onFrameEvent((float)((double)(time) / (double)1000));

	last_time = now_time;

	begin();
	onRenderToTarget(this, mUpdate);
	end();

	mUpdate = false;
}

void DirectXRenderManager::begin()
{
	mpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	mpD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_DIFFUSE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	mpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG0, D3DTA_DIFFUSE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	mpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	mpD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	mpD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	mpD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	mpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	mpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	mpD3DDevice->SetRenderState(D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA);
	mpD3DDevice->SetRenderState(D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA);

	mpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	mpD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	mpD3DDevice->SetRenderState(D3DRS_LIGHTING, 0);
	mpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	mpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	mpD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	mpD3DDevice->SetTransform(D3DTS_WORLD, &m);
	mpD3DDevice->SetTransform(D3DTS_VIEW, &m);
	mpD3DDevice->SetTransform(D3DTS_PROJECTION, &m);
}

void DirectXRenderManager::end()
{
}

ITexture* DirectXRenderManager::createTexture(const std::string& _name)
{
	MapTexture::const_iterator item = mTextures.find(_name);
	//MYGUI_PLATFORM_ASSERT(item == mTextures.end(), "Texture '" << _name << "' already exist");

	DirectXTexture* texture = new DirectXTexture(_name, mpD3DDevice);
	mTextures[_name] = texture;
	return texture;
}

void DirectXRenderManager::destroyTexture(ITexture* _texture)
{
	if (_texture == nullptr)
		return;

	MapTexture::iterator item = mTextures.find(_texture->getName());
	//MYGUI_PLATFORM_ASSERT(item != mTextures.end(), "Texture '" << _texture->getName() << "' not found");

	mTextures.erase(item);
	delete _texture;
}

ITexture* DirectXRenderManager::getTexture(const std::string& _name)
{
	MapTexture::const_iterator item = mTextures.find(_name);
	if (item == mTextures.end())
		return nullptr;
	return item->second;
}

bool DirectXRenderManager::isFormatSupported(PixelFormat _format, TextureUsage _usage)
{
	D3DFORMAT internalFormat = D3DFMT_UNKNOWN;
	unsigned long internalUsage = 0;
	D3DPOOL internalPool = D3DPOOL_MANAGED;

	if (_usage == TextureUsage::RenderTarget)
	{
		internalUsage |= D3DUSAGE_RENDERTARGET;
		internalPool = D3DPOOL_MANAGED;
	}
	else if (_usage == TextureUsage::Dynamic)
		internalUsage |= D3DUSAGE_DYNAMIC;
	else if (_usage == TextureUsage::Stream)
		internalUsage |= D3DUSAGE_DYNAMIC;

	if (_format == PixelFormat::R8G8B8A8)
	{
		internalFormat = D3DFMT_A8R8G8B8;
	}
	else if (_format == PixelFormat::R8G8B8)
	{
		internalFormat = D3DFMT_R8G8B8;
	}
	else if (_format == PixelFormat::L8A8)
	{
		internalFormat = D3DFMT_A8L8;
	}
	else if (_format == PixelFormat::L8)
	{
		internalFormat = D3DFMT_L8;
	}

	D3DFORMAT requestedlFormat = internalFormat;
	D3DXCheckTextureRequirements(mpD3DDevice, NULL, NULL, NULL, internalUsage, &internalFormat, internalPool);

	bool result = requestedlFormat == internalFormat;
	//if (!result)
	//	//MYGUI_PLATFORM_LOG(Warning, "Texture format '" << requestedlFormat << "'is not supported.");
	return result;
}

void DirectXRenderManager::destroyAllResources()
{
	for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
	{
		delete item->second;
	}
	mTextures.clear();
}

void DirectXRenderManager::setViewSize(int _width, int _height)
{
	if (_height == 0)
		_height = 1;
	if (_width == 0)
		_width = 1;

	mViewSize.set(_width, _height);

	mInfo.maximumDepth = 0.0f;
	mInfo.hOffset = -0.5f / float(mViewSize.width);
	mInfo.vOffset = -0.5f / float(mViewSize.height);
	mInfo.aspectCoef = float(mViewSize.height) / float(mViewSize.width);
	mInfo.pixScaleX = 1.0f / float(mViewSize.width);
	mInfo.pixScaleY = 1.0f / float(mViewSize.height);

	onResizeView(mViewSize);

	mUpdate = true;
}

void DirectXRenderManager::deviceLost()
{
	//MYGUI_PLATFORM_LOG(Info, "device D3D lost");

	for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
	{
		static_cast<DirectXTexture*>(item->second)->deviceLost();
	}
}

void DirectXRenderManager::deviceRestore()
{
	//MYGUI_PLATFORM_LOG(Info, "device D3D restore");

	for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
	{
		static_cast<DirectXTexture*>(item->second)->deviceRestore();
	}

	mUpdate = true;
}

DataMemoryStream::DataMemoryStream() :
	DataStream(),
	mMemoryStream(nullptr)
{
}

DataMemoryStream::DataMemoryStream(std::stringstream * _stream) :
	DataStream(_stream),
	mMemoryStream(_stream)
{
}

DataMemoryStream::~DataMemoryStream()
{
	if (mMemoryStream != nullptr)
	{
		delete mMemoryStream;
		mMemoryStream = nullptr;
	}
}

DirectXDataManager::DirectXDataManager() :
	mIsInitialise(false)
{
}

void DirectXDataManager::initialise()
{
	//MYGUI_PLATFORM_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
	//MYGUI_PLATFORM_LOG(Info, "* Initialise: " << getClassTypeName());

	//MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully initialized");
	mIsInitialise = true;
}

void DirectXDataManager::shutdown()
{
	//MYGUI_PLATFORM_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
	//MYGUI_PLATFORM_LOG(Info, "* Shutdown: " << getClassTypeName());

	//MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully shutdown");
	mIsInitialise = false;
}

IDataStream* DirectXDataManager::getData(const std::string& _name)
{
	if(!isDataExist(_name))
		return nullptr;

	CachePtr cache = ResourceMgr::getSingleton().OpenArchiveStream(_name, "")->GetWholeCache();
	std::stringstream * stream = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	stream->write((char *)&(*cache)[0], cache->size());
	stream->seekg(0);
	return new DataMemoryStream(stream);
}

bool DirectXDataManager::isDataExist(const std::string& _name)
{
	return ResourceMgr::getSingleton().CheckArchivePath(_name);
}

const VectorString& DirectXDataManager::getDataListNames(const std::string& _pattern)
{
	//static VectorString result;
	//common::VectorWString wresult;
	//result.clear();

	//for (VectorArhivInfo::const_iterator item = mPaths.begin(); item != mPaths.end(); ++item)
	//{
	//	common::scanFolder(wresult, (*item).name, (*item).recursive, MyGUI::UString(_pattern).asWStr(), false);
	//}

	//for (common::VectorWString::const_iterator item = wresult.begin(); item != wresult.end(); ++item)
	//{
	//	result.push_back(MyGUI::UString(*item).asUTF8());
	//}

	//return result;
	THROW_CUSEXCEPTION("havent implemented yet");
	static VectorString res;
	return res;
}

const std::string& DirectXDataManager::getDataPath(const std::string& _name)
{
	//static std::string path;
	//VectorString result;
	//common::VectorWString wresult;

	//for (VectorArhivInfo::const_iterator item = mPaths.begin(); item != mPaths.end(); ++item)
	//{
	//	common::scanFolder(wresult, (*item).name, (*item).recursive, MyGUI::UString(_name).asWStr(), true);
	//}

	//for (common::VectorWString::const_iterator item = wresult.begin(); item != wresult.end(); ++item)
	//{
	//	result.push_back(MyGUI::UString(*item).asUTF8());
	//}

	//path = result.size() == 1 ? result[0] : "";
	//return path;
	THROW_CUSEXCEPTION("havent implemented yet");
	static std::string ret;
	return ret;
}

void DirectXDataManager::addResourceLocation(const std::string& _name, bool _recursive)
{
	//ArhivInfo info;
	//info.name = MyGUI::UString(_name).asWStr();
	//info.recursive = _recursive;
	//mPaths.push_back(info);
	THROW_CUSEXCEPTION("havent implemented yet");
}

const size_t VERTEX_IN_QUAD = 6;
const size_t RENDER_ITEM_STEEP_REALLOCK = 5 * VERTEX_IN_QUAD;

DirectXVertexBuffer::DirectXVertexBuffer(IDirect3DDevice9* _device, DirectXRenderManager* _pRenderManager) :
	mNeedVertexCount(0),
	//mVertexCount(RENDER_ITEM_STEEP_REALLOCK),
	mpD3DDevice(_device),
	pRenderManager(_pRenderManager),
	mpBuffer(NULL)
{
}

DirectXVertexBuffer::~DirectXVertexBuffer()
{
	destroy();
}

void DirectXVertexBuffer::setVertexCount(size_t _count)
{
	if (_count != mNeedVertexCount)
	{
		mNeedVertexCount = _count;
		resize();
	}
}

size_t DirectXVertexBuffer::getVertexCount()
{
	return mNeedVertexCount;
}

MyGUI::Vertex* DirectXVertexBuffer::lock()
{
	void* lockPtr = nullptr;
	HRESULT result = mpBuffer->Lock(0, 0, (void**)&lockPtr, 0);
	if (FAILED(result))
	{
		//MYGUI_PLATFORM_EXCEPT("Failed to lock vertex buffer (error code " << result << ").");
		THROW_D3DEXCEPTION(result);
	}
	return (MyGUI::Vertex*)lockPtr;
}

void DirectXVertexBuffer::unlock()
{
	HRESULT result = mpBuffer->Unlock();
	if (FAILED(result))
	{
		//MYGUI_PLATFORM_EXCEPT("Failed to unlock vertex buffer (error code " << result << ").");
		THROW_D3DEXCEPTION(result);
	}
}

bool DirectXVertexBuffer::setToStream(size_t stream)
{
	if (SUCCEEDED(mpD3DDevice->SetStreamSource(stream, mpBuffer, 0, sizeof(MyGUI::Vertex))))
		return true;
	return false;
}

bool DirectXVertexBuffer::create()
{
	DWORD length = mNeedVertexCount * sizeof(MyGUI::Vertex);
	if (SUCCEEDED(mpD3DDevice->CreateVertexBuffer(length, 0, 0, D3DPOOL_MANAGED, &mpBuffer, nullptr)))
		return false;
	return false;
}

void DirectXVertexBuffer::destroy()
{
	if (mpBuffer)
	{
		mpBuffer->Release();
		mpBuffer = nullptr;
	}
}

void DirectXVertexBuffer::resize()
{
	if (mpD3DDevice)
	{
		destroy();
		create();
	}
}

DirectXTexture::DirectXTexture(const std::string& _name, IDirect3DDevice9* _device) :
	mName(_name),
	mpD3DDevice(_device),
	mpTexture(NULL),
	mNumElemBytes(0),
	mLock(false),
	mRenderTarget(nullptr),
	mInternalPool(D3DPOOL_MANAGED),
	mInternalFormat(D3DFMT_UNKNOWN),
	mInternalUsage(0)
{
}

DirectXTexture::~DirectXTexture()
{
	destroy();
}

const std::string& DirectXTexture::getName() const
{
	return mName;
}

void DirectXTexture::createManual(int _width, int _height, TextureUsage _usage, PixelFormat _format)
{
	destroy();

	mInternalUsage = 0;
	mInternalFormat = D3DFMT_UNKNOWN;

	mSize.set(_width, _height);
	mTextureUsage = _usage;
	mPixelFormat = _format;
	mInternalPool = D3DPOOL_MANAGED;

	if (mTextureUsage == TextureUsage::RenderTarget)
	{
		mInternalUsage |= D3DUSAGE_RENDERTARGET;
		mInternalPool = D3DPOOL_DEFAULT;
	}
	else if (mTextureUsage == TextureUsage::Dynamic)
		mInternalUsage |= D3DUSAGE_DYNAMIC;
	else if (mTextureUsage == TextureUsage::Stream)
		mInternalUsage |= D3DUSAGE_DYNAMIC;

	if (mPixelFormat == PixelFormat::R8G8B8A8)
	{
		mInternalFormat = D3DFMT_A8R8G8B8;
		mNumElemBytes = 4;
	}
	else if (mPixelFormat == PixelFormat::R8G8B8)
	{
		mInternalFormat = D3DFMT_R8G8B8;
		mNumElemBytes = 3;
	}
	else if (mPixelFormat == PixelFormat::L8A8)
	{
		mInternalFormat = D3DFMT_A8L8;
		mNumElemBytes = 2;
	}
	else if (mPixelFormat == PixelFormat::L8)
	{
		mInternalFormat = D3DFMT_L8;
		mNumElemBytes = 1;
	}
	else
	{
		//MYGUI_PLATFORM_EXCEPT("Creating texture with unknown pixel formal.");
		THROW_CUSEXCEPTION("Creating texture with unknown pixel formal.");
	}

	HRESULT result = mpD3DDevice->CreateTexture(mSize.width, mSize.height, 1, mInternalUsage, mInternalFormat, mInternalPool, &mpTexture, NULL);
	if (FAILED(result))
	{
		//MYGUI_PLATFORM_EXCEPT("Failed to create texture (error code " << result <<"): size '" << mSize <<
		//	"' internal usage '" << mInternalUsage <<
		//	"' internal format '" << mInternalFormat << "'."
		//	);
		THROW_D3DEXCEPTION(result);
	}

}

void DirectXTexture::loadFromFile(const std::string& _filename)
{
	destroy();
	mTextureUsage = TextureUsage::Default;
	mPixelFormat = PixelFormat::R8G8B8A8;
	mNumElemBytes = 4;

	//std::string fullname = DirectXDataManager::getInstance().getDataPath(_filename);
	CachePtr cache = ResourceMgr::getSingleton().OpenArchiveStream(_filename)->GetWholeCache();

	D3DXIMAGE_INFO info;
	//D3DXGetImageInfoFromFileA(fullname.c_str(), &info);
	D3DXGetImageInfoFromFileInMemory(&(*cache)[0], cache->size(), &info);

	if (info.Format == D3DFMT_A8R8G8B8)
	{
		mPixelFormat = PixelFormat::R8G8B8A8;
		mNumElemBytes = 4;
	}
	else if (info.Format == D3DFMT_R8G8B8)
	{
		mPixelFormat = PixelFormat::R8G8B8;
		mNumElemBytes = 3;
	}
	else if (info.Format == D3DFMT_A8L8)
	{
		mPixelFormat = PixelFormat::L8A8;
		mNumElemBytes = 2;
	}
	else if (info.Format == D3DFMT_L8)
	{
		mPixelFormat = PixelFormat::L8;
		mNumElemBytes = 1;
	}

	mSize.set(info.Width, info.Height);
	//HRESULT result = D3DXCreateTextureFromFileA(mpD3DDevice, fullname.c_str(), &mpTexture);
	HRESULT result = D3DXCreateTextureFromFileInMemory(mpD3DDevice, &(*cache)[0], cache->size(), &mpTexture);
	if (FAILED(result))
	{
		//MYGUI_PLATFORM_EXCEPT("Failed to load texture '" << _filename <<
		//	"' (error code " << result <<
		//	"): size '" << mSize <<
		//	"' format '" << info.Format << "'."
		//	);
		THROW_D3DEXCEPTION(result);
	}
}

void DirectXTexture::destroy()
{
	if (mRenderTarget != nullptr)
	{
		delete mRenderTarget;
		mRenderTarget = nullptr;
	}

	if (mpTexture != nullptr)
	{
		int nNewRefCount = mpTexture->Release();

		if (nNewRefCount > 0)
		{
			//MYGUI_PLATFORM_EXCEPT("The texture object failed to cleanup properly.\n"
			//	"Release() returned a reference count of '" << nNewRefCount << "'."
			//	);
			THROW_CUSEXCEPTION("The texture object failed to cleanup properly.");
		}

		mpTexture = nullptr;
	}
}

int DirectXTexture::getWidth()
{
	return mSize.width;
}

int DirectXTexture::getHeight()
{
	return mSize.height;
}

void* DirectXTexture::lock(TextureUsage _access)
{
	D3DLOCKED_RECT d3dlr;
	int lockFlag = (_access == TextureUsage::Write) ? D3DLOCK_DISCARD : D3DLOCK_READONLY;

	HRESULT result = mpTexture->LockRect(0, &d3dlr, NULL, lockFlag);
	//if (FAILED(result))
	//{
	//	//MYGUI_PLATFORM_EXCEPT("Failed to lock texture (error code " << result << ").");
	//	THROW_D3DEXCEPTION(result);
	//}

	//mLock = true;
	//return d3dlr.pBits;
	if(SUCCEEDED(result))
	{
		mLock = true;
		return d3dlr.pBits;
	}
	return nullptr;
}

void DirectXTexture::unlock()
{
	if(!mLock)
		return;

	HRESULT result = mpTexture->UnlockRect(0);
	if (FAILED(result))
	{
		//MYGUI_PLATFORM_EXCEPT("Failed to unlock texture (error code " << result << ").");
		THROW_D3DEXCEPTION(result);
	}

	mLock = false;
}

bool DirectXTexture::isLocked()
{
	return mLock;
}

PixelFormat DirectXTexture::getFormat()
{
	return mPixelFormat;
}

size_t DirectXTexture::getNumElemBytes()
{
	return mNumElemBytes;
}

TextureUsage DirectXTexture::getUsage()
{
	return mTextureUsage;
}

IRenderTarget* DirectXTexture::getRenderTarget()
{
	if (mpTexture == nullptr)
		return nullptr;

	if (mRenderTarget == nullptr)
		mRenderTarget = new DirectXRTTexture(mpD3DDevice, mpTexture);

	return mRenderTarget;
}

void DirectXTexture::deviceLost()
{
	if (mInternalPool == D3DPOOL_DEFAULT)
	{
		destroy();
	}
}

void DirectXTexture::deviceRestore()
{
	if (mInternalPool == D3DPOOL_DEFAULT)
	{
		HRESULT result = mpD3DDevice->CreateTexture(mSize.width, mSize.height, 1, mInternalUsage, mInternalFormat, D3DPOOL_DEFAULT, &mpTexture, NULL);
		if (FAILED(result))
		{
			//MYGUI_PLATFORM_EXCEPT("Failed to recreate texture on device restore (error code " << result << ").");
			THROW_D3DEXCEPTION(result);
		}
	}
}

DirectXRTTexture::DirectXRTTexture(IDirect3DDevice9* _device, IDirect3DTexture9* _texture) :
	mpD3DDevice(_device),
	mpTexture(_texture),
	mpRenderSurface(NULL),
	mpBackBuffer(NULL)
{
	mpTexture->GetSurfaceLevel(0, &mpRenderSurface);

	D3DSURFACE_DESC info;
	mpTexture->GetLevelDesc(0, &info);
	int width = info.Width;
	int height = info.Height;

	mRenderTargetInfo.maximumDepth = 0.0f;
	mRenderTargetInfo.hOffset = -0.5f / float(width);
	mRenderTargetInfo.vOffset = -0.5f / float(height);
	mRenderTargetInfo.aspectCoef = float(height) / float(width);
	mRenderTargetInfo.pixScaleX = 1.0f / float(width);
	mRenderTargetInfo.pixScaleY = 1.0f / float(height);
}

DirectXRTTexture::~DirectXRTTexture()
{
	if (mpRenderSurface != nullptr)
	{
		mpRenderSurface->Release();
		mpRenderSurface = nullptr;
	}
}

void DirectXRTTexture::begin()
{
	mpD3DDevice->GetRenderTarget(0, &mpBackBuffer);

	mpD3DDevice->SetRenderTarget(0, mpRenderSurface);
	mpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET,
		D3DCOLOR_RGBA(0, 0, 0, 0), 1, 0);

	mpD3DDevice->BeginScene();
}

void DirectXRTTexture::end()
{
	mpD3DDevice->EndScene();

	mpD3DDevice->SetRenderTarget(0, mpBackBuffer);
	mpBackBuffer->Release();
}

void DirectXRTTexture::doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count)
{
	DirectXRenderManager::getInstance().doRender(_buffer, _texture, _count);
}
