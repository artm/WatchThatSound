#include "Preferences.h"
#include "ui_Preferences.h"

using namespace WTS;

Preferences::Preferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
{
    ui->setupUi(this);
}

Preferences::~Preferences()
{
    delete ui;
}
