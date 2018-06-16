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

protected:
	CharacterController(void)
	{
	}

public:
	CharacterController(Actor * actor)
		: Controller(actor)
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
