#pragma once

#include "../demo2_3/Component/Actor.h"

class HistoryStep;

typedef boost::shared_ptr<HistoryStep> HistoryStepPtr;

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

	typedef std::list<ComponentPtr> ComponentPtrList;

public:
	HistoryStep(void)
	{
	}

	virtual ~HistoryStep(void)
	{
	}

	void Do(void);

	void Undo(void);

	static HistoryStepPtr AddComponentList(ComponentLevel * level, ComponentPtrList cmp_list);

	static HistoryStepPtr RemoveComponentList(ComponentLevel * level, ComponentPtrList cmp_list);
};

class OperatorAddComponentList : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	HistoryStep::ComponentPtrList m_cmp_list;

	OperatorAddComponentList(ComponentLevel * level, HistoryStep::ComponentPtrList cmp_list)
		: m_level(level)
		, m_cmp_list(cmp_list)
	{
	}

	virtual void Do(void);
};

class OperatorRemoveComponentList : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	HistoryStep::ComponentPtrList m_cmp_list;

	OperatorRemoveComponentList(ComponentLevel * level, HistoryStep::ComponentPtrList cmp_list)
		: m_level(level)
		, m_cmp_list(cmp_list)
	{
	}

	virtual void Do(void);
};
