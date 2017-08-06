#pragma once

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

	void AddController(ControllerPtr controller);

	void RemoveController(ControllerPtr controller);

	void ClearAllControllers(void);

	void Update(float fElapsedTime);
};

class Actor;

class Controller
{
public:
	Actor * m_Actor;

public:
	Controller(Actor * actor);

	virtual ~Controller(void);

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<Controller> ControllerPtr;

class PlayerController
	: public Controller
{
public:
	PlayerController(Actor * actor);

	virtual ~PlayerController(void);

	virtual void Update(float fElapsedTime);

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

typedef boost::shared_ptr<PlayerController> PlayerControllerPtr;
