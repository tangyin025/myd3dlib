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
