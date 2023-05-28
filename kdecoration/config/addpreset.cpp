#include "addpreset.h"

namespace Breeze
{

AddPreset::AddPreset(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui_AddPreset)
{
    m_ui->setupUi(this);
}

AddPreset::~AddPreset()
{
    delete m_ui;
}

}
