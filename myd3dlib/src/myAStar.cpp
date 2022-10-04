#include "myAStar.h"
#include "myResource.h"
#include <fstream>

using namespace my;

template class BilinearFiltering<D3DCOLOR>; // unresolved external symbol my::BilinearFiltering<unsigned long>::Sample

template <>
D3DCOLOR BilinearFiltering<D3DCOLOR>::Sample(float u, float v) const
{
	D3DCOLOR n[4];
	float uf, vf;
	SampleFourNeighbors(u, v, n[0], n[1], n[2], n[3], uf, vf);
	D3DXCOLOR r[3];
	D3DXColorLerp(&r[0], D3DXColorLerp(&r[1], &D3DXCOLOR(n[0]), &D3DXCOLOR(n[1]), uf), D3DXColorLerp(&r[2], &D3DXCOLOR(n[2]), &D3DXCOLOR(n[3]), uf), vf);
	return r[0];
}

void IndexedBitmap::LoadFromFile(const char* path)
{
	my::IStreamPtr ifs = ResourceMgr::getSingleton().OpenIStream(path);

	//-		bfh	{bfType=19778 bfSize=787512 bfReserved1=0 ...}	tagBITMAPFILEHEADER
	//		bfType	19778	unsigned short
	//		bfSize	787512	unsigned long
	//		bfReserved1	0	unsigned short
	//		bfReserved2	0	unsigned short
	//		bfOffBits	1078	unsigned long
	BITMAPFILEHEADER bfh;

	//-		bih	{biSize=40 biWidth=1024 biHeight=768 ...}	tagBITMAPINFOHEADER
	//		biSize	40	unsigned long
	//		biWidth	1024	long
	//		biHeight	768	long
	//		biPlanes	1	unsigned short
	//		biBitCount	8	unsigned short
	//		biCompression	0	unsigned long
	//		biSizeImage	786434	unsigned long
	//		biXPelsPerMeter	11808	long
	//		biYPelsPerMeter	11808	long
	//		biClrUsed	0	unsigned long
	//		biClrImportant	0	unsigned long
	BITMAPINFOHEADER bih;

	if (sizeof(bfh) != ifs->read(&bfh, sizeof(bfh)) || bfh.bfType != 0x4D42)
	{
		THROW_CUSEXCEPTION("sizeof(bfh) != ifs->read");
	}

	if (sizeof(bih) != ifs->read(&bih, sizeof(bih)))
	{
		THROW_CUSEXCEPTION("sizeof(bih) != ifs->read");
	}

	if (bih.biCompression != BI_RGB || bih.biBitCount != 8)
	{
		THROW_CUSEXCEPTION("bih.biCompression != BI_RGB || bih.biBitCount != 8");
	}

	int pitch = (bih.biWidth * bih.biBitCount + 31) / 32 * 4;
	for (int i = 0; i < Min<int>(GetHeight(), bih.biHeight); i++)
	{
		long offset = bfh.bfOffBits + (bih.biHeight - i - 1) * pitch;
		if (offset != ifs->seek(offset, SEEK_SET))
		{
			THROW_CUSEXCEPTION("offset != ifs->seek");
		}

		int rdnum = Min<int>(GetWidth(), bih.biWidth) * sizeof(element);
		if (rdnum != ifs->read(&operator[](i)[0], rdnum))
		{
			THROW_CUSEXCEPTION("rdnum != ifs->read");
		}

		_ASSERT(ifs->tell() <= bfh.bfSize);
	}
}

void IndexedBitmap::SaveIndexedBitmap(const char* path, const boost::function<DWORD(unsigned char)> & get_color)
{
	std::ofstream ofs(path, std::ios::binary, _SH_DENYRW);
	if (!ofs.is_open())
	{
		THROW_CUSEXCEPTION("!ofs.is_open()");
	}

	BITMAPFILEHEADER bfh = { 0 };
	BITMAPINFOHEADER bih = { 0 };
	bfh.bfType = 0x4D42;
	bfh.bfOffBits = sizeof(bfh) + sizeof(bih) + sizeof(RGBQUAD) * 256;
	bfh.bfSize = bfh.bfOffBits + num_elements() * sizeof(element);
	bih.biSize = sizeof(bih);
	bih.biWidth = GetWidth();
	bih.biHeight = GetHeight();
	bih.biPlanes = 1;
	bih.biBitCount = 8;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = num_elements() * sizeof(element);
	bih.biXPelsPerMeter = 11808;
	bih.biYPelsPerMeter = 11808;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	ofs.write((char*)&bfh, sizeof(bfh));
	ofs.write((char*)&bih, sizeof(bih));

	for (int i = 0; i < 256; i++)
	{
		DWORD color = get_color(i);
		RGBQUAD q = { LOBYTE(color), LOBYTE(color >> 8), LOBYTE(color >> 16), 0 };
		ofs.write((char*)&q, sizeof(q));
	}

	_ASSERT(ofs.tellp() == bfh.bfOffBits);

	for (int i = 0; i < GetHeight(); i++)
	{
		ofs.write((char*)&operator[](GetHeight() - i - 1)[0], GetWidth() * sizeof(element));
	}

	_ASSERT(ofs.tellp() == bfh.bfSize);
}
