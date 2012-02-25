#pragma once

#include "MyGUI_Prerequest.h"
#include "MyGUI_RenderFormat.h"
#include "MyGUI_IVertexBuffer.h"
#include "MyGUI_RenderManager.h"
#include "MyGUI_DataManager.h"
#include "MyGUI_DataStream.h"

namespace my
{
	class DirectXRenderManager :
		public MyGUI::RenderManager,
		public MyGUI::IRenderTarget
	{
	public:
		DirectXRenderManager();

		void initialise(IDirect3DDevice9* _device);

		void shutdown();

		static DirectXRenderManager& getInstance()
		{
			return *getInstancePtr();
		}

		static DirectXRenderManager* getInstancePtr()
		{
			return static_cast<DirectXRenderManager*>(RenderManager::getInstancePtr());
		}

		virtual const MyGUI::IntSize& getViewSize() const
		{
			return mViewSize;
		}

		virtual MyGUI::VertexColourType getVertexFormat()
		{
			return mVertexFormat;
		}

		virtual MyGUI::IVertexBuffer* createVertexBuffer();

		virtual void destroyVertexBuffer(MyGUI::IVertexBuffer* _buffer);

		virtual MyGUI::ITexture* createTexture(const std::string& _name);

		virtual void destroyTexture(MyGUI::ITexture* _texture);

		virtual MyGUI::ITexture* getTexture(const std::string& _name);

		virtual bool isFormatSupported(MyGUI::PixelFormat _format, MyGUI::TextureUsage _usage);

		virtual void begin();

		virtual void end();

		virtual void doRender(MyGUI::IVertexBuffer* _buffer, MyGUI::ITexture* _texture, size_t _count);

		virtual const MyGUI::RenderTargetInfo& getInfo()
		{
			return mInfo;
		}


		void drawOneFrame();

		void setViewSize(int _width, int _height);

		void deviceLost();

		void deviceRestore();

	private:
		void destroyAllResources();

	private:
		IDirect3DDevice9* mpD3DDevice;

		MyGUI::IntSize mViewSize;

		MyGUI::VertexColourType mVertexFormat;

		MyGUI::RenderTargetInfo mInfo;

		bool mUpdate;

		typedef std::map<std::string, MyGUI::ITexture*> MapTexture;

		MapTexture mTextures;

		bool mIsInitialise;
	};

	class DataMemoryStream :
		public MyGUI::DataStream
	{
	public:
		DataMemoryStream();
		DataMemoryStream(std::stringstream * _stream);
		virtual ~DataMemoryStream();

	private:
		std::stringstream * mMemoryStream;
	};

	class DirectXDataManager :
		public MyGUI::DataManager
	{
	public:
		DirectXDataManager();

		void initialise();

		void shutdown();

		static DirectXDataManager& getInstance()
		{
			return *getInstancePtr();
		}

		static DirectXDataManager* getInstancePtr()
		{
			return static_cast<DirectXDataManager*>(DataManager::getInstancePtr());
		}

		virtual MyGUI::IDataStream* getData(const std::string& _name);

		virtual bool isDataExist(const std::string& _name);

		virtual const MyGUI::VectorString& getDataListNames(const std::string& _pattern);

		virtual const std::string& getDataPath(const std::string& _name);

		void addResourceLocation(const std::string& _name, bool _recursive);

	private:
		struct ArhivInfo
		{
			std::wstring name;
			bool recursive;
		};

		typedef std::vector<ArhivInfo> VectorArhivInfo;

		VectorArhivInfo mPaths;

		bool mIsInitialise;
	};

	class DirectXVertexBuffer : public MyGUI::IVertexBuffer
	{
	public:
		DirectXVertexBuffer(IDirect3DDevice9* _device, DirectXRenderManager* _pRenderManager);

		virtual ~DirectXVertexBuffer();

		virtual void setVertexCount(size_t _count);

		virtual size_t getVertexCount();

		virtual MyGUI::Vertex* lock();

		virtual void unlock();

	/*internal:*/
		virtual bool setToStream(size_t stream);

	private:
		bool create();

		void destroy();

		void resize();

	private:
		IDirect3DDevice9* mpD3DDevice;

		IDirect3DVertexBuffer9* mpBuffer;

		DirectXRenderManager* pRenderManager;

		size_t mVertexCount;

		size_t mNeedVertexCount;
	};

	class DirectXTexture : public MyGUI::ITexture
	{
	public:
		DirectXTexture(const std::string& _name, IDirect3DDevice9* _device);

		virtual ~DirectXTexture();

		virtual const std::string& getName() const;

		virtual void createManual(int _width, int _height, MyGUI::TextureUsage _usage, MyGUI::PixelFormat _format);

		virtual void loadFromFile(const std::string& _filename);

		virtual void saveToFile(const std::string& _filename) { }

		virtual void destroy();

		virtual void* lock(MyGUI::TextureUsage _access);

		virtual void unlock();

		virtual bool isLocked();

		virtual int getWidth();

		virtual int getHeight();

		virtual MyGUI::PixelFormat getFormat();

		virtual MyGUI::TextureUsage getUsage();

		virtual size_t getNumElemBytes();

		virtual MyGUI::IRenderTarget* getRenderTarget();

		/*internal:*/
		IDirect3DTexture9* getDirectXTexture()
		{
			return mpTexture;
		}

		void deviceLost();

		void deviceRestore();

	private:
		IDirect3DDevice9* mpD3DDevice;

		IDirect3DTexture9* mpTexture;

		MyGUI::IntSize mSize;

		MyGUI::TextureUsage mTextureUsage;

		MyGUI::PixelFormat mPixelFormat;

		size_t mNumElemBytes;

		bool mLock;

		std::string mName;

		MyGUI::IRenderTarget* mRenderTarget;

		D3DPOOL mInternalPool;

		D3DFORMAT mInternalFormat;

		unsigned long mInternalUsage;
	};

	class DirectXRTTexture :
		public MyGUI::IRenderTarget
	{
	public:
		DirectXRTTexture(IDirect3DDevice9* _device, IDirect3DTexture9* _texture);

		virtual ~DirectXRTTexture();

		virtual void begin();

		virtual void end();

		virtual void doRender(MyGUI::IVertexBuffer* _buffer, MyGUI::ITexture* _texture, size_t _count);

		virtual const MyGUI::RenderTargetInfo& getInfo()
		{
			return mRenderTargetInfo;
		}

	private:
		IDirect3DDevice9* mpD3DDevice;

		IDirect3DTexture9* mpTexture;

		IDirect3DSurface9* mpRenderSurface;

		IDirect3DSurface9* mpBackBuffer;

		MyGUI::RenderTargetInfo mRenderTargetInfo;
	};
}
