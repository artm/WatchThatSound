#pragma once

#include <QObject>

namespace WTS {

/** Project data container.
 * Eventually all project specific data should end up here.
 */
class Project : public QObject {
    Q_OBJECT
public:

    Project(QObject * parent = 0);
};

}
