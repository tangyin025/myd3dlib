#pragma once

class Controller
{
protected:
	friend class Actor;

	Actor * m_Actor;

protected:
	Controller(void)
		: m_Actor(NULL)
	{
	}

public:
	Controller(Actor * actor)
		: m_Actor(actor)
	{
	}

	virtual ~Controller(void)
	{
	}

	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}
};

typedef boost::shared_ptr<Controller> ControllerPtr;

class CharacterController
	: public Controller
{
public:
	float m_MoveOrientation;

	float m_MoveAcceleration;

	float m_RotationSpeed;

protected:
	CharacterController(void)
		: m_MoveOrientation(0)
		, m_MoveAcceleration(0)
		, m_RotationSpeed(D3DX_PI*3)
	{
	}

public:
	CharacterController(Actor * actor)
		: Controller(actor)
		, m_MoveOrientation(0)
		, m_MoveAcceleration(0)
		, m_RotationSpeed(D3DX_PI * 3)
	{
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Controller);
	}

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<CharacterController> CharacterControllerPtr;
