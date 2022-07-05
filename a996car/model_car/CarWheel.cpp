#include "CarWheel.h"

int CarWheel::moveSubj(CarWheel* pGS) {
    float wheelRotationSpeedDg = 3;
    if (abs(pGS->ownCoords.eulerDg[1]) > 90)
        wheelRotationSpeedDg = -wheelRotationSpeedDg;
    pGS->ownSpeed.eulerDg[0] = wheelRotationSpeedDg;
    applySpeeds(pGS);
    return 1;
}

