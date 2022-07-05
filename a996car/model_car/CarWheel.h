#pragma once
#include "GameSubj.h"

class CarWheel : public GameSubj
{
public:
	float wheelDiameter = 20;
public:
	virtual GameSubj* clone() { return new CarWheel(*this); };
	virtual int moveSubj() { return moveSubj(this); };
	static int moveSubj(CarWheel* pGS);
};