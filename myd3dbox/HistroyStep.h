#pragma once

#include "../demo2_3/Component/Actor.h"

class HistoryStep
{
public:
	class Operator
	{
	public:
		Operator(void)
		{
		}

		virtual ~Operator(void)
		{
		}

		virtual void Do(void) = 0;
	};

	typedef boost::shared_ptr<Operator> OperatorPtr;

	typedef std::pair<OperatorPtr, OperatorPtr> OperatorPair;

	typedef std::vector<OperatorPair> OperatorPairList;

	OperatorPairList m_Ops;

public:
	HistoryStep(void)
	{
	}

	virtual ~HistoryStep(void)
	{
	}

	void Do(void);

	void Undo(void);
};

typedef boost::shared_ptr<HistoryStep> HistoryStepPtr;

class HistroyAddComponent : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	ComponentPtr m_cmp;

	HistroyAddComponent(ComponentLevel * level, ComponentPtr cmp)
		: m_cmp(cmp)
	{
	}

	virtual void Do(void);
};

class HistroyRemoveComponent : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	ComponentPtr m_cmp;

	HistroyRemoveComponent(ComponentLevel * level, ComponentPtr cmp)
		: m_cmp(cmp)
	{
	}

	virtual void Do(void);
};
