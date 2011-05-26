#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QSettings>

namespace Ui {
    class Preferences;
}

namespace WTS {

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();
    Ui::Preferences *ui;
};

}

#endif // PREFERENCES_H
