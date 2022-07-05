#pragma once

class Triangle01
{
public:
	char marks[128] = "";
	int subjN = -1; //game subject number
	int materialN = -1; //material number
	int flag = 0;
	int idx[3] = { 0,0,0 }; //3 vertex indices
	//for normals calculation
	float normal[3] = { 0,0,0 };
	int matchingNormalsN = 0;
	float triangleArea = 0;
};

