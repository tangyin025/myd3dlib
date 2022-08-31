#include "myUi.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "myCollision.h"
#include "myUtility.h"
#include "mySound.h"
#include "ImeUi.h"
#include "libc.h"
#include <fstream>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/scope_exit.hpp>

using namespace my;

BOOST_CLASS_EXPORT(ControlSkin)

BOOST_CLASS_EXPORT(Control)

BOOST_CLASS_EXPORT(StaticSkin)

BOOST_CLASS_EXPORT(Static)

BOOST_CLASS_EXPORT(ProgressBarSkin)

BOOST_CLASS_EXPORT(ProgressBar)

BOOST_CLASS_EXPORT(ButtonSkin)

BOOST_CLASS_EXPORT(Button)

BOOST_CLASS_EXPORT(EditBoxSkin)

BOOST_CLASS_EXPORT(EditBox)

BOOST_CLASS_EXPORT(ImeEditBox)

BOOST_CLASS_EXPORT(ScrollBarSkin)

BOOST_CLASS_EXPORT(ScrollBar)

BOOST_CLASS_EXPORT(CheckBox)

BOOST_CLASS_EXPORT(ComboBoxSkin)

BOOST_CLASS_EXPORT(ComboBox)

BOOST_CLASS_EXPORT(ListBoxSkin)

BOOST_CLASS_EXPORT(ListBox)

BOOST_CLASS_EXPORT(DialogSkin)

BOOST_CLASS_EXPORT(Dialog)

UIRender::UIRender(void)
	: m_WhiteTex(new my::Texture2D())
	, m_LayerDrawCall(0)
{
	UILayerList::iterator layer_iter = m_Layer.begin();
	for (; layer_iter != m_Layer.end(); layer_iter++)
	{
		layer_iter->first = NULL;
	}
}

UIRender::~UIRender(void)
{
	_ASSERT(!m_Device);
	_ASSERT(!m_WhiteTex->m_ptr);
}

HRESULT UIRender::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_Device = pd3dDevice;

	m_WhiteTex->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8);
	RECT rc = { 0, 0, 1, 1 };
	D3DLOCKED_RECT SrcLrc = m_WhiteTex->LockRect(&rc, 0, 0);
	D3DCOLOR * DstBits = (D3DCOLOR *)SrcLrc.pBits;
	*DstBits = D3DCOLOR_ARGB(255, 255, 255, 255);
	m_WhiteTex->UnlockRect();

	return S_OK;
}

HRESULT UIRender::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void UIRender::OnLostDevice(void)
{
}

void UIRender::OnDestroyDevice(void)
{
	m_Device.Release();
}

void UIRender::Begin(void)
{
	m_LayerDrawCall = 0;

	V(m_Device->SetVertexShader(NULL));
	V(m_Device->SetPixelShader(NULL));
	V(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(m_Device->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	V(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	V(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	V(m_Device->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	V(m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	V(m_Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	V(m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
}

void UIRender::End(void)
{
	Flush();
}

void UIRender::SetWorld(const Matrix4 & World)
{
	V(m_Device->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&World));
}

void UIRender::SetViewProj(const Matrix4 & ViewProj)
{
	V(m_Device->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&ViewProj));
	V(m_Device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Matrix4::identity));
}

void UIRender::Flush(void)
{
	UILayerList::iterator layer_iter = m_Layer.begin();
	for (; layer_iter != m_Layer.end(); layer_iter++)
	{
		if (!layer_iter->second.empty())
		{
			_ASSERT(layer_iter->first && layer_iter->first->m_ptr);

			V(m_Device->SetTexture(0, layer_iter->first->m_ptr));
			V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
			V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, layer_iter->second.size() / 3, &layer_iter->second[0], sizeof(CUSTOMVERTEX)));
			m_LayerDrawCall++;
		}
	}
	m_Layer.clear();
}

void UIRender::PushTriangleSimple(VertexList & vertex_list, const CUSTOMVERTEX & v0, const CUSTOMVERTEX & v1, const CUSTOMVERTEX & v2)
{
	vertex_list.push_back(v0);
	vertex_list.push_back(v1);
	vertex_list.push_back(v2);
}

static void _PushTriangleClipX_1_2(UIRender::VertexList & vertex_list, const UIRender::CUSTOMVERTEX & v0, const UIRender::CUSTOMVERTEX & v1, const UIRender::CUSTOMVERTEX & v2, float x, const my::Rectangle & clip, DWORD clipmask)
{
	float alpha0 = (x - v0.x) / (v1.x - v0.x);
	float alpha1 = (x - v0.x) / (v2.x - v0.x);
	UIRender::CUSTOMVERTEX v3 = { x, Lerp(v0.y, v1.y, alpha0), Lerp(v0.z, v1.z, alpha0), v0.color, Lerp(v0.u, v1.u, alpha0), Lerp(v0.v, v1.v, alpha0) };
	UIRender::CUSTOMVERTEX v4 = { x, Lerp(v0.y, v2.y, alpha1), Lerp(v0.z, v2.z, alpha1), v0.color, Lerp(v0.u, v2.u, alpha1), Lerp(v0.v, v2.v, alpha1) };
	UIRender::PushTriangleSimple(vertex_list, v3, v1, v2, clip, clipmask);
	UIRender::PushTriangleSimple(vertex_list, v3, v2, v4, clip, clipmask);
}

static void _PushTriangleClipX_2_1(UIRender::VertexList & vertex_list, const UIRender::CUSTOMVERTEX & v0, const UIRender::CUSTOMVERTEX & v1, const UIRender::CUSTOMVERTEX & v2, float x, const my::Rectangle & clip, DWORD clipmask)
{
	float alpha0 = (x - v0.x) / (v2.x - v0.x);
	float alpha1 = (x - v1.x) / (v2.x - v1.x);
	UIRender::CUSTOMVERTEX v3 = { x, Lerp(v0.y, v2.y, alpha0), Lerp(v0.z, v2.z, alpha0), v0.color, Lerp(v0.u, v2.u, alpha0), Lerp(v0.v, v2.v, alpha0) };
	UIRender::CUSTOMVERTEX v4 = { x, Lerp(v1.y, v2.y, alpha1), Lerp(v1.z, v2.z, alpha1), v1.color, Lerp(v1.u, v2.u, alpha1), Lerp(v1.v, v2.v, alpha1) };
	UIRender::PushTriangleSimple(vertex_list, v3, v2, v4, clip, clipmask);
}

static void _PushTriangleClipY_1_2(UIRender::VertexList & vertex_list, const UIRender::CUSTOMVERTEX & v0, const UIRender::CUSTOMVERTEX & v1, const UIRender::CUSTOMVERTEX & v2, float y, const my::Rectangle & clip, DWORD clipmask)
{
	float alpha0 = (y - v0.y) / (v1.y - v0.y);
	float alpha1 = (y - v0.y) / (v2.y - v0.y);
	UIRender::CUSTOMVERTEX v3 = { Lerp(v0.x, v1.x, alpha0), y, Lerp(v0.z, v1.z, alpha0), v0.color, Lerp(v0.u, v1.u, alpha0), Lerp(v0.v, v1.v, alpha0) };
	UIRender::CUSTOMVERTEX v4 = { Lerp(v0.x, v2.x, alpha1), y, Lerp(v0.z, v2.z, alpha1), v0.color, Lerp(v0.u, v2.u, alpha1), Lerp(v0.v, v2.v, alpha1) };
	UIRender::PushTriangleSimple(vertex_list, v3, v1, v2, clip, clipmask);
	UIRender::PushTriangleSimple(vertex_list, v3, v2, v4, clip, clipmask);
}

static void _PushTriangleClipY_2_1(UIRender::VertexList & vertex_list, const UIRender::CUSTOMVERTEX & v0, const UIRender::CUSTOMVERTEX & v1, const UIRender::CUSTOMVERTEX & v2, float y, const my::Rectangle & clip, DWORD clipmask)
{
	float alpha0 = (y - v0.y) / (v2.y - v0.y);
	float alpha1 = (y - v1.y) / (v2.y - v1.y);
	UIRender::CUSTOMVERTEX v3 = { Lerp(v0.x, v2.x, alpha0), y, Lerp(v0.z, v2.z, alpha0), v0.color, Lerp(v0.u, v2.u, alpha0), Lerp(v0.v, v2.v, alpha0) };
	UIRender::CUSTOMVERTEX v4 = { Lerp(v1.x, v2.x, alpha1), y, Lerp(v1.z, v2.z, alpha1), v1.color, Lerp(v1.u, v2.u, alpha1), Lerp(v1.v, v2.v, alpha1) };
	UIRender::PushTriangleSimple(vertex_list, v3, v2, v4, clip, clipmask);
}

void UIRender::PushTriangleSimple(VertexList & vertex_list, const CUSTOMVERTEX & v0, const CUSTOMVERTEX & v1, const CUSTOMVERTEX & v2, const Rectangle & clip, DWORD clipmask)
{
	if (clipmask & ClipLeft)
	{
		if (v0.x < clip.l)
		{
			if (v1.x < clip.l)
			{
				if (v2.x < clip.l)
				{
					return;
				}
				else
				{
					_PushTriangleClipX_2_1(vertex_list, v0, v1, v2, clip.l, clip, clipmask & ~ClipLeft);
				}
			}
			else
			{
				if (v2.x < clip.l)
				{
					_PushTriangleClipX_2_1(vertex_list, v2, v0, v1, clip.l, clip, clipmask & ~ClipLeft);
				}
				else
				{
					_PushTriangleClipX_1_2(vertex_list, v0, v1, v2, clip.l, clip, clipmask & ~ClipLeft);
				}
			}
		}
		else
		{
			if (v1.x < clip.l)
			{
				if (v2.x < clip.l)
				{
					_PushTriangleClipX_2_1(vertex_list, v1, v2, v0, clip.l, clip, clipmask & ~ClipLeft);
				}
				else
				{
					_PushTriangleClipX_1_2(vertex_list, v1, v2, v0, clip.l, clip, clipmask & ~ClipLeft);
				}
			}
			else
			{
				if (v2.x < clip.l)
				{
					_PushTriangleClipX_1_2(vertex_list, v2, v0, v1, clip.l, clip, clipmask & ~ClipLeft);
				}
				else
				{
					PushTriangleSimple(vertex_list, v0, v1, v2, clip, clipmask & ~ClipLeft);
				}
			}
		}
	}
	else if (clipmask & ClipTop)
	{
		if (v0.y < clip.t)
		{
			if (v1.y < clip.t)
			{
				if (v2.y < clip.t)
				{
					return;
				}
				else
				{
					_PushTriangleClipY_2_1(vertex_list, v0, v1, v2, clip.t, clip, clipmask & ~ClipTop);
				}
			}
			else
			{
				if (v2.y < clip.t)
				{
					_PushTriangleClipY_2_1(vertex_list, v2, v0, v1, clip.t, clip, clipmask & ~ClipTop);
				}
				else
				{
					_PushTriangleClipY_1_2(vertex_list, v0, v1, v2, clip.t, clip, clipmask & ~ClipTop);
				}
			}
		}
		else
		{
			if (v1.y < clip.t)
			{
				if (v2.y < clip.t)
				{
					_PushTriangleClipY_2_1(vertex_list, v1, v2, v0, clip.t, clip, clipmask & ~ClipTop);
				}
				else
				{
					_PushTriangleClipY_1_2(vertex_list, v1, v2, v0, clip.t, clip, clipmask & ~ClipTop);
				}
			}
			else
			{
				if (v2.y < clip.t)
				{
					_PushTriangleClipY_1_2(vertex_list, v2, v0, v1, clip.t, clip, clipmask & ~ClipTop);
				}
				else
				{
					PushTriangleSimple(vertex_list, v0, v1, v2, clip, clipmask & ~ClipTop);
				}
			}
		}
	}
	else if (clipmask & ClipRight)
	{
		if (v0.x > clip.r)
		{
			if (v1.x > clip.r)
			{
				if (v2.x > clip.r)
				{
					return;
				}
				else
				{
					_PushTriangleClipX_2_1(vertex_list, v0, v1, v2, clip.r, clip, clipmask & ~ClipRight);
				}
			}
			else
			{
				if (v2.x > clip.r)
				{
					_PushTriangleClipX_2_1(vertex_list, v2, v0, v1, clip.r, clip, clipmask & ~ClipRight);
				}
				else
				{
					_PushTriangleClipX_1_2(vertex_list, v0, v1, v2, clip.r, clip, clipmask & ~ClipRight);
				}
			}
		}
		else
		{
			if (v1.x > clip.r)
			{
				if (v2.x > clip.r)
				{
					_PushTriangleClipX_2_1(vertex_list, v1, v2, v0, clip.r, clip, clipmask & ~ClipRight);
				}
				else
				{
					_PushTriangleClipX_1_2(vertex_list, v1, v2, v0, clip.r, clip, clipmask & ~ClipRight);
				}
			}
			else
			{
				if (v2.x > clip.r)
				{
					_PushTriangleClipX_1_2(vertex_list, v2, v0, v1, clip.r, clip, clipmask & ~ClipRight);
				}
				else
				{
					PushTriangleSimple(vertex_list, v0, v1, v2, clip, clipmask & ~ClipRight);
				}
			}
		}
	}
	else if (clipmask & ClipBottom)
	{
		if (v0.y > clip.b)
		{
			if (v1.y > clip.b)
			{
				if (v2.y > clip.b)
				{
					return;
				}
				else
				{
					_PushTriangleClipY_2_1(vertex_list, v0, v1, v2, clip.b, clip, clipmask & ~ClipBottom);
				}
			}
			else
			{
				if (v2.y > clip.b)
				{
					_PushTriangleClipY_2_1(vertex_list, v2, v0, v1, clip.b, clip, clipmask & ~ClipBottom);
				}
				else
				{
					_PushTriangleClipY_1_2(vertex_list, v0, v1, v2, clip.b, clip, clipmask & ~ClipBottom);
				}
			}
		}
		else
		{
			if (v1.y > clip.b)
			{
				if (v2.y > clip.b)
				{
					_PushTriangleClipY_2_1(vertex_list, v1, v2, v0, clip.b, clip, clipmask & ~ClipBottom);
				}
				else
				{
					_PushTriangleClipY_1_2(vertex_list, v1, v2, v0, clip.b, clip, clipmask & ~ClipBottom);
				}
			}
			else
			{
				if (v2.y > clip.b)
				{
					_PushTriangleClipY_1_2(vertex_list, v2, v0, v1, clip.b, clip, clipmask & ~ClipBottom);
				}
				else
				{
					PushTriangleSimple(vertex_list, v0, v1, v2, clip, clipmask & ~ClipBottom);
				}
			}
		}
	}
	else
		PushTriangleSimple(vertex_list, v0, v1, v2);
}

void UIRender::PushRectangleSimple(VertexList & vertex_list, const my::Rectangle & rect, const my::Rectangle & UvRect, D3DCOLOR color)
{
	PushTriangleSimple(vertex_list,
		CUSTOMVERTEX(rect.l, rect.t, 0, color, UvRect.l, UvRect.t),
		CUSTOMVERTEX(rect.r, rect.t, 0, color, UvRect.r, UvRect.t),
		CUSTOMVERTEX(rect.l, rect.b, 0, color, UvRect.l, UvRect.b));

	PushTriangleSimple(vertex_list,
		CUSTOMVERTEX(rect.r, rect.b, 0, color, UvRect.r, UvRect.b),
		CUSTOMVERTEX(rect.l, rect.b, 0, color, UvRect.l, UvRect.b),
		CUSTOMVERTEX(rect.r, rect.t, 0, color, UvRect.r, UvRect.t));
}

void UIRender::PushRectangleSimple(VertexList & vertex_list, const my::Rectangle & rect, const my::Rectangle & UvRect, D3DCOLOR color, const Rectangle & clip)
{
	DWORD clipmask = (rect.l < clip.l ? ClipLeft : 0) | (rect.t < clip.t ? ClipTop : 0) | (rect.r > clip.r ? ClipRight : 0) | (rect.b > clip.b ? ClipBottom : 0);

	PushTriangleSimple(vertex_list,
		CUSTOMVERTEX(rect.l, rect.t, 0, color, UvRect.l, UvRect.t),
		CUSTOMVERTEX(rect.r, rect.t, 0, color, UvRect.r, UvRect.t),
		CUSTOMVERTEX(rect.l, rect.b, 0, color, UvRect.l, UvRect.b), clip, clipmask);

	PushTriangleSimple(vertex_list,
		CUSTOMVERTEX(rect.r, rect.b, 0, color, UvRect.r, UvRect.b),
		CUSTOMVERTEX(rect.l, rect.b, 0, color, UvRect.l, UvRect.b),
		CUSTOMVERTEX(rect.r, rect.t, 0, color, UvRect.r, UvRect.t), clip, clipmask);
}

UIRender::VertexList & UIRender::GetVertexList(const BaseTexture * texture)
{
	_ASSERT(texture);

	UILayerList::iterator layer_iter = boost::find_if(m_Layer, boost::bind(std::equal_to<const BaseTexture*>(), boost::bind(&UILayer::first, boost::placeholders::_1), texture));
	if (layer_iter != m_Layer.end())
	{
		return layer_iter->second;
	}

	m_Layer.push_back(UILayer(texture, VertexList()));
	return m_Layer.back().second;
}

void UIRender::PushRectangle(const my::Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle UvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	PushRectangleSimple(GetVertexList(texture), rect, UvRect, color);
}

void UIRender::PushRectangle(const my::Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Rectangle & clip)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle UvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	PushRectangleSimple(GetVertexList(texture), rect, UvRect, color, clip);
}

void UIRender::PushRectangle(const my::Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Matrix4 & transform)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle UvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	const CUSTOMVERTEX v[] = {
		{ rect.l, rect.t, 0, color, UvRect.l, UvRect.t },
		{ rect.r, rect.t, 0, color, UvRect.r, UvRect.t },
		{ rect.r, rect.b, 0, color, UvRect.r, UvRect.b },
		{ rect.l, rect.b, 0, color, UvRect.l, UvRect.b },
	};
	D3DXVec3TransformCoordArray((D3DXVECTOR3*)v, sizeof(v[0]), (D3DXVECTOR3*)v, sizeof(v[0]), (D3DXMATRIX*)&transform, _countof(v));
	PushTriangleSimple(GetVertexList(texture), v[0], v[1], v[3]);
	PushTriangleSimple(GetVertexList(texture), v[2], v[3], v[1]);
}

void UIRender::PushRectangle(const my::Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Matrix4 & transform, const Rectangle & clip)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle UvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	const CUSTOMVERTEX v[] = {
		{ rect.l, rect.t, 0, color, UvRect.l, UvRect.t },
		{ rect.r, rect.t, 0, color, UvRect.r, UvRect.t },
		{ rect.r, rect.b, 0, color, UvRect.r, UvRect.b },
		{ rect.l, rect.b, 0, color, UvRect.l, UvRect.b },
	};
	D3DXVec3TransformCoordArray((D3DXVECTOR3*)v, sizeof(v[0]), (D3DXVECTOR3*)v, sizeof(v[0]), (D3DXMATRIX*)&transform, _countof(v));
	PushTriangleSimple(GetVertexList(texture), v[0], v[1], v[3], clip, 0xFF);
	PushTriangleSimple(GetVertexList(texture), v[2], v[3], v[1], clip, 0xFF);
}

void UIRender::PushWindowSimple(VertexList & vertex_list, const my::Rectangle & rect, const Rectangle & InRect, const Rectangle & OutUvRect, const Rectangle & InUvRect, DWORD color)
{
	PushRectangleSimple(vertex_list, Rectangle(rect.l, rect.t, InRect.l, InRect.t), Rectangle(OutUvRect.l, OutUvRect.t, InUvRect.l, InUvRect.t), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, rect.t, InRect.r, InRect.t), Rectangle(InUvRect.l, OutUvRect.t, InUvRect.r, InUvRect.t), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, rect.t, rect.r, InRect.t), Rectangle(InUvRect.r, OutUvRect.t, OutUvRect.r, InUvRect.t), color);
	PushRectangleSimple(vertex_list, Rectangle(rect.l, InRect.t, InRect.l, InRect.b), Rectangle(OutUvRect.l, InUvRect.t, InUvRect.l, InUvRect.b), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, InRect.t, InRect.r, InRect.b), Rectangle(InUvRect.l, InUvRect.t, InUvRect.r, InUvRect.b), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, InRect.t, rect.r, InRect.b), Rectangle(InUvRect.r, InUvRect.t, OutUvRect.r, InUvRect.b), color);
	PushRectangleSimple(vertex_list, Rectangle(rect.l, InRect.b, InRect.l, rect.b), Rectangle(OutUvRect.l, InUvRect.b, InUvRect.l, OutUvRect.b), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, InRect.b, InRect.r, rect.b), Rectangle(InUvRect.l, InUvRect.b, InUvRect.r, OutUvRect.b), color);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, InRect.b, rect.r, rect.b), Rectangle(InUvRect.r, InUvRect.b, OutUvRect.r, OutUvRect.b), color);
}

void UIRender::PushWindowSimple(VertexList & vertex_list, const my::Rectangle & rect, const Rectangle & InRect, const Rectangle & OutUvRect, const Rectangle & InUvRect, DWORD color, const Rectangle & clip)
{
	PushRectangleSimple(vertex_list, Rectangle(rect.l, rect.t, InRect.l, InRect.t), Rectangle(OutUvRect.l, OutUvRect.t, InUvRect.l, InUvRect.t), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, rect.t, InRect.r, InRect.t), Rectangle(InUvRect.l, OutUvRect.t, InUvRect.r, InUvRect.t), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, rect.t, rect.r, InRect.t), Rectangle(InUvRect.r, OutUvRect.t, OutUvRect.r, InUvRect.t), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(rect.l, InRect.t, InRect.l, InRect.b), Rectangle(OutUvRect.l, InUvRect.t, InUvRect.l, InUvRect.b), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, InRect.t, InRect.r, InRect.b), Rectangle(InUvRect.l, InUvRect.t, InUvRect.r, InUvRect.b), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, InRect.t, rect.r, InRect.b), Rectangle(InUvRect.r, InUvRect.t, OutUvRect.r, InUvRect.b), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(rect.l, InRect.b, InRect.l, rect.b), Rectangle(OutUvRect.l, InUvRect.b, InUvRect.l, OutUvRect.b), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.l, InRect.b, InRect.r, rect.b), Rectangle(InUvRect.l, InUvRect.b, InUvRect.r, OutUvRect.b), color, clip);
	PushRectangleSimple(vertex_list, Rectangle(InRect.r, InRect.b, rect.r, rect.b), Rectangle(InUvRect.r, InUvRect.b, OutUvRect.r, OutUvRect.b), color, clip);
}

void UIRender::PushWindow(const my::Rectangle & rect, DWORD color, const my::Rectangle & WindowRect, const Vector4 & WindowBorder, const BaseTexture * texture)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle InRect(rect.l + WindowBorder.x, rect.t + WindowBorder.y, rect.r - WindowBorder.z, rect.b - WindowBorder.w);
	Rectangle OutUvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	Rectangle InUvRect((WindowRect.l + WindowBorder.x) / desc.Width, (WindowRect.t + WindowBorder.y) / desc.Height, (WindowRect.r - WindowBorder.z) / desc.Width, (WindowRect.b - WindowBorder.w) / desc.Height);
	PushWindowSimple(GetVertexList(texture), rect, InRect, OutUvRect, InUvRect, color);
}

void UIRender::PushWindow(const my::Rectangle & rect, DWORD color, const my::Rectangle & WindowRect, const Vector4 & WindowBorder, const BaseTexture * texture, const Rectangle & clip)
{
	D3DSURFACE_DESC desc = texture->GetLevelDesc();
	Rectangle InRect(rect.l + WindowBorder.x, rect.t + WindowBorder.y, rect.r - WindowBorder.z, rect.b - WindowBorder.w);
	Rectangle OutUvRect(WindowRect.l / desc.Width, WindowRect.t / desc.Height, WindowRect.r / desc.Width, WindowRect.b / desc.Height);
	Rectangle InUvRect((WindowRect.l + WindowBorder.x) / desc.Width, (WindowRect.t + WindowBorder.y) / desc.Height, (WindowRect.r - WindowBorder.z) / desc.Width, (WindowRect.b - WindowBorder.w) / desc.Height);
	PushWindowSimple(GetVertexList(texture), rect, InRect, OutUvRect, InUvRect, color, clip);
}

void UIRender::PushCharacter(float x, float y, const Font::CharacterInfo* info, Font* font, D3DCOLOR color)
{
	Rectangle UvRect(
		(float)info->textureRect.left / font->m_textureDesc.Width,
		(float)info->textureRect.top / font->m_textureDesc.Height,
		(float)info->textureRect.right / font->m_textureDesc.Width,
		(float)info->textureRect.bottom / font->m_textureDesc.Height);

	PushRectangleSimple(GetVertexList(font->m_Texture.get()),
		Rectangle::LeftTop(x + info->horiBearingX, y - info->horiBearingY, info->width, info->height), UvRect, color);
}

void UIRender::PushCharacter(float x, float y, const Font::CharacterInfo* info, Font* font, D3DCOLOR color, const Matrix4& transform)
{
	const CUSTOMVERTEX v[] = {
		{ x + info->horiBearingX, y - info->horiBearingY, 0, color, (float)info->textureRect.left / font->m_textureDesc.Width, (float)info->textureRect.top / font->m_textureDesc.Height },
		{ x + info->horiBearingX + info->width, y - info->horiBearingY, 0, color, (float)info->textureRect.right / font->m_textureDesc.Width, (float)info->textureRect.top / font->m_textureDesc.Height },
		{ x + info->horiBearingX + info->width, y - info->horiBearingY + info->height, 0, color, (float)info->textureRect.right / font->m_textureDesc.Width, (float)info->textureRect.bottom / font->m_textureDesc.Height },
		{ x + info->horiBearingX, y - info->horiBearingY + info->height, 0, color, (float)info->textureRect.left / font->m_textureDesc.Width, (float)info->textureRect.bottom / font->m_textureDesc.Height },
	};

	D3DXVec3TransformCoordArray((D3DXVECTOR3*)v, sizeof(v[0]), (D3DXVECTOR3*)v, sizeof(v[0]), (D3DXMATRIX*)&transform, _countof(v));

	PushTriangleSimple(GetVertexList(font->m_Texture.get()), v[0], v[1], v[3]);
	PushTriangleSimple(GetVertexList(font->m_Texture.get()), v[2], v[3], v[1]);
}

void UIRender::PushString(const my::Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, Font * font)
{
	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterInfo, font, boost::placeholders::_1),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, color));
}

void UIRender::PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, D3DCOLOR outlineColor, float outlineWidth, Font * font)
{
	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterOutlineInfo, font, boost::placeholders::_1, outlineWidth),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, outlineColor));

	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterInfo, font, boost::placeholders::_1),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, color));
}

void UIRender::PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, Font * font, const Matrix4 & transform)
{
	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterInfo, font, boost::placeholders::_1),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, color, boost::ref(transform)));
}

void UIRender::PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, D3DCOLOR outlineColor, float outlineWidth, Font * font, const Matrix4 & transform)
{
	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterOutlineInfo, font, boost::placeholders::_1, outlineWidth),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, outlineColor, boost::ref(transform)));

	font->DrawString(rect, str, align, boost::bind(&Font::GetCharacterInfo, font, boost::placeholders::_1),
		boost::bind(&UIRender::PushCharacter, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, font, color, boost::ref(transform)));
}

ControlImage::~ControlImage(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void ControlImage::RequestResource(void)
{
	m_Requested = true;

	if (!m_TexturePath.empty())
	{
		_ASSERT(!m_Texture);

		my::ResourceMgr::getSingleton().LoadTextureAsync(m_TexturePath.c_str(), boost::bind(&ControlImage::OnTextureReady, this, boost::placeholders::_1), INT_MAX);
	}
}

void ControlImage::ReleaseResource(void)
{
	m_Requested = false;

	if (!m_TexturePath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_TexturePath.c_str(), boost::bind(&ControlImage::OnTextureReady, this, boost::placeholders::_1));

		m_Texture.reset();
	}
}

void ControlImage::OnTextureReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<BaseTexture>(res);
}

ControlImagePtr ControlImage::Clone(void) const
{
	ControlImagePtr ret(new ControlImage());
	ret->m_TexturePath = m_TexturePath;
	ret->m_Rect = m_Rect;
	ret->m_Border = m_Border;
	return ret;
}

void ControlImage::Draw(UIRender * ui_render, const Rectangle & rect, DWORD color)
{
	if (m_Texture)
	{
		if (m_Border.x == 0 && m_Border.y == 0 && m_Border.z == 0 && m_Border.w == 0)
		{
			ui_render->PushRectangle(rect, color, m_Rect, m_Texture.get());
		}
		else
		{
			ui_render->PushWindow(rect, color, m_Rect, m_Border, m_Texture.get());
		}
	}
}

void ControlImage::Draw(UIRender * ui_render, const Rectangle & rect, DWORD color, const Rectangle & clip)
{
	if (m_Texture)
	{
		if (m_Border.x == 0 && m_Border.y == 0 && m_Border.z == 0 && m_Border.w == 0)
		{
			ui_render->PushRectangle(rect, color, m_Rect, m_Texture.get(), clip);
		}
		else
		{
			ui_render->PushWindow(rect, color, m_Rect, m_Border, m_Texture.get(), clip);
		}
	}
}

ControlSkin::ControlSkin(void)
	: m_Color(D3DCOLOR_ARGB(255, 255, 255, 255))
	, m_Requested(false)
{
}

ControlSkin::~ControlSkin(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void ControlSkin::RequestResource(void)
{
	m_Requested = true;

	if (m_Image)
	{
		m_Image->RequestResource();
	}

	if (!m_VisibleShowSoundPath.empty())
	{
		_ASSERT(!m_VisibleShowSound);

		my::ResourceMgr::getSingleton().LoadWavAsync(m_VisibleShowSoundPath.c_str(), boost::bind(&ControlSkin::OnVisibleShowSoundReady, this, boost::placeholders::_1), 0);
	}

	if (!m_VisibleHideSoundPath.empty())
	{
		_ASSERT(!m_VisibleHideSound);

		my::ResourceMgr::getSingleton().LoadWavAsync(m_VisibleHideSoundPath.c_str(), boost::bind(&ControlSkin::OnVisibleHideSoundReady, this, boost::placeholders::_1), 0);
	}

	if (!m_MouseEnterSoundPath.empty())
	{
		_ASSERT(!m_MouseEnterSound);

		my::ResourceMgr::getSingleton().LoadWavAsync(m_MouseEnterSoundPath.c_str(), boost::bind(&ControlSkin::OnMouseEnterSoundReady, this, boost::placeholders::_1), 0);
	}

	if (!m_MouseLeaveSoundPath.empty())
	{
		_ASSERT(!m_MouseLeaveSound);

		my::ResourceMgr::getSingleton().LoadWavAsync(m_MouseLeaveSoundPath.c_str(), boost::bind(&ControlSkin::OnMouseLeaveSoundReady, this, boost::placeholders::_1), 0);
	}

	if (!m_MouseClickSoundPath.empty())
	{
		_ASSERT(!m_MouseClickSound);

		my::ResourceMgr::getSingleton().LoadWavAsync(m_MouseClickSoundPath.c_str(), boost::bind(&ControlSkin::OnMouseClickSoundReady, this, boost::placeholders::_1), 0);
	}
}

void ControlSkin::ReleaseResource(void)
{
	m_Requested = false;

	if (m_Image)
	{
		m_Image->ReleaseResource();
	}

	if (!m_VisibleShowSoundPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_VisibleShowSoundPath, boost::bind(&ControlSkin::OnVisibleShowSoundReady, this, boost::placeholders::_1));

		m_VisibleShowSound.reset();
	}

	if (!m_VisibleHideSoundPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_VisibleHideSoundPath, boost::bind(&ControlSkin::OnVisibleHideSoundReady, this, boost::placeholders::_1));

		m_VisibleHideSound.reset();
	}

	if (!m_MouseEnterSoundPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MouseEnterSoundPath, boost::bind(&ControlSkin::OnMouseEnterSoundReady, this, boost::placeholders::_1));

		m_MouseEnterSound.reset();
	}

	if (!m_MouseLeaveSoundPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MouseLeaveSoundPath, boost::bind(&ControlSkin::OnMouseLeaveSoundReady, this, boost::placeholders::_1));

		m_MouseLeaveSound.reset();
	}

	if (!m_MouseClickSoundPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MouseClickSoundPath, boost::bind(&ControlSkin::OnMouseClickSoundReady, this, boost::placeholders::_1));

		m_MouseClickSound.reset();
	}
}

void ControlSkin::OnVisibleShowSoundReady(my::DeviceResourceBasePtr res)
{
	m_VisibleShowSound = boost::dynamic_pointer_cast<Wav>(res);
}

void ControlSkin::OnVisibleHideSoundReady(my::DeviceResourceBasePtr res)
{
	m_VisibleHideSound = boost::dynamic_pointer_cast<Wav>(res);
}

void ControlSkin::OnMouseEnterSoundReady(my::DeviceResourceBasePtr res)
{
	m_MouseEnterSound = boost::dynamic_pointer_cast<Wav>(res);
}

void ControlSkin::OnMouseLeaveSoundReady(my::DeviceResourceBasePtr res)
{
	m_MouseLeaveSound = boost::dynamic_pointer_cast<Wav>(res);
}

void ControlSkin::OnMouseClickSoundReady(my::DeviceResourceBasePtr res)
{
	m_MouseClickSound = boost::dynamic_pointer_cast<Wav>(res);
}

void ControlSkin::DrawImage(UIRender * ui_render, const ControlImagePtr & Image, const my::Rectangle & rect, DWORD color)
{
	if(Image)
	{
		Image->Draw(ui_render, rect, color);
	}
}

void ControlSkin::DrawImage(UIRender * ui_render, const ControlImagePtr & Image, const Rectangle & rect, DWORD color, const Rectangle & clip)
{
	if (Image)
	{
		Image->Draw(ui_render, rect, color, clip);
	}
}

ControlSkinPtr ControlSkin::Clone(void) const
{
	std::stringstream sstr;
	boost::archive::text_oarchive oa(sstr);
	oa << boost::serialization::make_nvp(__FUNCTION__, shared_from_this());

	ControlSkinPtr ret(new ControlSkin());
	boost::archive::text_iarchive ia(sstr);
	ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
	return ret;
}

Control * Control::s_FocusControl = NULL;

Control * Control::s_CaptureControl = NULL;

Control * Control::s_MouseOverControl = NULL;

Control::~Control(void)
{
	// ! must clear global reference
	if (ContainsControl(s_FocusControl))
	{
		SetFocusControl(NULL);
	}

	if (ContainsControl(s_CaptureControl))
	{
		SetCaptureControl(NULL);
	}

	if (ContainsControl(s_MouseOverControl))
	{
		SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
	}

	// ! must detach parent relationship
	ClearAllControl();

	if (m_Parent)
	{
		_ASSERT(false); //m_Parent->RemoveControl(shared_from_this());
	}
}

template<class Archive>
void Control::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
	ar << BOOST_SERIALIZATION_NVP(m_bEnabled);
	ar << BOOST_SERIALIZATION_NVP(m_bVisible);
	ar << BOOST_SERIALIZATION_NVP(m_nHotkey);
	ar << BOOST_SERIALIZATION_NVP(m_x);
	ar << BOOST_SERIALIZATION_NVP(m_y);
	ar << BOOST_SERIALIZATION_NVP(m_Width);
	ar << BOOST_SERIALIZATION_NVP(m_Height);
	ar << BOOST_SERIALIZATION_NVP(m_Skin);
}

template<class Archive>
void Control::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	ar >> BOOST_SERIALIZATION_NVP(m_bEnabled);
	ar >> BOOST_SERIALIZATION_NVP(m_bVisible);
	ar >> BOOST_SERIALIZATION_NVP(m_nHotkey);
	ar >> BOOST_SERIALIZATION_NVP(m_x);
	ar >> BOOST_SERIALIZATION_NVP(m_y);
	ar >> BOOST_SERIALIZATION_NVP(m_Width);
	ar >> BOOST_SERIALIZATION_NVP(m_Height);
	ar >> BOOST_SERIALIZATION_NVP(m_Skin);

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_Parent = this;
	}
}

void Control::RequestResource(void)
{
	m_Requested = true;

	if (m_Skin)
	{
		_ASSERT(!m_Skin->IsRequested());

		m_Skin->RequestResource();
	}

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		_ASSERT(!(*ctrl_iter)->IsRequested());

		(*ctrl_iter)->RequestResource();
	}
}

void Control::ReleaseResource(void)
{
	m_Requested = false;

	if (m_Skin)
	{
		_ASSERT(m_Skin->IsRequested());

		m_Skin->ReleaseResource();
	}

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		_ASSERT((*ctrl_iter)->IsRequested());

		(*ctrl_iter)->ReleaseResource();
	}
}

void Control::SetFocusControl(Control * control)
{
	if (s_FocusControl != control)
	{
		Control* focusParent = NULL;

		if (s_FocusControl)
		{
			if (control)
			{
				if (s_FocusControl->m_Parent && s_FocusControl->m_Parent->ContainsControl(control))
				{
					focusParent = s_FocusControl->m_Parent;
				}
				else if (control->m_Parent && control->m_Parent->ContainsControl(s_FocusControl))
				{
					focusParent = control->m_Parent;
				}
			}
		}

		Control * old_control = s_FocusControl;

		s_FocusControl = control;

		if (old_control)
		{
			Control* ctrl = old_control;
			for (; ctrl != focusParent; ctrl = ctrl->m_Parent)
			{
				ctrl->OnFocusOut();
			}
		}

		if (s_FocusControl)
		{
			Control* ctrl = s_FocusControl;
			for (; ctrl != focusParent; ctrl = ctrl->m_Parent)
			{
				ctrl->OnFocusIn();
			}
		}

		D3DContext::getSingleton().OnControlFocus(s_FocusControl);
	}
}

void Control::SetCaptureControl(Control * control)
{
	if (s_CaptureControl != control)
	{
		s_CaptureControl = control;
	}
}

void Control::SetMouseOverControl(Control * control, const Vector2 & pt)
{
	if (s_MouseOverControl != control)
	{
		if (s_MouseOverControl)
		{
			s_MouseOverControl->OnMouseLeave(pt);
		}

		s_MouseOverControl = control;

		if (control)
		{
			control->OnMouseEnter(pt);
		}
	}
}

void Control::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if (m_Skin)
		{
			m_Skin->DrawImage(ui_render, m_Skin->m_Image, m_Rect, m_Skin->m_Color);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool Control::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Control::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				if (m_Parent)
				{
					Control* nearest_ctrl = NULL;
					float nearest_ctrl_dist = FLT_MAX;
					m_Parent->GetNearestControl(m_Rect, wParam, &nearest_ctrl, nearest_ctrl_dist, this);
					if (nearest_ctrl)
					{
						SetFocusControl(nearest_ctrl);

						SetMouseOverControl(s_FocusControl, s_FocusControl->m_Rect.lt);
						return true;
					}
				}
				break;

			case VK_RETURN:
				m_bPressed = true;
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_KEYUP:
			switch (wParam)
			{
			case VK_RETURN:
				if (m_bPressed)
				{
					m_bPressed = false;
					SetCaptureControl(NULL);

					if (m_Skin && m_Skin->m_MouseClickSound)
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound);
					}

					if (m_EventMouseClick)
					{
						MouseEventArg arg(this, Vector2(0, 0));
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;
		}
	}
	return false;
}

bool Control::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (HitTest(pt))
			{
				m_bPressed = true;
				if (CanHaveFocus())
				{
					SetFocusControl(this);
				}
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				SetCaptureControl(NULL);
				m_bPressed = false;

				if (m_Skin && m_Skin->m_MouseClickSound)
				{
					D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound);
				}

				if (m_EventMouseClick)
				{
					MouseEventArg arg(this, pt);
					m_EventMouseClick(&arg);
				}
				return true;
			}
			break;
		}
	}
	return false;
}

bool Control::CanHaveFocus(void) const
{
	return false;
}

void Control::OnFocusIn(void)
{
	// ! fix ListBox scroll pos
	if (m_Parent && m_Parent->GetControlType() == ControlTypeListBox)
	{
		ListBox* listBox = dynamic_cast<ListBox*>(m_Parent);
		_ASSERT(listBox);
		int i = 0;
		for (; i < listBox->m_ScrollBar->m_nEnd; i++)
		{
			const int column = Min(listBox->m_ItemColumn, (int)m_Parent->m_Childs.size() - i * listBox->m_ItemColumn);
			for (int j = 0; j < column; j++)
			{
				ControlPtr& ctrl = m_Parent->m_Childs[i * listBox->m_ItemColumn + j];
				if (ctrl->GetVisible() && ctrl.get() == this)
				{
					goto listbox_loop_end;
				}
			}
		}
listbox_loop_end:
		listBox->m_ScrollBar->ScrollTo(i);
	}

	if (m_EventFocusChanged)
	{
		FocusEventArg arg(this, true);
		m_EventFocusChanged(&arg);
	}
}

void Control::OnFocusOut(void)
{
	if (m_EventFocusChanged)
	{
		FocusEventArg arg(this, false);
		m_EventFocusChanged(&arg);
	}
}

void Control::OnMouseEnter(const Vector2 & pt)
{
	if(m_bEnabled && m_bVisible)
	{
		if (m_Skin && m_Skin->m_MouseEnterSound)
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseEnterSound);
		}

		if (m_EventMouseEnter)
		{
			MouseEventArg arg(this, pt);
			m_EventMouseEnter(&arg);
		}
	}
}

void Control::OnMouseLeave(const Vector2 & pt)
{
	if(m_bEnabled && m_bVisible)
	{
		if (m_EventMouseLeave)
		{
			MouseEventArg arg(this, pt);
			m_EventMouseLeave(&arg);
		}

		if (m_Skin && m_Skin->m_MouseLeaveSound)
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseLeaveSound);
		}
	}
}

void Control::OnHotkey(void)
{
	if (m_bEnabled && m_bVisible)
	{
		if (m_Skin && m_Skin->m_MouseClickSound)
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound);
		}

		if (m_EventMouseClick)
		{
			MouseEventArg arg(this, Vector2(0, 0));
			m_EventMouseClick(&arg);
		}
	}
}

bool Control::HitTest(const Vector2 & pt) const
{
	return m_Rect.PtInRect(pt);
}

void Control::OnLayout(void)
{
}

void Control::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
}

bool Control::GetEnabled(void) const
{
	return m_bEnabled;
}

bool Control::GetEnabledHierarchy(void) const
{
	return GetEnabled() && (!m_Parent || m_Parent->GetEnabledHierarchy());
}

void Control::SetVisible(bool bVisible)
{
	if (m_bVisible != bVisible)
	{
		m_bVisible = bVisible;

		if (m_bVisible)
		{
			if (m_Skin && m_Skin->m_VisibleShowSound)
			{
				D3DContext::getSingleton().OnControlSound(m_Skin->m_VisibleShowSound);
			}
		}
		else
		{
			if (m_Skin && m_Skin->m_VisibleHideSound)
			{
				D3DContext::getSingleton().OnControlSound(m_Skin->m_VisibleHideSound);
			}
		}

		if (m_EventVisibleChanged)
		{
			VisibleEventArg arg(this, bVisible);
			m_EventVisibleChanged(&arg);
		}

		if (!m_bVisible)
		{
			if (ContainsControl(s_FocusControl) && !GetTopControl()->SetFocusRecursive())
			{
				SetFocusControl(NULL);
			}

			if (ContainsControl(s_CaptureControl))
			{
				SetCaptureControl(NULL);
			}

			if (ContainsControl(s_MouseOverControl))
			{
				SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
			}
		}
	}
}

bool Control::GetVisible(void) const
{
	return m_bVisible;
}

bool Control::GetVisibleHierarchy(void) const
{
	return GetVisible() && (!m_Parent || m_Parent->GetVisibleHierarchy());
}

void Control::SetFocused(bool bFocused)
{
	if (bFocused)
	{
		if (!GetFocused())
		{
			Control::SetFocusRecursive();
		}
	}
	else
	{
		if (GetFocused())
		{
			Control::SetFocusControl(NULL);
		}
	}
}

bool Control::GetFocused(void) const
{
	return ContainsControl(s_FocusControl);
}

void Control::SetCaptured(bool bCaptured)
{
	if (bCaptured)
	{
		Control::SetCaptureControl(this);
	}
	else if (GetCaptured())
	{
		Control::SetCaptureControl(NULL);
	}
}

bool Control::GetCaptured(void) const
{
	return Control::GetCaptureControl() == this;
}

void Control::SetMouseOver(bool bMouseOver)
{
	if (bMouseOver)
	{
		Control::SetMouseOverControl(this, m_Rect.lt);
	}
	else if (GetMouseOver())
	{
		Control::SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
	}
}

bool Control::GetMouseOver(void) const
{
	return Control::GetMouseOverControl() == this;
}

bool Control::RayToWorld(const Ray & ray, Vector2 & ptWorld) const
{
	if (m_Parent)
	{
		return m_Parent->RayToWorld(ray, ptWorld);
	}
	return false;
}

void Control::InsertControl(unsigned int i, ControlPtr control)
{
	_ASSERT(!control->m_Parent);

	_ASSERT(i <= m_Childs.size());

	m_Childs.insert(m_Childs.begin() + i, control);

	control->m_Parent = this;

	if (IsRequested() && !control->IsRequested())
	{
		control->RequestResource();
	}
}

void Control::RemoveControl(unsigned int i)
{
	_ASSERT(i < m_Childs.size());

	ControlPtrList::iterator ctrl_iter = m_Childs.begin() + i;

	ControlPtr dummy_ctrl = *ctrl_iter;

	_ASSERT(dummy_ctrl->m_Parent == this);

	m_Childs.erase(ctrl_iter);

	if (dummy_ctrl->IsRequested())
	{
		dummy_ctrl->ReleaseResource();
	}

	dummy_ctrl->m_Parent = NULL;

	if (dummy_ctrl->ContainsControl(s_FocusControl) && !GetTopControl()->SetFocusRecursive())
	{
		SetFocusControl(NULL);
	}
}

unsigned int Control::GetChildNum(void) const
{
	return m_Childs.size();
}

unsigned int Control::GetSiblingId(void) const
{
	if (m_Parent)
	{
		ControlPtrList::iterator self_iter = boost::find_if(m_Parent->m_Childs, boost::bind(std::equal_to<const Control*>(), this, boost::bind(&ControlPtr::get, boost::placeholders::_1)));
		_ASSERT(self_iter != m_Parent->m_Childs.end());
		return std::distance(m_Parent->m_Childs.begin(), self_iter);
	}
	return 0;
}

void Control::SetSiblingId(unsigned int i)
{
	if (m_Parent && i < m_Parent->m_Childs.size())
	{
		int sibling_id = GetSiblingId();
		if (i < sibling_id)
		{
			std::rotate(m_Parent->m_Childs.rend() - sibling_id - 1, m_Parent->m_Childs.rend() - sibling_id, m_Parent->m_Childs.rend() - i);
		}
		else if (i > sibling_id)
		{
			std::rotate(m_Parent->m_Childs.begin() + sibling_id, m_Parent->m_Childs.begin() + sibling_id + 1, m_Parent->m_Childs.begin() + i + 1);
		}
	}
}

void Control::ClearAllControl(void)
{
	while (GetChildNum() > 0)
	{
		RemoveControl(GetChildNum() - 1);
	}
}

bool Control::ContainsControl(Control * control) const
{
	for (; control; control = control->m_Parent)
	{
		if (this == control)
		{
			return true;
		}
	}
	return false;
}

Control * Control::GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible)
{
	if (bIgnoreVisible || m_bEnabled && m_bVisible)
	{
		ControlPtrList::const_reverse_iterator ctrl_iter = m_Childs.rbegin();
		for (; ctrl_iter != m_Childs.rend(); ctrl_iter++)
		{
			Control * ctrl = (*ctrl_iter)->GetChildAtPoint(pt, bIgnoreVisible);
			if (ctrl)
			{
				return ctrl;
			}
		}

		if (HitTest(pt))
		{
			return this;
		}
	}
	return NULL;
}

int Control::GetChildAtFrustum(const my::Frustum & ftm, bool bIgnoreVisible, std::vector<Control *> & childs)
{
	if (bIgnoreVisible || m_bEnabled && m_bVisible)
	{
		int deep = 0;
		ControlPtrList::const_reverse_iterator ctrl_iter = m_Childs.rbegin();
		for (; ctrl_iter != m_Childs.rend(); ctrl_iter++)
		{
			deep = Max(deep, (*ctrl_iter)->GetChildAtFrustum(ftm, bIgnoreVisible, childs));
			if (deep > 1)
			{
				return deep + 1;
			}
		}

		if (deep > 0)
		{
			return deep + 1;
		}

		Dialog* dlg = GetTopControl();

		Vector3 v[] = {
			Vector3(m_Rect.l, m_Rect.t, 0).transformCoord(dlg->m_World),
			Vector3(m_Rect.r, m_Rect.t, 0).transformCoord(dlg->m_World),
			Vector3(m_Rect.r, m_Rect.b, 0).transformCoord(dlg->m_World),
			Vector3(m_Rect.l, m_Rect.b, 0).transformCoord(dlg->m_World) };

		if (IntersectionTests::IntersectionTypeOutside != IntersectionTests::IntersectTriangleAndFrustum(v[0], v[1], v[2], ftm)
			|| IntersectionTests::IntersectionTypeOutside != IntersectionTests::IntersectTriangleAndFrustum(v[0], v[2], v[3], ftm))
		{
			childs.push_back(this);
			return 1;
		}
	}
	return 0;
}

Dialog * Control::GetTopControl(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopControl();
	}

	_ASSERT(GetControlType() == ControlTypeDialog);

	return dynamic_cast<Dialog*>(this);
}

bool Control::SetFocusRecursive(void)
{
	if (m_bEnabled && m_bVisible)
	{
		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			if ((*ctrl_iter)->SetFocusRecursive())
			{
				return true;
			}
		}

		if (CanHaveFocus())
		{
			SetFocusControl(this);

			SetMouseOverControl(this, m_Rect.lt);
			return true;
		}
	}
	return false;
}

static float _GetNearestDist(const my::Rectangle & rcSrc, const my::Rectangle & rcDst, DWORD dir)
{
	float dist = FLT_MAX;
	switch (dir)
	{
	case VK_UP:
		if (rcDst.Center().y < rcSrc.t)
		{
			Vector2 diff = rcSrc.Center() - rcDst.Center();
			if (fabs(diff.x) > rcSrc.Width() * 0.5f)
			{
				diff.x *= 500.0f;
			}
			dist = diff.magnitudeSq();
		}
		break;
	case VK_DOWN:
		if (rcDst.Center().y > rcSrc.b)
		{
			Vector2 diff = rcSrc.Center() - rcDst.Center();
			if (fabs(diff.x) > rcSrc.Width() * 0.5f)
			{
				diff.x *= 500.0f;
			}
			dist = diff.magnitudeSq();
		}
		break;
	case VK_LEFT:
		if (rcDst.Center().x < rcSrc.l)
		{
			Vector2 diff = rcSrc.Center() - rcDst.Center();
			if (fabs(diff.y) > rcSrc.Height() * 0.5f)
			{
				diff.y *= 500.0f;
			}
			dist = diff.magnitudeSq();
		}
		break;
	case VK_RIGHT:
		if (rcDst.Center().x > rcSrc.r)
		{
			Vector2 diff = rcSrc.Center() - rcDst.Center();
			if (fabs(diff.y) > rcSrc.Height() * 0.5f)
			{
				diff.y *= 500.0f;
			}
			dist = diff.magnitudeSq();
		}
		break;
	}
	return dist;
}

void Control::GetNearestControl(const my::Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self)
{
	if (m_bEnabled && m_bVisible)
	{
		_ASSERT(!recursive_self || ContainsControl(recursive_self) && recursive_self != this);

		if (CanHaveFocus())
		{
			float dist = _GetNearestDist(rect, m_Rect, dir);
			if (dist > 0 && dist < nearest_ctrl_dist)
			{
				*nearest_ctrl = this;
				nearest_ctrl_dist = dist;
			}
		}
		else
		{
			ControlPtrList::iterator ctrl_iter = m_Childs.begin();
			for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
			{
				if (ctrl_iter->get() != recursive_self)
				{
					(*ctrl_iter)->GetNearestControl(rect, dir, nearest_ctrl, nearest_ctrl_dist, NULL);
				}
			}
		}

		if (!*nearest_ctrl && recursive_self && m_Parent)
		{
			m_Parent->GetNearestControl(rect, dir, nearest_ctrl, nearest_ctrl_dist, this);
		}
	}
}

void Control::SetHotkey(UINT nHotkey)
{
	m_nHotkey = nHotkey;
}

UINT Control::GetHotkey(void)
{
	return m_nHotkey;
}

StaticSkin::StaticSkin(void)
	: m_FontHeight(13)
	, m_FontFaceIndex(0)
	, m_TextColor(D3DCOLOR_ARGB(255, 255, 255, 255))
	, m_TextAlign(Font::AlignLeftTop)
	, m_TextOutlineColor(D3DCOLOR_ARGB(0, 0, 0, 0))
	, m_TextOutlineWidth(1.0f)
{
}

void StaticSkin::RequestResource(void)
{
	ControlSkin::RequestResource();
	if (!m_FontPath.empty())
	{
		_ASSERT(!m_Font);

		my::ResourceMgr::getSingleton().LoadFontAsync(m_FontPath.c_str(), m_FontHeight, m_FontFaceIndex, boost::bind(&StaticSkin::OnFontReady, this, boost::placeholders::_1), INT_MAX);
	}
}

void StaticSkin::ReleaseResource(void)
{
	ControlSkin::ReleaseResource();
	if (!m_FontPath.empty())
	{
		std::string Key = my::FontIORequest::BuildKey(m_FontPath.c_str(), m_FontHeight, m_FontFaceIndex);

		my::ResourceMgr::getSingleton().RemoveIORequestCallback(Key, boost::bind(&StaticSkin::OnFontReady, this, boost::placeholders::_1));

		m_Font.reset();
	}
}

void StaticSkin::OnFontReady(my::DeviceResourceBasePtr res)
{
	m_Font = boost::dynamic_pointer_cast<Font>(res);
}

void StaticSkin::DrawString(UIRender* ui_render, const wchar_t* str, const my::Rectangle& rect)
{
	if (m_Font)
	{
		if (!(m_TextOutlineColor & D3DCOLOR_ARGB(255, 0, 0, 0)))
		{
			ui_render->PushString(rect, str, m_TextColor, m_TextAlign, m_Font.get());
		}
		else
		{
			ui_render->PushString(rect, str, m_TextColor, m_TextAlign, m_TextOutlineColor, m_TextOutlineWidth, m_Font.get());
		}
	}
}

template<class Archive>
void Static::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	std::string text_u8 = wstou8(m_Text);
	ar << boost::serialization::make_nvp("m_Text", text_u8);
}

template<class Archive>
void Static::load(Archive & ar, const unsigned int version)
{
	// ! https://www.boost.org/doc/libs/1_74_0/libs/serialization/doc/implementation.html#charencoding 
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	std::string text_u8;
	ar >> boost::serialization::make_nvp("m_Text", text_u8);
	m_Text = D3DContext::getSingleton().OnControlTranslate(text_u8);
}

void Static::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if(m_Skin)
		{
			StaticSkinPtr Skin = boost::dynamic_pointer_cast<StaticSkin>(m_Skin);
			_ASSERT(Skin);

			Skin->DrawImage(ui_render, m_Skin->m_Image, m_Rect, m_Skin->m_Color);

			Skin->DrawString(ui_render, m_Text.c_str(), m_Rect);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

void ProgressBarSkin::RequestResource(void)
{
	StaticSkin::RequestResource();
	if (m_ForegroundImage)
	{
		m_ForegroundImage->RequestResource();
	}
}

void ProgressBarSkin::ReleaseResource(void)
{
	StaticSkin::ReleaseResource();
	if (m_ForegroundImage)
	{
		m_ForegroundImage->ReleaseResource();
	}
}

void ProgressBar::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if (m_Skin)
		{
			ProgressBarSkinPtr Skin = boost::dynamic_pointer_cast<ProgressBarSkin>(m_Skin);
			_ASSERT(Skin);

			Skin->DrawImage(ui_render, Skin->m_Image, m_Rect, m_Skin->m_Color);

			m_BlendProgress = Lerp(m_BlendProgress, m_Progress, 1.0f - powf(0.8f, 30 * fElapsedTime));
			Rectangle ProgressRect(m_Rect.l, m_Rect.t, Lerp(m_Rect.l, m_Rect.r, Max(0.0f, Min(1.0f, m_BlendProgress))), m_Rect.b);
			Skin->DrawImage(ui_render, Skin->m_ForegroundImage, ProgressRect, m_Skin->m_Color);

			Skin->DrawString(ui_render, m_Text.c_str(), m_Rect);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool ProgressBar::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ProgressBar::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void ButtonSkin::RequestResource(void)
{
	StaticSkin::RequestResource();
	if (m_DisabledImage)
	{
		m_DisabledImage->RequestResource();
	}
	if (m_PressedImage)
	{
		m_PressedImage->RequestResource();
	}
	if (m_MouseOverImage)
	{
		m_MouseOverImage->RequestResource();
	}
}

void ButtonSkin::ReleaseResource(void)
{
	StaticSkin::ReleaseResource();
	if (m_DisabledImage)
	{
		m_DisabledImage->ReleaseResource();
	}
	if (m_PressedImage)
	{
		m_PressedImage->ReleaseResource();
	}
	if (m_MouseOverImage)
	{
		m_MouseOverImage->ReleaseResource();
	}
}

void Button::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		Rectangle ButtonRect(m_Rect);

		if(m_Skin)
		{
			ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, ButtonRect, m_Skin->m_Color);
			}
			else
			{
				if(m_bPressed)
				{
					ButtonRect.offsetSelf(Skin->m_PressedOffset);
					Skin->DrawImage(ui_render, Skin->m_PressedImage, ButtonRect, m_Skin->m_Color);
				}
				else
				{
					D3DXCOLOR DstColor = m_Skin->m_Color;
					if(GetMouseOver() /*|| m_bHasFocus*/)
					{
						ButtonRect.offsetSelf(-Skin->m_PressedOffset);
					}
					else
					{
						DstColor.a = 0;
					}
					Skin->DrawImage(ui_render, Skin->m_Image, ButtonRect, m_Skin->m_Color);
					D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, ButtonRect, m_BlendColor);
				}
			}

			Skin->DrawString(ui_render, m_Text.c_str(), ButtonRect);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, ButtonRect.LeftTop(), ButtonRect.Extent());
		}
	}
}

bool Button::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return Control::HandleKeyboard(uMsg, wParam, lParam);
}

bool Button::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				SetCaptureControl(NULL);
				m_bPressed = false;

				if (HitTest(pt))
				{
					if (m_Skin && m_Skin->m_MouseClickSound)
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound);
					}

					if (m_EventMouseClick)
					{
						MouseEventArg arg(this, pt);
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;
		}
	}
	return Control::HandleMouse(uMsg, pt, wParam, lParam);
}

bool Button::CanHaveFocus(void) const
{
	return m_bVisible && m_bEnabled;
}

void EditBoxSkin::RequestResource(void)
{
	StaticSkin::RequestResource();
	if (m_DisabledImage)
	{
		m_DisabledImage->RequestResource();
	}
	if (m_FocusedImage)
	{
		m_FocusedImage->RequestResource();
	}
	if (m_CaretImage)
	{
		m_CaretImage->RequestResource();
	}
}

void EditBoxSkin::ReleaseResource(void)
{
	StaticSkin::ReleaseResource();
	if (m_DisabledImage)
	{
		m_DisabledImage->ReleaseResource();
	}
	if (m_FocusedImage)
	{
		m_FocusedImage->ReleaseResource();
	}
	if (m_CaretImage)
	{
		m_CaretImage->ReleaseResource();
	}
}

void EditBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		DWORD dwAbsoluteTime = timeGetTime();
		if(dwAbsoluteTime - m_dwLastBlink >= m_dwBlink )
		{
			m_bCaretOn = !m_bCaretOn;
			m_dwLastBlink = dwAbsoluteTime;
		}

		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if(m_Skin)
		{
			EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, m_Rect, m_Skin->m_Color);
			}
			else if(GetFocused())
			{
				Skin->DrawImage(ui_render, Skin->m_FocusedImage, m_Rect, m_Skin->m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_Image, m_Rect, m_Skin->m_Color);
			}

			if(Skin->m_Font)
			{
				Rectangle TextRect = m_Rect.shrink(m_Border);

				float x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
				float caret_x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
				if(m_nSelStart != m_nCaret)
				{
					float sel_start_x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nSelStart);
					float sel_left_x = __min(caret_x, sel_start_x) - x1st;
					float sel_right_x = __max(caret_x, sel_start_x) - x1st;

					Rectangle SelRect(
						Max(TextRect.l, TextRect.l + sel_left_x),
						TextRect.t,
						Min(TextRect.r, TextRect.l + sel_right_x),
						TextRect.b);

					Skin->DrawImage(ui_render, Skin->m_CaretImage, SelRect, Skin->m_SelBkColor);
				}

				ui_render->PushString(TextRect, m_Text.c_str() + m_nFirstVisible, Skin->m_TextColor, Font::AlignLeftMiddle, Skin->m_Font.get());

				if(GetFocused() && m_bCaretOn && !ImeEditBox::s_bHideCaret)
				{
					Rectangle CaretRect(
						TextRect.l + caret_x - x1st - 1,
						TextRect.t,
						TextRect.l + caret_x - x1st + 1,
						TextRect.b);

					if(!m_bInsertMode)
					{
						float charWidth;
						if(m_nCaret < (int)m_Text.length())
						{
							const Font::CharacterInfo * info = Skin->m_Font->GetCharacterInfo(m_Text[m_nCaret]);
							charWidth = info->horiAdvance;
						}
						else
						{
							const Font::CharacterInfo * info = Skin->m_Font->GetCharacterInfo(L'_');
							charWidth = info->horiAdvance;
						}
						CaretRect.r = TextRect.l + caret_x - x1st + charWidth;
					}

					Skin->DrawImage(ui_render, Skin->m_CaretImage, CaretRect, Skin->m_CaretColor);
				}
			}
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool EditBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_CHAR:
			{
				switch((WCHAR)wParam)
				{
				case VK_BACK:
					if(m_nCaret != m_nSelStart)
					{
						DeleteSelectionText();
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					else if(m_nCaret > 0)
					{
						PlaceCaret(m_nCaret - 1);
						m_nSelStart = m_nCaret;
						m_Text.erase(m_nCaret, 1);
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					ResetCaretBlink();
					break;

                case 24:        // Ctrl-X Cut
                case VK_CANCEL: // Ctrl-C Copy
					CopyToClipboard();
					if((WCHAR)wParam == 24)
					{
						DeleteSelectionText();
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					break;

				case 22:		// Ctrl-V Paste
					PasteFromClipboard();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
					break;

				case 1:
					//if(m_nSelStart == m_nCaret)
					{
						m_nSelStart = 0;
						PlaceCaret(m_Text.length());
					}
					break;

				case VK_RETURN:
					if(m_EventEnter)
					{
						ControlEventArg arg(this);
						m_EventEnter(&arg);
					}
					break;

				// Junk characters we don't want in the string
				case 26:  // Ctrl Z
				case 2:   // Ctrl B
				case 14:  // Ctrl N
				case 19:  // Ctrl S
				case 4:   // Ctrl D
				case 6:   // Ctrl F
				case 7:   // Ctrl G
				case 10:  // Ctrl J
				case 11:  // Ctrl K
				case 12:  // Ctrl L
				case 17:  // Ctrl Q
				case 23:  // Ctrl W
				case 5:   // Ctrl E
				case 18:  // Ctrl R
				case 20:  // Ctrl T
				case 25:  // Ctrl Y
				case 21:  // Ctrl U
				case 9:   // Ctrl I
				case 15:  // Ctrl O
				case 16:  // Ctrl P
				case 27:  // Ctrl [
				case 29:  // Ctrl ]
				case 28:  // Ctrl \ 
					break;

				default:
					if(m_nCaret != m_nSelStart)
						DeleteSelectionText();

					if(!m_bInsertMode && m_nCaret < (int)m_Text.length())
					{
						m_Text[m_nCaret] = (WCHAR)wParam;
						PlaceCaret(m_nCaret + 1);
						m_nSelStart = m_nCaret;
					}
					else
					{
						m_Text.insert(m_nCaret, 1, (WCHAR)wParam);
						PlaceCaret(m_nCaret + 1);
						m_nSelStart = m_nCaret;
					}
					ResetCaretBlink();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool EditBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_TAB:
				break;

			case VK_HOME:
				PlaceCaret(0);
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_END:
				PlaceCaret(m_Text.length());
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_INSERT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					CopyToClipboard();
				}
				else if(GetKeyState(VK_SHIFT) < 0)
				{
					PasteFromClipboard();
				}
				else
				{
					m_bInsertMode = !m_bInsertMode;
				}
				ResetCaretBlink();
				break;

			case VK_DELETE:
				if(m_nCaret != m_nSelStart)
				{
					DeleteSelectionText();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
				else
				{
					m_Text.erase(m_nCaret, 1);
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
				ResetCaretBlink();
				return true;

			case VK_LEFT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					PlaceCaret(GetPriorItemPos(m_nCaret));
				}
				else if(m_nCaret > 0)
				{
					if(m_nCaret != m_nSelStart && GetKeyState(VK_SHIFT) >= 0)
						PlaceCaret(__min(m_nCaret, m_nSelStart));
					else
						PlaceCaret(m_nCaret - 1);
				}
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_RIGHT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					PlaceCaret(GetNextItemPos(m_nCaret));
				}
				else if(m_nCaret < (int)m_Text.length())
				{
					PlaceCaret(m_nCaret + 1);
				}
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_UP:
			case VK_DOWN:
				ResetCaretBlink();
				return true;

			default:
				return wParam != VK_ESCAPE;
			}
			break;
		}
	}
	return false;
}

bool EditBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(HitTest(pt))
			{
				m_bMouseDrag = true;
				SetFocusControl(this);
				SetCaptureControl(this);

				if (m_Skin)
				{
					EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
					_ASSERT(Skin);

					if (Skin->m_Font)
					{
						Vector2 ptLocal = pt - m_Rect.LeftTop();
						float x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
						float x = ptLocal.x - m_Border.x + x1st;
						int nCP = Skin->m_Font->XtoCP(m_Text.c_str(), x);
						if (nCP < (int)m_Text.length())
						{
							float xLeft = Skin->m_Font->CPtoX(m_Text.c_str(), nCP);
							const Font::CharacterInfo* info = Skin->m_Font->GetCharacterInfo(m_Text[nCP]);
							if (x > xLeft + info->horiAdvance * 0.5f)
							{
								nCP += 1;
							}
						}

						if (uMsg == WM_LBUTTONDBLCLK)
						{
							m_nSelStart = GetPriorItemPos(nCP);

							PlaceCaret(GetNextItemPos(nCP));
						}
						else
						{
							PlaceCaret(nCP);

							if (GetKeyState(VK_SHIFT) >= 0)
							{
								m_nSelStart = m_nCaret;
							}
						}

						ResetCaretBlink();
					}
				}
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bMouseDrag)
			{
				SetCaptureControl(NULL);
				m_bMouseDrag = false;
			}
			break;

		case WM_MOUSEMOVE:
			if(m_bMouseDrag)
			{
				if (m_Skin)
				{
					EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
					_ASSERT(Skin);

					if (Skin->m_Font)
					{
						Vector2 ptLocal = pt - m_Rect.LeftTop();
						float x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
						float x = ptLocal.x - m_Border.x + x1st;
						int nCP = Skin->m_Font->XtoCP(m_Text.c_str(), x);
						if (nCP < (int)m_Text.length())
						{
							float xLeft = Skin->m_Font->CPtoX(m_Text.c_str(), nCP);
							const Font::CharacterInfo* info = Skin->m_Font->GetCharacterInfo(m_Text[nCP]);
							if (x > xLeft + info->horiAdvance * 0.5f)
							{
								nCP += 1;
							}
						}

						PlaceCaret(nCP);
					}
				}
			}
			break;
		}
	}
	return false;
}

bool EditBox::CanHaveFocus(void) const
{
	return m_bVisible && m_bEnabled;
}

void EditBox::OnFocusIn(void)
{
	Control::OnFocusIn();

	ResetCaretBlink();
}

void EditBox::SetText(const std::wstring & Text)
{
	m_Text = Text;
	PlaceCaret(Text.length());
	m_nSelStart = m_nCaret;
}

const std::wstring & EditBox::GetText(void) const
{
	return m_Text;
}

void EditBox::PlaceCaret(int nCP)
{
	m_nCaret = nCP;

	if (m_Skin)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
		_ASSERT(Skin);

		if (Skin->m_Font)
		{
			float x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
			float x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
			float x2;
			if (m_nCaret < (int)m_Text.length())
			{
				const Font::CharacterInfo* info = Skin->m_Font->GetCharacterInfo(m_Text[m_nCaret]);
				x2 = x + info->horiAdvance;
			}
			else
			{
				x2 = x;
			}

			if (x <= x1st) // ! '=' to rewrite 1st visible, if it was large than caret
			{
				m_nFirstVisible = m_nCaret;
			}
			else
			{
				float xNewLeft = x2 - (m_Rect.Width() - m_Border.x - m_Border.z);
				if (xNewLeft > x1st)
				{
					int nCPNew1st = Skin->m_Font->XtoCP(m_Text.c_str(), xNewLeft);
					float xNew1st = Skin->m_Font->CPtoX(m_Text.c_str(), nCPNew1st);
					if (xNew1st < xNewLeft)
					{
						nCPNew1st++;
					}
					m_nFirstVisible = nCPNew1st;
				}
			}
		}
	}
}

void EditBox::ResetCaretBlink(void)
{
	m_bCaretOn = true;
	m_dwLastBlink = timeGetTime();
}

void EditBox::DeleteSelectionText(void)
{
	int nFirst = __min(m_nCaret, m_nSelStart);
	int nLast = __max(m_nCaret, m_nSelStart);
	PlaceCaret(nFirst);
	m_nSelStart = m_nCaret;
	m_Text.erase(nFirst, nLast - nFirst);
}

void EditBox::CopyToClipboard(void)
{
	if(m_nCaret != m_nSelStart && OpenClipboard(NULL))
	{
		EmptyClipboard();

        HGLOBAL hBlock = GlobalAlloc( GMEM_MOVEABLE, sizeof( WCHAR ) * ( m_Text.length() + 1 ) );
        if( hBlock )
        {
            WCHAR* pwszText = ( WCHAR* )GlobalLock( hBlock );
            if( pwszText )
            {
                int nFirst = __min( m_nCaret, m_nSelStart );
                int nLast = __max( m_nCaret, m_nSelStart );
                if( nLast - nFirst > 0 )
                    CopyMemory( pwszText, m_Text.c_str() + nFirst, ( nLast - nFirst ) * sizeof( WCHAR ) );
                pwszText[nLast - nFirst] = L'\0';  // Terminate it
                GlobalUnlock( hBlock );
            }
            SetClipboardData( CF_UNICODETEXT, hBlock );
        }
        CloseClipboard();
        // We must not free the object until CloseClipboard is called.
        if( hBlock )
            GlobalFree( hBlock );
	}
}

void EditBox::PasteFromClipboard(void)
{
	DeleteSelectionText();

    if( OpenClipboard( NULL ) )
    {
        HANDLE handle = GetClipboardData( CF_UNICODETEXT );
        if( handle )
        {
            // Convert the ANSI string to Unicode, then
            // insert to our buffer.
            WCHAR* pwszText = ( WCHAR* )GlobalLock( handle );
            if( pwszText )
            {
                // Copy all characters up to null.
				m_Text.insert(m_nCaret, pwszText);
				PlaceCaret( m_nCaret + lstrlenW( pwszText ) );
                m_nSelStart = m_nCaret;
                GlobalUnlock( handle );
            }
        }
        CloseClipboard();
    }
}

int EditBox::GetPriorItemPos(int nCP)
{
	int i = Max(Min(nCP, (int)m_Text.length()) - 1, 0);
	int state = 0;
	for(; i >= 0; i--)
	{
		switch(state)
		{
		case 0:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else if(L' ' == m_Text[i])
				state = 0;
			else
				state = 2;
			break;

		case 1:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else
				return i + 1;
			break;

		case 2:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				return i + 1;
			else
				state = 2;
			break;
		}
	}
	return 0;
}

int EditBox::GetNextItemPos(int nCP)
{
	int i = Max(Min(nCP, (int)m_Text.length() - 1), 0);
	int state = 0;
	for(; i < (int)m_Text.length(); i++)
	{
		switch(state)
		{
		case 0:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else
				state = 2;
			break;

		case 1:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else if(L' ' == m_Text[i])
				state = 3;
			else
				return i;
			break;

		case 2:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				return i;
			else
				state = 2;
			break;

		case 3:
			if(L' ' == m_Text[i])
				state = 3;
			else
				return i;
		}
	}
	return m_Text.length();
}

bool ImeEditBox::s_bHideCaret = false;

std::wstring ImeEditBox::s_CompString;

void ImeEditBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		EditBox::Draw(ui_render, fElapsedTime, Offset, Size);

	    ImeUi_RenderUI();

		if(GetFocused())
		{
			RenderIndicator(ui_render, fElapsedTime);

			RenderComposition(ui_render, fElapsedTime);

			if(ImeUi_IsShowCandListWindow())
				RenderCandidateWindow(ui_render, fElapsedTime);
		}
	}
}

bool ImeEditBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_DESTROY:
			ImeUi_Uninitialize();
			break;
		}

		if(!ImeUi_IsEnabled())
			return EditBox::MsgProc(hWnd, uMsg, wParam, lParam);

		bool trapped = false;
		ImeUi_ProcessMessage(hWnd, uMsg, wParam, lParam, &trapped);
		if(!trapped)
			trapped = EditBox::MsgProc(hWnd, uMsg, wParam, lParam); // ! Toggle trapped if processed this msg

		return trapped;
	}
	return false;
}

bool ImeEditBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (EditBox::HandleMouse(uMsg, pt, wParam, lParam))
	{
		return true;
	}
	return false;
}

void ImeEditBox::OnFocusIn(void)
{
	ImeUi_EnableIme(true);

	EditBox::OnFocusIn();
}

void ImeEditBox::OnFocusOut(void)
{
	ImeUi_FinalizeString();
	ImeUi_EnableIme(false);

	EditBox::OnFocusOut();
}

void ImeEditBox::Initialize(HWND hWnd)
{
    ImeUiCallback_DrawRect = NULL;
    ImeUiCallback_Malloc = malloc;
    ImeUiCallback_Free = free;
    ImeUiCallback_DrawFans = NULL;

    ImeUi_Initialize(hWnd);
    
    ImeUi_EnableIme(true);
}

void ImeEditBox::Uninitialize(void)
{
    ImeUi_EnableIme(false);

    ImeUi_Uninitialize();
}

bool ImeEditBox::StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!ImeUi_IsEnabled())
		return false;

	switch(uMsg)
	{
	case WM_INPUTLANGCHANGE:
		return true;

	case WM_IME_SETCONTEXT:
		lParam = 0;
		return false;

	case WM_IME_STARTCOMPOSITION:
		ResetCompositionString();
		s_bHideCaret = true;
		return true;

	case WM_IME_ENDCOMPOSITION:
		s_bHideCaret = false;
		return false;

	case WM_IME_COMPOSITION:
		return false;
	}
	return false;
}

void ImeEditBox::ResetCompositionString(void)
{
	s_CompString.clear();
}

void ImeEditBox::EnableImeSystem(bool bEnable)
{
	ImeUi_EnableIme(bEnable);
}

void ImeEditBox::RenderIndicator(UIRender * ui_render, float fElapsedTime)
{
}

void ImeEditBox::RenderComposition(UIRender * ui_render, float fElapsedTime)
{
	if (m_Skin)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
		_ASSERT(Skin);

		if (Skin->m_Font)
		{
			s_CompString = ts2ws(ImeUi_GetCompositionString());

			Rectangle TextRect = m_Rect.shrink(m_Border);

			float x, x1st;
			x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
			x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);

			Rectangle rc(TextRect.l + x - x1st, TextRect.t, TextRect.l + x - x1st + Skin->m_Font->CalculateStringWidth(s_CompString.c_str()), TextRect.b);
			if (rc.r > TextRect.r)
				rc.offsetSelf(TextRect.l - rc.l, TextRect.Height());

			Skin->DrawImage(ui_render, Skin->m_CaretImage, rc, m_CompWinColor);

			ui_render->PushString(rc, s_CompString.c_str(), Skin->m_TextColor, Font::AlignLeftTop, Skin->m_Font.get());

			float caret_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());
			if (m_bCaretOn)
			{
				Rectangle CaretRect(rc.l + caret_x - 1, rc.t, rc.l + caret_x + 1, rc.b);

				Skin->DrawImage(ui_render, Skin->m_CaretImage, CaretRect, Skin->m_CaretColor);
			}
		}
	}
}

void ImeEditBox::RenderCandidateWindow(UIRender * ui_render, float fElapsedTime)
{
	if (m_Skin)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
		_ASSERT(Skin);

		if (Skin->m_Font)
		{
			Rectangle TextRect = m_Rect.shrink(m_Border);

			float x, x1st, comp_x;
			x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
			x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);

			Rectangle CompRect(TextRect.l + x - x1st, TextRect.t, TextRect.l + x - x1st + Skin->m_Font->CalculateStringWidth(s_CompString.c_str()), TextRect.b);
			if (CompRect.r > TextRect.r)
				CompRect.offsetSelf(TextRect.l - CompRect.l, TextRect.Height());

			comp_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());

			float WidthRequired = 0;
			float HeightRequired = 0;
			float SingleLineHeight = 0;

			std::wstring horizontalText;
			for (UINT i = 0; i < MAX_CANDLIST && *ImeUi_GetCandidate(i) != L'\0'; i++)
			{
				horizontalText += ts2ws(ImeUi_GetCandidate(i));
			}

			Rectangle CandRect = Rectangle::LeftTop(CompRect.l + comp_x, CompRect.b, Skin->m_Font->CalculateStringWidth(horizontalText.c_str()), (float)Skin->m_Font->m_LineHeight);

			Skin->DrawImage(ui_render, Skin->m_CaretImage, CandRect, m_CandidateWinColor);

			ui_render->PushString(CandRect, horizontalText.c_str(), Skin->m_TextColor, Font::AlignLeftTop, Skin->m_Font.get());
		}
	}
}

void ScrollBarSkin::RequestResource(void)
{
	ControlSkin::RequestResource();
	if (m_UpBtnNormalImage)
	{
		m_UpBtnNormalImage->RequestResource();
	}
	if (m_UpBtnDisabledImage)
	{
		m_UpBtnDisabledImage->RequestResource();
	}
	if (m_DownBtnNormalImage)
	{
		m_DownBtnNormalImage->RequestResource();
	}
	if (m_DownBtnDisabledImage)
	{
		m_DownBtnDisabledImage->RequestResource();
	}
	if (m_ThumbBtnNormalImage)
	{
		m_ThumbBtnNormalImage->RequestResource();
	}
}

void ScrollBarSkin::ReleaseResource(void)
{
	ControlSkin::ReleaseResource();
	if (m_UpBtnNormalImage)
	{
		m_UpBtnNormalImage->ReleaseResource();
	}
	if (m_UpBtnDisabledImage)
	{
		m_UpBtnDisabledImage->ReleaseResource();
	}
	if (m_DownBtnNormalImage)
	{
		m_DownBtnNormalImage->ReleaseResource();
	}
	if (m_DownBtnDisabledImage)
	{
		m_DownBtnDisabledImage->ReleaseResource();
	}
	if (m_ThumbBtnNormalImage)
	{
		m_ThumbBtnNormalImage->ReleaseResource();
	}
}

void ScrollBar::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	SimulateRepeatedScroll();

	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if(m_Skin)
		{
			ScrollBarSkinPtr Skin = boost::dynamic_pointer_cast<ScrollBarSkin>(m_Skin);
			_ASSERT(Skin);

			Skin->DrawImage(ui_render, Skin->m_Image, m_Rect, m_Skin->m_Color);

			Rectangle UpButtonRect = Rectangle::LeftTop(m_Rect.l, m_Rect.t, m_Rect.Width(), m_UpDownButtonHeight);

			Rectangle DownButtonRect = Rectangle::RightBottom(m_Rect.r, m_Rect.b, m_Rect.Width(), m_UpDownButtonHeight);

			if(m_bEnabled && m_nEnd - m_nStart > m_nPageSize)
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnNormalImage, UpButtonRect, m_Skin->m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnNormalImage, DownButtonRect, m_Skin->m_Color);

				float fTrackHeight = m_Rect.Height() - m_UpDownButtonHeight * 2;
				float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
				int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
				float fThumbTop = UpButtonRect.b + (float)(m_nPosition - m_nStart) / nMaxPosition * (fTrackHeight - fThumbHeight);
				Rectangle ThumbButtonRect(m_Rect.l, fThumbTop, m_Rect.r, fThumbTop + fThumbHeight);

				Skin->DrawImage(ui_render, Skin->m_ThumbBtnNormalImage, ThumbButtonRect, m_Skin->m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnDisabledImage, UpButtonRect, m_Skin->m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnDisabledImage, DownButtonRect, m_Skin->m_Color);
			}
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool ScrollBar::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//if(WM_CAPTURECHANGED == uMsg)
	//{
	//	if((HWND)lParam != DxutApp::getSingleton().GetHWND())
	//		m_bDrag = false;
	//}
	return false;
}

bool ScrollBar::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ScrollBar::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			Rectangle UpButtonRect = Rectangle::LeftTop(m_Rect.l, m_Rect.t, m_Rect.Width(), m_UpDownButtonHeight);
			if(UpButtonRect.PtInRect(pt))
			{
				if(m_nPosition > m_nStart)
					--m_nPosition;
				m_Arrow = CLICKED_UP;
				m_dwArrowTS = timeGetTime();
				m_bPressed = true;
				SetCaptureControl(this);
				return true;
			}

			Rectangle DownButtonRect = Rectangle::RightBottom(m_Rect.r, m_Rect.b, m_Rect.Width(), m_UpDownButtonHeight);
			if(DownButtonRect.PtInRect(pt))
			{
				if(m_nPosition + m_nPageSize < m_nEnd)
					++m_nPosition;
				m_Arrow = CLICKED_DOWN;
				m_dwArrowTS = timeGetTime();
				m_bPressed = true;
				SetCaptureControl(this);
				return true;
			}

			float fTrackHeight = m_Rect.Height() - m_UpDownButtonHeight * 2;
			float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			float fMaxThumb = fTrackHeight - fThumbHeight;
			float fThumbTop = (float)(m_nPosition - m_nStart) / nMaxPosition * fMaxThumb;
			Rectangle ThumbButtonRect(m_Rect.l, UpButtonRect.b + fThumbTop, m_Rect.r, UpButtonRect.b + fThumbTop + fThumbHeight);
			if(ThumbButtonRect.PtInRect(pt))
			{
				m_bDrag = true;
				m_fThumbOffsetY = pt.y - fThumbTop;
				SetCaptureControl(this);
				return true;
			}

			if(pt.x >= ThumbButtonRect.l && pt.x < ThumbButtonRect.r)
			{
				if(pt.y >= UpButtonRect.b && pt.y < ThumbButtonRect.t)
				{
					Scroll(-m_nPageSize);
					m_bPressed = true;
					SetCaptureControl(this);
					return true;
				}
				else if(pt.y >= ThumbButtonRect.b && pt.y < DownButtonRect.t)
				{
					Scroll( m_nPageSize);
					m_bPressed = true;
					SetCaptureControl(this);
					return true;
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		if (m_bPressed || m_bDrag)
		{
			SetCaptureControl(NULL);
			m_bPressed = false;
			m_bDrag = false;
			m_Arrow = CLEAR;
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			float fTrackHeight = m_Rect.Height() - m_UpDownButtonHeight * 2;
			float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			float fMaxThumb = fTrackHeight - fThumbHeight;
			float fThumbTop = pt.y - m_fThumbOffsetY;

			if(fThumbTop < 0)
				fThumbTop = 0;
			else if(fThumbTop + fThumbHeight > m_Rect.Height() - m_UpDownButtonHeight * 2)
				fThumbTop = m_Rect.Height() - m_UpDownButtonHeight * 2 - fThumbHeight;

			m_nPosition = (int)(m_nStart + (fThumbTop + fMaxThumb / (nMaxPosition * 2)) * nMaxPosition / fMaxThumb);
			return true;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			UINT uLines;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0);
			int zDelta = (short)HIWORD(wParam) / WHEEL_DELTA;
			Scroll(-zDelta * uLines);
			return true;
		}
		break;
	}
	return false;
}

void ScrollBar::SimulateRepeatedScroll(void)
{
	// Check if the arrow button has been held for a while.
	// If so, update the thumb position to simulate repeated
	// scroll.
	if (m_Arrow != CLEAR)
	{
		DWORD dwAbsoluteTime = timeGetTime();
		switch (m_Arrow)
		{
		case CLICKED_UP:
			if (330 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(-1);
				m_Arrow = HELD_UP;
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case HELD_UP:
			if (50 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(-1);
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case CLICKED_DOWN:
			if (330 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(1);
				m_Arrow = HELD_DOWN;
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case HELD_DOWN:
			if (50 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(1);
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;
		}
	}
}

void ScrollBar::Scroll(int nDelta)
{
	m_nPosition = Max(m_nStart, Min(m_nEnd - m_nPageSize, m_nPosition + nDelta));
}

void ScrollBar::ScrollTo(int nPosition)
{
	if (nPosition >= m_nStart)
	{
		if (nPosition < m_nPosition)
		{
			m_nPosition = nPosition;
		}
		else if (nPosition >= m_nPosition + m_nPageSize)
		{
			m_nPosition = Min(m_nEnd - m_nPageSize, nPosition + 1 - m_nPageSize);
		}
	}
}

void CheckBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if(m_Skin)
		{
			ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, m_Rect, m_Skin->m_Color);
			}
			else
			{
				if (m_bPressed)
				{
					m_Rect = m_Rect.offset(Skin->m_PressedOffset);
					Skin->DrawImage(ui_render, Skin->m_PressedImage, m_Rect, m_Skin->m_Color);
				}
				else
				{
					D3DXCOLOR DstColor = m_Skin->m_Color;
					if (GetMouseOver() /*|| m_bHasFocus*/)
					{
						m_Rect = m_Rect.offset(-Skin->m_PressedOffset);
					}
					else
					{
						DstColor.a = 0;
					}
					Skin->DrawImage(ui_render, m_Checked ? Skin->m_PressedImage : Skin->m_Image, m_Rect, m_Skin->m_Color);
					D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, m_Rect, m_BlendColor);
				}
			}

			Skin->DrawString(ui_render, m_Text.c_str(), m_Rect);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool CheckBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				m_Checked = true;

				return Control::HandleKeyboard(uMsg, wParam, lParam);
			}
			break;
		}
	}
	return Control::HandleKeyboard(uMsg, wParam, lParam);
}

bool CheckBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(HitTest(pt))
			{
				m_Checked = true;

				return Control::HandleMouse(uMsg, pt, wParam, lParam);
			}
			break;
		}
	}
	return Control::HandleMouse(uMsg, pt, wParam, lParam);
}

void ComboBoxSkin::RequestResource(void)
{
	ButtonSkin::RequestResource();
	if (m_DropdownImage)
	{
		m_DropdownImage->RequestResource();
	}
	if (m_DropdownItemMouseOverImage)
	{
		m_DropdownItemMouseOverImage->RequestResource();
	}
}

void ComboBoxSkin::ReleaseResource(void)
{
	ButtonSkin::ReleaseResource();
	if (m_DropdownImage)
	{
		m_DropdownImage->ReleaseResource();
	}
	if (m_DropdownItemMouseOverImage)
	{
		m_DropdownItemMouseOverImage->ReleaseResource();
	}
}

ComboBox::ComboBox(const char* Name)
	: Button(Name)
	, m_DropdownSize(100, 100)
	, m_DropdownRect(0, 0, 100, 100)
	, m_ScrollBar(new ScrollBar(NamedObject::MakeUniqueName((std::string(Name) + "_scrollbar").c_str()).c_str()))
	, m_ScrollbarWidth(20)
	, m_ScrollbarUpDownBtnHeight(20)
	, m_Border(0, 0, 0, 0)
	, m_ItemHeight(15)
	, m_iFocused(0)
	, m_iSelected(-1)
{
	OnLayout();

	m_ScrollBar->m_Parent = this;
}

template<class Archive>
void ComboBox::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Button);
	ar << BOOST_SERIALIZATION_NVP(m_DropdownSize);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollBar);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar << BOOST_SERIALIZATION_NVP(m_Border);
	ar << BOOST_SERIALIZATION_NVP(m_ItemHeight);
}

template<class Archive>
void ComboBox::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Button);
	ar >> BOOST_SERIALIZATION_NVP(m_DropdownSize);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollBar);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar >> BOOST_SERIALIZATION_NVP(m_Border);
	ar >> BOOST_SERIALIZATION_NVP(m_ItemHeight);
	m_ScrollBar->m_Parent = this;
}

void ComboBox::RequestResource(void)
{
	Control::RequestResource();

	m_ScrollBar->RequestResource();
}

void ComboBox::ReleaseResource(void)
{
	m_ScrollBar->ReleaseResource();

	Control::ReleaseResource();
}

void ComboBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if(m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if(m_Skin)
		{
			ComboBoxSkinPtr Skin = boost::dynamic_pointer_cast<ComboBoxSkin>(m_Skin);
			_ASSERT(Skin);

			Rectangle BtnRect = m_Rect;

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, BtnRect, m_Skin->m_Color);
			}
			else
			{
				if(m_bPressed)
				{
					ui_render->Flush();

					BtnRect = BtnRect.offset(Skin->m_PressedOffset);

					Skin->DrawImage(ui_render, Skin->m_PressedImage, BtnRect, m_Skin->m_Color);

					m_DropdownRect = Rectangle::LeftTop(m_Rect.l, m_Rect.b, m_DropdownSize.x, m_DropdownSize.y);

					Skin->DrawImage(ui_render, Skin->m_DropdownImage, m_DropdownRect, m_Skin->m_Color);

					m_ScrollBar->Draw(ui_render, fElapsedTime, Vector2(m_DropdownRect.r, m_DropdownRect.t), Vector2(m_ScrollbarWidth, m_DropdownSize.y));

					int i = m_ScrollBar->m_nPosition;
					float y = m_DropdownRect.t + m_Border.y;
					for(; i < (int)m_Items.size() && y <= m_DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect = Rectangle::LeftTop(m_DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight);

						if(i == m_iFocused)
						{
							Skin->DrawImage(ui_render, Skin->m_DropdownItemMouseOverImage, ItemRect, m_Skin->m_Color);
						}

						if (Skin->m_Font)
						{
							ComboBoxItem& item = m_Items[i];
							Rectangle ItemTextRect = ItemRect.shrink(m_Border.x, 0, m_Border.z, 0);
							ui_render->PushString(ItemTextRect, item.strText.c_str(), Skin->m_DropdownItemTextColor, Skin->m_DropdownItemTextAlign, Skin->m_Font.get());
						}
					}
				}
				else
				{
					D3DXCOLOR DstColor = m_Skin->m_Color;
					if(GetMouseOver() /*|| m_bHasFocus*/)
					{
						BtnRect = BtnRect.offset(-Skin->m_PressedOffset);
					}
					else
					{
						DstColor.a = 0;
					}
					Skin->DrawImage(ui_render, Skin->m_Image, BtnRect, m_Skin->m_Color);
					D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, BtnRect, m_BlendColor);
				}
			}

			Skin->DrawString(ui_render, m_Text.c_str(), BtnRect);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, m_Rect.LeftTop(), m_Rect.Extent());
		}
	}
}

bool ComboBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ComboBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				if (!m_bPressed)
				{
					m_bPressed = true;
					m_iFocused = m_iSelected;
					m_ScrollBar->ScrollTo(m_iFocused);
					return true;
				}
				else
				{
					if (m_iSelected != m_iFocused)
					{
						SetSelected(m_iFocused);

						if (m_EventSelectionChanged)
						{
							ControlEventArg arg(this);
							m_EventSelectionChanged(&arg);
						}
					}
					m_bPressed = false;
					return true;
				}
			}
			else if (wParam == VK_UP)
			{
				if (m_bPressed)
				{
					if (m_iFocused > 0)
					{
						m_ScrollBar->ScrollTo(--m_iFocused);
					}
					return true;
				}
			}
			else if (wParam == VK_DOWN)
			{
				if (m_bPressed)
				{
					if (m_iFocused + 1 < (int)m_Items.size())
					{
						m_ScrollBar->ScrollTo(++m_iFocused);
					}
					return true;
				}
			}
			else if (wParam == VK_ESCAPE)
			{
				if (m_bPressed)
				{
					m_bPressed = false;
					return true;
				}
			}
			break;

		case WM_KEYUP:
			if (wParam == VK_RETURN)
			{
				return false;
			}
			break;
		}
	}
	return Control::HandleKeyboard(uMsg, wParam, lParam);
}

bool ComboBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_MOUSEWHEEL:
			if (m_ScrollBar->HandleMouse(uMsg, pt, wParam, lParam))
			{
				return true;
			}
			break;

		case WM_MOUSEMOVE:
			if(m_bPressed)
			{
				if(m_DropdownRect.PtInRect(pt))
				{
					int i = m_ScrollBar->m_nPosition;
					float y = m_DropdownRect.t;
					for(; i < (int)m_Items.size() && y <= m_DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect = Rectangle::LeftTop(m_DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight);

						if(ItemRect.PtInRect(pt))
						{
							m_iFocused = i;
							break;
						}
					}
					return true;
				}
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(m_Rect.PtInRect(pt))
			{
				m_bPressed = !m_bPressed;
				m_iFocused = m_iSelected;
				m_ScrollBar->ScrollTo(m_iFocused);
				SetFocusControl(this);
				SetCaptureControl(this);
				return true;
			}

			if(m_bPressed)
			{
				if(m_DropdownRect.PtInRect(pt))
				{
					int i = m_ScrollBar->m_nPosition;
					float y = m_DropdownRect.t;
					for(; i < (int)m_Items.size() && y <= m_DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect = Rectangle::LeftTop(m_DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight);

						if(ItemRect.PtInRect(pt))
						{
							if(m_iSelected != i)
							{
								SetSelected(i);

								if(m_EventSelectionChanged)
								{
									ControlEventArg arg(this);
									m_EventSelectionChanged(&arg);
								}
							}
							m_bPressed = false;
							break;
						}
					}
					OnMouseLeave(pt);
					return true;
				}
			}
			m_bPressed = false;
			OnMouseLeave(pt);
			break;

		case WM_LBUTTONUP:
			if (GetCaptureControl() == this)
			{
				SetCaptureControl(NULL);
				return true;
			}
			break;
		}
	}
	return false;
}

void ComboBox::OnFocusOut(void)
{
	m_bPressed = false;

	Control::OnFocusOut();
}

bool ComboBox::HitTest(const Vector2 & pt) const
{
	if (m_bPressed)
	{
		return m_Rect.PtInRect(pt) || m_DropdownRect.PtInRect(pt) || m_ScrollBar->m_Rect.PtInRect(pt);
	}

	return Control::HitTest(pt);
}

void ComboBox::OnLayout(void)
{
	m_ScrollBar->m_x = UDim(0, 0);

	m_ScrollBar->m_y = UDim(0, 0);

	m_ScrollBar->m_Width = UDim(0, m_ScrollbarWidth);

	m_ScrollBar->m_Height = UDim(0, m_DropdownSize.y);

	m_ScrollBar->m_nPageSize = (int)((m_DropdownSize.y - m_Border.y - m_Border.w) / m_ItemHeight);

	m_ScrollBar->m_nEnd = m_Items.size();
}

Control * ComboBox::GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible)
{
	if (m_bPressed)
	{
		Control* ctrl = m_ScrollBar->GetChildAtPoint(pt, bIgnoreVisible);
		if (ctrl)
		{
			return ctrl;
		}
	}

	return Control::GetChildAtPoint(pt, bIgnoreVisible);
}

void ComboBox::SetDropdownSize(const Vector2 & DropdownSize)
{
	m_DropdownSize = DropdownSize;

	OnLayout();
}

const Vector2 & ComboBox::GetDropdownSize(void) const
{
	return m_DropdownSize;
}

void ComboBox::SetScrollbarWidth(float Width)
{
	m_ScrollbarWidth = Width;

	OnLayout();
}

float ComboBox::GetScrollbarWidth(void) const
{
	return m_ScrollbarWidth;
}

void ComboBox::SetScrollbarUpDownBtnHeight(float Height)
{
	m_ScrollbarUpDownBtnHeight = Height;

	OnLayout();
}

float ComboBox::GetScrollbarUpDownBtnHeight(void) const
{
	return m_ScrollbarUpDownBtnHeight;
}

void ComboBox::SetBorder(const Vector4 & Border)
{
	m_Border = Border;

	OnLayout();
}

const Vector4 & ComboBox::GetBorder(void) const
{
	return m_Border;
}

void ComboBox::SetItemHeight(float ItemHeight)
{
	m_ItemHeight = ItemHeight;

	OnLayout();
}

float ComboBox::GetItemHeight(void) const
{
	return m_ItemHeight;
}

void ComboBox::SetSelected(int iSelected)
{
	if (m_iSelected != iSelected)
	{
		m_iSelected = iSelected;

		if (m_iSelected >= 0 && m_iSelected < (int)m_Items.size())
		{
			m_Text = m_Items[m_iSelected].strText;
		}
	}
}

int ComboBox::GetSelected(void) const
{
	return m_iSelected;
}

void ComboBox::AddItem(const std::wstring & strText)
{
	_ASSERT(!strText.empty());

	m_Items.push_back(ComboBoxItem(strText));

	m_ScrollBar->m_nEnd = m_Items.size();
}

void ComboBox::RemoveAllItems(void)
{
	m_Items.clear();

	m_ScrollBar->m_nEnd = 0;

	m_iSelected = -1;
}

bool ComboBox::ContainsItem(const std::wstring & strText, UINT iStart) const
{
	return -1 != FindItem(strText, iStart);
}

int ComboBox::FindItem(const std::wstring & strText, UINT iStart) const
{
	ComboBoxItemList::const_iterator item_iter = boost::find_if(
		boost::make_iterator_range(m_Items.begin() + iStart, m_Items.end()), boost::bind(std::equal_to<std::wstring>(), boost::bind(&ComboBoxItem::strText, boost::placeholders::_1), strText));
	if(item_iter != m_Items.end())
	{
		return std::distance(m_Items.begin(), item_iter);
	}
	return -1;
}

void * ComboBox::GetItemData(int index)
{
	return m_Items[index].pData;
}

void ComboBox::SetItemData(int index, void * pData)
{
	m_Items[index].pData = pData;
}

unsigned int ComboBox::GetItemDataUInt(int index)
{
	return PtrToUint(GetItemData(index));
}

void ComboBox::SetItemData(int index, unsigned int uData)
{
	SetItemData(index, UintToPtr(uData));
}

UINT ComboBox::GetNumItems(void)
{
	return m_Items.size();
}

void ListBoxSkin::RequestResource(void)
{
	ControlSkin::RequestResource();
}

void ListBoxSkin::ReleaseResource(void)
{
	ControlSkin::ReleaseResource();
}

ListBox::ListBox(const char* Name)
	: Control(Name)
	, m_ScrollBar(new ScrollBar(NamedObject::MakeUniqueName((std::string(Name) + "_scrollbar").c_str()).c_str()))
	, m_ScrollbarWidth(20)
	, m_ScrollbarUpDownBtnHeight(20)
	, m_ItemSize(50, 50)
	, m_ItemColumn(1)
{
	OnLayout();

	m_ScrollBar->m_Parent = this;
}

template<class Archive>
void ListBox::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollBar);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar << BOOST_SERIALIZATION_NVP(m_ItemSize);
	ar << BOOST_SERIALIZATION_NVP(m_ItemColumn);
}

template<class Archive>
void ListBox::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollBar);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar >> BOOST_SERIALIZATION_NVP(m_ItemSize);
	ar >> BOOST_SERIALIZATION_NVP(m_ItemColumn);
	m_ScrollBar->m_Parent = this;
}

void ListBox::RequestResource(void)
{
	Control::RequestResource();

	m_ScrollBar->RequestResource();
}

void ListBox::ReleaseResource(void)
{
	m_ScrollBar->ReleaseResource();

	Control::ReleaseResource();
}

void ListBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	if (m_bVisible)
	{
		m_Rect = Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		if (m_Skin)
		{
			ListBoxSkinPtr Skin = boost::dynamic_pointer_cast<ListBoxSkin>(m_Skin);
			_ASSERT(Skin);

			m_ScrollBar->Draw(ui_render, fElapsedTime, Vector2(m_Rect.l + m_Rect.Width() - m_ScrollbarWidth, m_Rect.t), Vector2(m_ScrollbarWidth, m_Rect.Height()));

			for (int i = m_ScrollBar->m_nPosition; i < m_ScrollBar->m_nPosition + m_ScrollBar->m_nPageSize; i++)
			{
				const int column = Min(m_ItemColumn, (int)m_Childs.size() - i * m_ItemColumn);
				for (int j = 0; j < column; j++)
				{
					ControlPtr& ctrl = m_Childs[i * m_ItemColumn + j];
					if (ctrl->GetVisible())
					{
						ctrl->Draw(ui_render, fElapsedTime, Vector2(m_Rect.l + j * m_ItemSize.x, m_Rect.t + (i - m_ScrollBar->m_nPosition) * m_ItemSize.y), m_ItemSize);
					}
				}
			}
		}
	}
}

bool ListBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ListBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ListBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_MOUSEWHEEL:
			if (m_ScrollBar->HandleMouse(uMsg, pt, wParam, lParam))
			{
				return true;
			}
			break;
		}
	}
	return Control::HandleMouse(uMsg, pt, wParam, lParam);
}

void ListBox::OnLayout(void)
{
	m_ScrollBar->m_x = UDim(0, 0);

	m_ScrollBar->m_y = UDim(0, 0);

	m_ScrollBar->m_Width = UDim(0, m_ScrollbarWidth);

	m_ScrollBar->m_Height = UDim(0, m_Height.offset);

	m_ScrollBar->m_nPageSize = (int)(m_ScrollBar->m_Height.offset / m_ItemSize.y);

	m_ItemColumn = Max(1, (int)((fabs(m_Width.offset) - m_ScrollbarWidth) / m_ItemSize.x));

	m_ScrollBar->m_nEnd = (int)ceilf(m_Childs.size() / (float)m_ItemColumn);

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_x = UDim(0, 0);

		(*ctrl_iter)->m_y = UDim(0, 0);

		(*ctrl_iter)->m_Width = UDim(0, m_ItemSize.x);

		(*ctrl_iter)->m_Height = UDim(0, m_ItemSize.y);
	}
}

void ListBox::InsertControl(unsigned int i, ControlPtr control)
{
	Control::InsertControl(i, control);

	m_ScrollBar->m_nEnd = (int)ceilf(m_Childs.size() / (float)m_ItemColumn);
}

void ListBox::RemoveControl(unsigned int i)
{
	Control::RemoveControl(i);

	m_ScrollBar->m_nEnd = (int)ceilf(m_Childs.size() / (float)m_ItemColumn);
}

void ListBox::ClearAllControl(void)
{
	Control::ClearAllControl();

	m_ScrollBar->m_nEnd = 0;
}

Control * ListBox::GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible)
{
	if (bIgnoreVisible || m_bEnabled && m_bVisible)
	{
		for (int i = m_ScrollBar->m_nPosition + m_ScrollBar->m_nPageSize - 1; i >= m_ScrollBar->m_nPosition; i--)
		{
			const int column = Min(m_ItemColumn, (int)m_Childs.size() - i * m_ItemColumn);
			for (int j = column - 1; j >= 0; j--)
			{
				Control* ctrl = m_Childs[i * m_ItemColumn + j]->GetChildAtPoint(pt, bIgnoreVisible);
				if (ctrl)
				{
					return ctrl;
				}
			}
		}

		Control* ctrl = m_ScrollBar->GetChildAtPoint(pt, bIgnoreVisible);
		if (ctrl)
		{
			return ctrl;
		}

		if (HitTest(pt))
		{
			return this;
		}
	}
	return NULL;
}

void ListBox::GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self)
{
	if (m_bEnabled && m_bVisible)
	{
		_ASSERT(!recursive_self || ContainsControl(recursive_self) && recursive_self != this);

		Vector2 cent = rect.Center();
		int start_i, end_i;
		float y = 0;
		if (cent.y < m_Rect.t)
		{
			start_i = 0;
			end_i = 1;
			y = m_Rect.t;
		}
		else if (cent.y > m_Rect.b)
		{
			start_i = m_ScrollBar->m_nEnd - 1;
			end_i = m_ScrollBar->m_nEnd;
			y = m_Rect.t + (m_ScrollBar->m_nPageSize - 1) * m_ItemSize.y;
		}
		else
		{
			start_i = m_ScrollBar->m_nPosition - 1;
			end_i = m_ScrollBar->m_nPosition + m_ScrollBar->m_nPageSize + 1;
			y = m_Rect.t - m_ItemSize.y;
		}

		for (int i = Max(0, start_i); i < end_i; i++)
		{
			const int column = Min(m_ItemColumn, (int)m_Childs.size() - i * m_ItemColumn);
			for (int j = 0; j < column; j++)
			{
				ControlPtr& ctrl = m_Childs[i * m_ItemColumn + j];
				if (ctrl->CanHaveFocus())
				{
					ctrl->m_Rect = Rectangle::LeftTop(m_Rect.l + j * m_ItemSize.x, y + (i - start_i) * m_ItemSize.y, m_ItemSize.x, m_ItemSize.y);
					float dist = _GetNearestDist(rect, ctrl->m_Rect, dir);
					if (dist > 0 && dist < nearest_ctrl_dist)
					{
						*nearest_ctrl = ctrl.get();
						nearest_ctrl_dist = dist;
					}
				}
			}
		}

		if (!*nearest_ctrl && recursive_self && m_Parent)
		{
			m_Parent->GetNearestControl(rect, dir, nearest_ctrl, nearest_ctrl_dist, this);
		}
	}
}

void ListBox::SetScrollbarWidth(float Width)
{
	m_ScrollbarWidth = Width;

	OnLayout();
}

float ListBox::GetScrollbarWidth(void) const
{
	return m_ScrollbarWidth;
}

void ListBox::SetScrollbarUpDownBtnHeight(float Height)
{
	m_ScrollbarUpDownBtnHeight = Height;

	OnLayout();
}

float ListBox::GetScrollbarUpDownBtnHeight(void) const
{
	return m_ScrollbarUpDownBtnHeight;
}

void ListBox::SetItemSize(const my::Vector2 & ItemSize)
{
	m_ItemSize = ItemSize;

	OnLayout();
}

const my::Vector2 & ListBox::GetItemSize(void) const
{
	return m_ItemSize;
}

void DialogSkin::RequestResource(void)
{
	ControlSkin::RequestResource();
}

void DialogSkin::ReleaseResource(void)
{
	ControlSkin::ReleaseResource();
}

Dialog::~Dialog(void)
{
	if (m_Manager)
	{
		_ASSERT(false); m_Manager->RemoveDlg(this);
	}
}

void Dialog::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size)
{
	_ASSERT(m_Manager);

	Control::Draw(ui_render, fElapsedTime, Offset, Size);
}

bool Dialog::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Dialog::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Dialog::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (HitTest(pt))
			{
				m_MouseOffset.x = pt.x - m_x.offset;
				m_MouseOffset.y = pt.y - m_y.offset;
				
				return Control::HandleMouse(uMsg, pt, wParam, lParam);
			}
			break;

		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				if (m_bMouseDrag)
				{
					m_bMouseDrag = false;

					SetCaptureControl(NULL);
					m_bPressed = false;

					if (m_Skin && m_Skin->m_MouseClickSound)
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound);
					}

					return true;
				}
				return Control::HandleMouse(uMsg, pt, wParam, lParam);
			}
			break;

		case WM_MOUSEMOVE:
			if (m_bPressed)
			{
				if (m_EnableDrag && !m_bMouseDrag)
				{
					m_bMouseDrag = true;
				}

				if (m_bMouseDrag)
				{
					m_x.offset = pt.x - m_MouseOffset.x;
					m_y.offset = pt.y - m_MouseOffset.y;
				}
			}
			break;
		}
	}
	return false;
}

void Dialog::SetVisible(bool bVisible)
{
	if (m_bVisible != bVisible)
	{
		Control::SetVisible(bVisible);

		if (m_bVisible)
		{
			if (my::Control::s_FocusControl)
			{
				m_LastFocusControl = my::Control::s_FocusControl->shared_from_this();
			}
			SetFocused(true);
		}
		else if (m_Manager && !my::Control::s_FocusControl)
		{
			ControlPtr control = m_LastFocusControl.lock();
			m_LastFocusControl.reset();
			Control* ctrl = control.get();
			for (; ctrl; ctrl = ctrl->m_Parent)
			{
				if (!ctrl->GetVisible())
				{
					break;
				}
			}
			if (control && !ctrl)
			{
				control->SetFocused(true);
			}
			else
			{
				DialogMgr::DialogList::reverse_iterator dlg_iter = m_Manager->m_DlgList.rbegin();
				for (; dlg_iter != m_Manager->m_DlgList.rend(); dlg_iter++)
				{
					(*dlg_iter)->SetFocused(true);

					if (Control::s_FocusControl)
					{
						return;
					}
				}
			}
		}
	}
}

bool Dialog::RayToWorld(const Ray & ray, Vector2 & ptWorld) const
{
	Vector3 dialogNormal = m_World[2].xyz.normalize();
	float dialogDist = m_World[3].xyz.dot(dialogNormal);
	RayResult result = IntersectionTests::rayAndHalfSpace(ray.p, ray.d, Plane::NormalDistance(dialogNormal, dialogDist));
	if (result.first)
	{
		Vector3 ptInt(ray.p + ray.d * result.second);
		ptWorld = ptInt.transformCoord(m_World.inverse()).xy;
		return true;
	}
	return false;
}

void Dialog::GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self)
{
	Control::GetNearestControl(rect, dir, nearest_ctrl, nearest_ctrl_dist, recursive_self);

	if (m_bEnabled && m_bVisible && !*nearest_ctrl && m_Manager)
	{
		switch (dir)
		{
		case VK_UP:
			Control::GetNearestControl(rect.offset(Vector2(0, m_Manager->GetDlgViewport().y)), dir, nearest_ctrl, nearest_ctrl_dist, NULL);
			break;
		case VK_DOWN:
			Control::GetNearestControl(rect.offset(Vector2(0, -m_Manager->GetDlgViewport().y)), dir, nearest_ctrl, nearest_ctrl_dist, NULL);
			break;
		case VK_LEFT:
			Control::GetNearestControl(rect.offset(Vector2(m_Manager->GetDlgViewport().x, 0)), dir, nearest_ctrl, nearest_ctrl_dist, NULL);
			break;
		case VK_RIGHT:
			Control::GetNearestControl(rect.offset(Vector2(-m_Manager->GetDlgViewport().x, 0)), dir, nearest_ctrl, nearest_ctrl_dist, NULL);
			break;
		}
	}
}

void Dialog::MoveToFront(void) const
{
	if (m_Manager)
	{
		DialogMgr::DialogList::const_iterator self_iter = std::find(m_Manager->m_DlgList.begin(), m_Manager->m_DlgList.end(), this);
		_ASSERT(self_iter != m_Manager->m_DlgList.end());
		m_Manager->m_DlgList.splice(m_Manager->m_DlgList.end(), m_Manager->m_DlgList, self_iter, std::next(self_iter, 1));
	}
}

void DialogMgr::SetDlgViewport(const Vector2 & Viewport, float fov)
{
	m_Eye = Vector3(Viewport.x * 0.5f, Viewport.y * 0.5f, -Viewport.y * 0.5f * cotf(fov / 2));

	m_View = Matrix4::LookAtRH(m_Eye, Vector3(m_Eye.x, m_Eye.y, 0), Vector3(0, -1, 0));

	m_Proj = Matrix4::PerspectiveFovRH(fov, Viewport.x / Viewport.y, 0.1f, 3000.0f);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();

	DialogList::iterator dlg_iter = m_DlgList.begin();
	for(; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		if((*dlg_iter)->m_EventAlign)
		{
			ControlEventArg arg(*dlg_iter);
			(*dlg_iter)->m_EventAlign(&arg);
		}
	}
}

Ray DialogMgr::CalculateRay(const Vector2 & pt, const Vector2 & dim)
{
	Vector3 At = Vector3(Lerp(-1.0f, 1.0f, pt.x / dim.x), Lerp(1.0f, -1.0f, pt.y / dim.y), 0.0f).transformCoord(m_InverseViewProj);

	return Ray(m_Eye, (At - m_Eye).normalize());
}

Vector2 DialogMgr::GetDlgViewport(void) const
{
	return Vector2(-m_View._41*2, m_View._42*2);
}

void DialogMgr::Draw(UIRender * ui_render, double fTime, float fElapsedTime, const Vector2 & Viewport)
{
	UIPassObjList::iterator obj_iter = m_UIPassObjs.begin();
	for (; obj_iter != m_UIPassObjs.end(); obj_iter++)
	{
		(*obj_iter)(ui_render, fElapsedTime, Viewport);
	}
	m_UIPassObjs.clear();

	DialogList::iterator dlg_iter = m_DlgList.begin();
	for(; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		ui_render->SetWorld((*dlg_iter)->m_World);

		(*dlg_iter)->Draw(ui_render, fElapsedTime, Vector2(0, 0), Viewport);

		ui_render->Flush();
	}
}

bool DialogMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Control::s_FocusControl)
	{
		if (Control::s_FocusControl->MsgProc(hWnd, uMsg, wParam, lParam))
		{
			return true;
		}
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			Control * ControlPtd = Control::s_FocusControl;
			for (; ControlPtd; ControlPtd = ControlPtd->m_Parent)
			{
				if (ControlPtd->HandleKeyboard(uMsg, wParam, lParam))
				{
					return true;
				}
			}
	
			if (uMsg == WM_KEYDOWN)
			{
				DialogList::reverse_iterator dlg_iter = m_DlgList.rbegin();
				for (; dlg_iter != m_DlgList.rend(); dlg_iter++)
				{
					Control::ControlPtrList::iterator ctrl_iter = (*dlg_iter)->m_Childs.begin();
					for (; ctrl_iter != (*dlg_iter)->m_Childs.end(); ctrl_iter++)
					{
						if ((*ctrl_iter)->GetVisibleHierarchy() && (*ctrl_iter)->GetEnabledHierarchy() && (*ctrl_iter)->GetHotkey() == wParam)
						{
							(*ctrl_iter)->OnHotkey();
							return true;
						}
					}
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			CPoint point((short)LOWORD(lParam), (short)HIWORD(lParam));
			if (uMsg == WM_MOUSEWHEEL)
			{
				ScreenToClient(hWnd, &point);
			}
			Ray ray = CalculateRay(Vector2(point.x + 0.5f, point.y + 0.5f), Vector2(ClientRect.Width(), ClientRect.Height()));

			if (Control::s_CaptureControl)
			{
				Vector2 pt;
				if (Control::s_CaptureControl->RayToWorld(ray, pt))
				{
					if (Control::s_CaptureControl->HandleMouse(uMsg, pt, wParam, lParam))
					{
						return true;
					}
				}
				break;
			}

			bool bFindMouseOver = false;
			DialogList::reverse_iterator dlg_iter = m_DlgList.rbegin();
			for(; dlg_iter != m_DlgList.rend(); dlg_iter++)
			{
				// ! 只处理看得见的 Dialog
				if((*dlg_iter)->GetVisible())
				{
					Vector2 pt;
					if ((*dlg_iter)->RayToWorld(ray, pt))
					{
						Control * ControlPtd = (*dlg_iter)->GetChildAtPoint(pt, false);
						if (!bFindMouseOver && ControlPtd)
						{
							Control::SetMouseOverControl(ControlPtd, pt);
							bFindMouseOver = true;
						}
						for (; ControlPtd; ControlPtd = ControlPtd->m_Parent)
						{
							if (ControlPtd->HitTest(pt) && ControlPtd->HandleMouse(uMsg, pt, wParam, lParam))
							{
								return true;
							}
						}
					}
				}
			}

			if (!bFindMouseOver && Control::s_MouseOverControl)
			{
				Control::SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
			}

			// ! WM_RBUTTONDOWN + Mouse::Unacquire may cause Visual Studio IDE input issue
			if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN /*|| uMsg == WM_RBUTTONDOWN */|| uMsg == WM_XBUTTONDOWN) && !bFindMouseOver && Control::s_FocusControl)
			{
				Control::SetFocusControl(NULL);
			}
		}
		break;
	}

	return false;
}

void DialogMgr::InsertDlg(Dialog * dlg)
{
	_ASSERT(!dlg->m_Manager);

	m_DlgList.push_back(dlg);

	dlg->m_Manager = this;

	if (!dlg->IsRequested())
	{
		dlg->RequestResource();
	}

	if(dlg->m_EventAlign)
	{
		ControlEventArg arg(dlg);
		dlg->m_EventAlign(&arg);
	}
}

void DialogMgr::RemoveDlg(Dialog * dlg)
{
	DialogList::iterator dlg_iter = std::find(m_DlgList.begin(), m_DlgList.end(), dlg);
	if(dlg_iter != m_DlgList.end())
	{
		_ASSERT((*dlg_iter)->m_Manager == this);

		if (Control::s_FocusControl && (*dlg_iter)->ContainsControl(Control::s_FocusControl))
		{
			Control::SetFocusControl(NULL);
		}

		(*dlg_iter)->m_Manager = NULL;

		if ((*dlg_iter)->IsRequested())
		{
			(*dlg_iter)->ReleaseResource();
		}

		m_DlgList.erase(dlg_iter);
	}
	else
	{
		_ASSERT(false);
	}
}

void DialogMgr::RemoveAllDlg()
{
	DialogList::iterator dlg_iter = m_DlgList.begin();
	for (; dlg_iter != m_DlgList.end(); dlg_iter = m_DlgList.begin())
	{
		RemoveDlg(*dlg_iter);
	}
}
