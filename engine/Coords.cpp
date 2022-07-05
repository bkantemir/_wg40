#include "Coords.h"
#include "platform.h"
#include <string>

float PI = 3.141592f;
float degrees2radians = PI / 180.0f;
float radians2degrees = 180.0f / PI;

void Coords::getPositionFromMatrix(float* pos, mat4x4 m) {
	//extracts position from matrix
	for (int i = 0; i < 3; i++)
		pos[i] = m[i][3];
}
void Coords::eulerDgToMatrix(mat4x4 rotationMatrix, float* eulerDg) {
	//builds rotation matrix from Euler angles (in degreed)
	mat4x4_identity(rotationMatrix);
	//rotation order: Z-X-Y
	float a = eulerDg[1];
	if (a != 0)
		mat4x4_rotate_Y(rotationMatrix, rotationMatrix, a * degrees2radians);
	a = eulerDg[0];
	if (a != 0)
		mat4x4_rotate_X(rotationMatrix, rotationMatrix, a * degrees2radians);
	a = eulerDg[2];
	if (a != 0)
		mat4x4_rotate_Z(rotationMatrix, rotationMatrix, a * degrees2radians);
}
void Coords::matrixToEulerDg(float* eulerDg, mat4x4 m) {
	//calculates Euler angles (in degrees) from matrix
	float yaw, pitch, roll; //in redians

	if (m[1][2] > 0.998 || m[1][2] < -0.998) { // singularity at south or north pole
		yaw = atan2f(-m[2][0], m[0][0]);
		roll = 0;
	}
	else {
		yaw = atan2f(-m[0][2], m[2][2]);
		roll = atan2f(-m[1][0], m[1][1]);
	}
	pitch = asinf(m[1][2]);

	eulerDg[0] = pitch * radians2degrees;
	eulerDg[1] = yaw * radians2degrees;
	eulerDg[2] = roll * radians2degrees;
}