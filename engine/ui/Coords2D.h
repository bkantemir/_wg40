#pragma once

class Coords2D
{
public:
	float pos[4] = { 0,0,0,0 }; //x,y,z position + 4-th element for compatibility with 3D 4x4 matrices math
	float aZdg = 0; //in degrees
public:
};

