#pragma once

class Actor;

class Controller
{
public:
	Actor * m_Actor;

public:
	Controller(void);

	virtual ~Controller(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	}

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<Controller> ControllerPtr;

class PlayerController
	: public Controller
{
public:
	my::Vector3 m_LookAngles;

	my::Vector3 m_LookDir;

	float m_LookDist;

	float m_FaceAngle;

	float m_FaceAngleInerp;

	int m_InputLtRt;

	int m_InputUpDn;

public:
	PlayerController(void);

	virtual ~PlayerController(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Controller);
	}

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
