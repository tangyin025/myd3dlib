#pragma once

#include "RenderPipeline.h"
#include "Component.h"
#include "Animator.h"

class Actor
{
public:
	Actor(void)
	{
	}

	virtual ~Actor(void)
	{
	}

	void Update(float fElapsedTime);
};

typedef boost::shared_ptr<Actor> ActorPtr;

typedef std::vector<ActorPtr> ActorPtrList;
