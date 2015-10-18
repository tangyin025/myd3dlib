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

class OperatorAddComponent : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	ComponentPtr m_cmp;

	OperatorAddComponent(ComponentLevel * level, ComponentPtr cmp)
		: m_level(level)
		, m_cmp(cmp)
	{
	}

	virtual void Do(void);
};

class OperatorRemoveComponent : public HistoryStep::Operator
{
public:
	ComponentLevel * m_level;

	ComponentPtr m_cmp;

	OperatorRemoveComponent(ComponentLevel * level, ComponentPtr cmp)
		: m_level(level)
		, m_cmp(cmp)
	{
	}

	virtual void Do(void);
};

class OperatorMatrix4Property : public HistoryStep::Operator
{
public:
	my::Matrix4 & m_OldValue;

	my::Matrix4 m_NewValue;

	OperatorMatrix4Property(my::Matrix4 & OldValue, const my::Matrix4 & NewValue)
		: m_OldValue(OldValue)
		, m_NewValue(NewValue)
	{
	}

	virtual void Do(void);
};

class OperatorComponentWorld : public OperatorMatrix4Property
{
public:
	ComponentLevel * m_level;

	ComponentPtr m_cmp;

	OperatorComponentWorld(ComponentLevel * level, ComponentPtr cmp, const my::Matrix4 & NewValue)
		: OperatorMatrix4Property(cmp->m_World, NewValue)
		, m_level(level)
		, m_cmp(cmp)
	{
	}

	virtual void Do(void);
};
