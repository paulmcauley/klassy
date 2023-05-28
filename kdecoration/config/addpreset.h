#ifndef ADDPRESET_H
#define ADDPRESET_H

#include "ui_addpreset.h"
#include <QDialog>

namespace Breeze
{

class AddPreset : public QDialog
{
    Q_OBJECT

    friend class LoadPreset;

public:
    explicit AddPreset(QWidget *parent = nullptr);
    ~AddPreset();

private:
    Ui_AddPreset *m_ui;
};

}

#endif // ADDPRESET_H
