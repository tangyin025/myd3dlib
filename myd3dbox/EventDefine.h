#pragma once

struct EventArg
{
public:
	EventArg(void)
	{
	}

	virtual ~EventArg(void)
	{
	}
};

typedef boost::signals2::signal<void (EventArg *)> Event;
