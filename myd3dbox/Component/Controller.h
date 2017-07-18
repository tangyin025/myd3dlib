#pragma once

#include "character.h"

class Controller;

typedef boost::shared_ptr<Controller> ControllerPtr;

class ControllerMgr
{
public:
	typedef std::set<ControllerPtr> ControllerPtrSet;

	ControllerPtrSet m_controllers;

public:
	ControllerMgr(void)
	{
	}
};

class Controller
{
public:
	Controller(void);

	virtual ~Controller(void);
};

class PlayerController
	: public Controller
{
public:
	PlayerController(void);

	virtual ~PlayerController(void);

	void OnMouseMove(my::InputEventArg * arg);

	void OnMouseBtnDown(my::InputEventArg * arg);

	void OnMouseBtnUp(my::InputEventArg * arg);

	void OnKeyDown(my::InputEventArg * arg);

	void OnKeyUp(my::InputEventArg * arg);

	void OnJoystickAxisMove(my::InputEventArg * arg);

	void OnJoystickPovMove(my::InputEventArg * arg);

	void OnJoystickBtnDown(my::InputEventArg * arg);

	void OnJoystickBtnUp(my::InputEventArg * arg);
};
