#pragma once

class HistoryStepBase
{
public:
	HistoryStepBase(void)
	{
	}

	virtual ~HistoryStepBase(void)
	{
	}

	virtual void Do(void);
};

typedef boost::shared_ptr<HistoryStepBase> HistoryStepBasePtr;

class History
	: protected std::vector<std::pair<HistoryStepBasePtr, HistoryStepBasePtr> >
{
public:
	History(void)
	{
	}

	virtual ~History(void)
	{
	}

	void Do(void);

	void Undo(void);
};

typedef boost::shared_ptr<History> HistoryPtr;

class CDocHistoryMgr
	: protected std::deque<HistoryPtr>
	, public my::SingleInstance<CDocHistoryMgr>
{
public:
	CDocHistoryMgr(void)
	{
	}

	virtual ~CDocHistoryMgr(void)
	{
	}
};
