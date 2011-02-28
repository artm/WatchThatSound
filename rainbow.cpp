#include "rainbow.h"

QColor Rainbow::getColor(int i, int a)
{
    return QColor::fromHsl( abs(i) * 31 % 360, 200, 200, a);
}
