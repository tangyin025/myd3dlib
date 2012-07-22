#pragma once

#include "Game.h"

typedef boost::statechart::event_base GameEventBase;

class GameEventLoadOver : public boost::statechart::event<GameEventLoadOver>
{
};

class GameStatePlay;

class GameStateLoad
	: public GameStateBase
	, public boost::statechart::simple_state<GameStateLoad, Game>
{
public:
	typedef boost::statechart::transition<GameEventLoadOver, GameStatePlay> reactions;

	GameStateLoad(void);

	~GameStateLoad(void);

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnD3D9LostDevice(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);
};

class GameStatePlay
	: public GameStateBase
	, public boost::statechart::simple_state<GameStatePlay, Game>
{
public:
	GameStatePlay(void);

	~GameStatePlay(void);

	virtual HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnD3D9LostDevice(void);

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime);

	virtual void OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing);
};
