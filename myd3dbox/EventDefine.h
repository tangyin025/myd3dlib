#pragma once

struct EventArgs
{
public:
	EventArgs(void)
	{
	}

	virtual ~EventArgs(void)
	{
	}
};

typedef boost::signals2::signal<void (EventArgs *)> Event;
