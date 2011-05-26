#ifndef RAINBOW_H
#define RAINBOW_H

#include <QColor>

namespace WTS {

class Rainbow
{
public:
    static QColor getColor(int i, int a = 255, int l = 200);
};

}

#endif // RAINBOW_H
