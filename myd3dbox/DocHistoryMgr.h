#pragma once

class CDocStepBase
{
public:
	CDocStepBase(void)
	{
	}

	virtual ~CDocStepBase(void)
	{
	}

	virtual void Do(void);
};

typedef boost::shared_ptr<CDocStepBase> CDocStepBasePtr;

class CDocHistory
	: protected std::vector<std::pair<CDocStepBasePtr, CDocStepBasePtr> >
{
public:
	CDocHistory(void)
	{
	}

	virtual ~CDocHistory(void)
	{
	}

	void Do(void);

	void Undo(void);
};

typedef boost::shared_ptr<CDocHistory> CDocHistoryPtr;

class CDocHistoryMgr
	: protected std::deque<CDocHistoryPtr>
{
public:
	CDocHistoryMgr(void)
	{
	}

	virtual ~CDocHistoryMgr(void)
	{
	}
};
