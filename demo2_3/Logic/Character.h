#pragma once

class Character
{
public:
	Character(void);

	virtual ~Character(void);
};

typedef boost::shared_ptr<Character> CharacterPtr;
