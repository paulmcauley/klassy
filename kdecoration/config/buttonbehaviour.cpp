/*
 * SPDX-FileCopyrightText: 2023-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "buttonbehaviour.h"
#include "breezeconfigwidget.h"
#include "dbusmessages.h"
#include "presetsmodel.h"
#include <QPushButton>

namespace Breeze
{

ButtonBehaviour::ButtonBehaviour(KSharedConfig::Ptr config, KSharedConfig::Ptr presetsConfig, QObject *parent)
    : QDialog(static_cast<ConfigWidget *>(parent)->widget())
    , m_ui(new Ui_ButtonBehaviour)
    , m_configuration(config)
    , m_presetsConfiguration(presetsConfig)
    , m_parent(parent)
{
    m_ui->setupUi(this);

    // create the table columns and rows
    int numColumns = 3;
    int numRows = 4;
    for (int columnIndex = 0; columnIndex < numColumns; columnIndex++) {
        m_ui->buttonBehaviourTableActive->insertColumn(columnIndex);
        m_ui->closeButtonBehaviourTableActive->insertColumn(columnIndex);

        m_ui->buttonBehaviourTableInactive->insertColumn(columnIndex);
        m_ui->closeButtonBehaviourTableInactive->insertColumn(columnIndex);
    }
    for (int rowIndex = 0; rowIndex < numRows; rowIndex++) {
        m_ui->buttonBehaviourTableActive->insertRow(rowIndex);
        m_ui->closeButtonBehaviourTableActive->insertRow(rowIndex);

        m_ui->buttonBehaviourTableInactive->insertRow(rowIndex);
        m_ui->closeButtonBehaviourTableInactive->insertRow(rowIndex);
    }

    QStringList horizontalHeadersNormal = {i18n("Icons"), i18n("Backgrounds"), i18n("Outlines")};
    QStringList horizontalHeadersClose = {i18n("Close\nIcon"), i18n("Close\nBackground"), i18n("Close\nOutline")};
    QStringList verticalHeaders = {i18n("Show normally"), i18n("Show on hover"), i18n("Show on press"), i18n("Vary colour on state change")};

    m_ui->buttonBehaviourTableActive->setHorizontalHeaderLabels(horizontalHeadersNormal);
    m_ui->closeButtonBehaviourTableActive->setHorizontalHeaderLabels(horizontalHeadersClose);
    m_ui->buttonBehaviourTableActive->setVerticalHeaderLabels(verticalHeaders);
    m_ui->closeButtonBehaviourTableActive->setVerticalHeaderLabels(verticalHeaders);
    m_ui->buttonBehaviourTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->buttonBehaviourTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->closeButtonBehaviourTableActive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->closeButtonBehaviourTableActive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    m_ui->buttonBehaviourTableInactive->setHorizontalHeaderLabels(horizontalHeadersNormal);
    m_ui->closeButtonBehaviourTableInactive->setHorizontalHeaderLabels(horizontalHeadersClose);
    m_ui->buttonBehaviourTableInactive->setVerticalHeaderLabels(verticalHeaders);
    m_ui->closeButtonBehaviourTableInactive->setVerticalHeaderLabels(verticalHeaders);
    m_ui->buttonBehaviourTableInactive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->buttonBehaviourTableInactive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->closeButtonBehaviourTableInactive->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    m_ui->closeButtonBehaviourTableInactive->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    // set the horizontal header widths to the same as the largest section
    int largestSectionNormal = 0;
    int largestSectionClose = 0;

    for (int columnIndex = 0; columnIndex < numColumns; columnIndex++) {
        int sectionSizeNormal = m_ui->buttonBehaviourTableActive->horizontalHeader()->sectionSize(columnIndex);
        if (sectionSizeNormal > largestSectionNormal)
            largestSectionNormal = sectionSizeNormal;

        int sectionSizeClose = m_ui->closeButtonBehaviourTableActive->horizontalHeader()->sectionSize(columnIndex);
        if (sectionSizeClose > largestSectionClose)
            largestSectionClose = sectionSizeClose;
    }

    m_ui->buttonBehaviourTableActive->horizontalHeader()->setMinimumSectionSize(largestSectionNormal);
    m_ui->closeButtonBehaviourTableActive->horizontalHeader()->setMinimumSectionSize(largestSectionClose);
    m_ui->buttonBehaviourTableInactive->horizontalHeader()->setMinimumSectionSize(largestSectionNormal);
    m_ui->closeButtonBehaviourTableInactive->horizontalHeader()->setMinimumSectionSize(largestSectionClose);

    // create the checkboxes
    for (int i = 0; i < static_cast<int>(TableCheckBox::COUNT); i++) {
        QCheckBox *checkBoxActive = new QCheckBox();
        QCheckBox *checkBoxInactive = new QCheckBox();
        TableCheckBox checkBoxName = static_cast<TableCheckBox>(i);
        m_tableCheckBoxesActive.insert(checkBoxName, checkBoxActive);
        m_tableCheckBoxesInactive.insert(checkBoxName, checkBoxInactive);
    }

    for (int i = 0; i < static_cast<int>(TableCloseCheckBox::COUNT); i++) {
        QCheckBox *checkBoxActive = new QCheckBox();
        QCheckBox *checkBoxInactive = new QCheckBox();
        TableCloseCheckBox checkBoxName = static_cast<TableCloseCheckBox>(i);
        m_tableCloseCheckBoxesActive.insert(checkBoxName, checkBoxActive);
        m_tableCloseCheckBoxesInactive.insert(checkBoxName, checkBoxInactive);
    }

    QStringList varyColorComboBoxItems = {
        i18n("No"),
        i18n("Progressively more opaque"),
        i18n("Most opaque on hover"),
        i18n("Progressively more transparent"),
        i18n("Most transparent on hover"),
        i18n("Progressively lighter"),
        i18n("Lightest on hover"),
        i18n("Progressively darker"),
        i18n("Darkest on hover"),
        i18n("Progressively more titlebar"),
        i18n("Most titlebar on hover"),
        i18n("Progressively less titlebar"),
        i18n("Least titlebar on hover"),
    };
    // create the comboboxes
    for (int i = 0; i < static_cast<int>(TableVaryColorComboBox::COUNT); i++) {
        QComboBox *comboBoxActive = new QComboBox();
        comboBoxActive->addItems(varyColorComboBoxItems);
        QComboBox *comboBoxInactive = new QComboBox();
        comboBoxInactive->addItems(varyColorComboBoxItems);
        TableVaryColorComboBox comboBoxName = static_cast<TableVaryColorComboBox>(i);
        m_tableComboBoxesActive.insert(comboBoxName, comboBoxActive);
        m_tableComboBoxesInactive.insert(comboBoxName, comboBoxInactive);
    }

    for (int i = 0; i < static_cast<int>(TableCloseVaryColorComboBox::COUNT); i++) {
        QComboBox *comboBoxActive = new QComboBox();
        comboBoxActive->addItems(varyColorComboBoxItems);
        QComboBox *comboBoxInactive = new QComboBox();
        comboBoxInactive->addItems(varyColorComboBoxItems);
        TableCloseVaryColorComboBox comboBoxName = static_cast<TableCloseVaryColorComboBox>(i);
        m_tableCloseComboBoxesActive.insert(comboBoxName, comboBoxActive);
        m_tableCloseComboBoxesInactive.insert(comboBoxName, comboBoxInactive);
    }

    // map the checkboxes to the correct table cells
    struct CheckBoxLocation {
        int row;
        int column;
        TableCheckBox checkBox;
    };

    struct CloseCheckBoxLocation {
        int row;
        int column;
        TableCloseCheckBox checkBox;
    };

    QList<CheckBoxLocation> normalCheckboxLocations;
    normalCheckboxLocations << CheckBoxLocation{0, 0, TableCheckBox::showIconNormally};
    normalCheckboxLocations << CheckBoxLocation{0, 1, TableCheckBox::showBackgroundNormally};
    normalCheckboxLocations << CheckBoxLocation{0, 2, TableCheckBox::showOutlineNormally};
    normalCheckboxLocations << CheckBoxLocation{1, 0, TableCheckBox::showIconOnHover};
    normalCheckboxLocations << CheckBoxLocation{1, 1, TableCheckBox::showBackgroundOnHover};
    normalCheckboxLocations << CheckBoxLocation{1, 2, TableCheckBox::showOutlineOnHover};
    normalCheckboxLocations << CheckBoxLocation{2, 0, TableCheckBox::showIconOnPress};
    normalCheckboxLocations << CheckBoxLocation{2, 1, TableCheckBox::showBackgroundOnPress};
    normalCheckboxLocations << CheckBoxLocation{2, 2, TableCheckBox::showOutlineOnPress};

    QList<CloseCheckBoxLocation> closeCheckboxLocations;
    closeCheckboxLocations << CloseCheckBoxLocation{0, 0, TableCloseCheckBox::showCloseIconNormally};
    closeCheckboxLocations << CloseCheckBoxLocation{0, 1, TableCloseCheckBox::showCloseBackgroundNormally};
    closeCheckboxLocations << CloseCheckBoxLocation{0, 2, TableCloseCheckBox::showCloseOutlineNormally};
    closeCheckboxLocations << CloseCheckBoxLocation{1, 0, TableCloseCheckBox::showCloseIconOnHover};
    closeCheckboxLocations << CloseCheckBoxLocation{1, 1, TableCloseCheckBox::showCloseBackgroundOnHover};
    closeCheckboxLocations << CloseCheckBoxLocation{1, 2, TableCloseCheckBox::showCloseOutlineOnHover};
    closeCheckboxLocations << CloseCheckBoxLocation{2, 0, TableCloseCheckBox::showCloseIconOnPress};
    closeCheckboxLocations << CloseCheckBoxLocation{2, 1, TableCloseCheckBox::showCloseBackgroundOnPress};
    closeCheckboxLocations << CloseCheckBoxLocation{2, 2, TableCloseCheckBox::showCloseOutlineOnPress};

    for (auto &normalCheckboxLocation : normalCheckboxLocations) {
        QHBoxLayout *hlayoutActive = new QHBoxLayout();
        QWidget *wActive = new QWidget();
        hlayoutActive->addStretch();
        hlayoutActive->addWidget(m_tableCheckBoxesActive.value(normalCheckboxLocation.checkBox));
        hlayoutActive->addStretch();
        wActive->setLayout(hlayoutActive);
        m_ui->buttonBehaviourTableActive->setCellWidget(normalCheckboxLocation.row, normalCheckboxLocation.column, wActive);

        QHBoxLayout *hlayoutInactive = new QHBoxLayout();
        QWidget *wInactive = new QWidget();
        hlayoutInactive->addStretch();
        hlayoutInactive->addWidget(m_tableCheckBoxesInactive.value(normalCheckboxLocation.checkBox));
        hlayoutInactive->addStretch();
        wInactive->setLayout(hlayoutInactive);
        m_ui->buttonBehaviourTableInactive->setCellWidget(normalCheckboxLocation.row, normalCheckboxLocation.column, wInactive);
    }

    for (auto &closeCheckboxLocation : closeCheckboxLocations) {
        QHBoxLayout *hlayoutActive = new QHBoxLayout();
        QWidget *wActive = new QWidget();
        hlayoutActive->addStretch();
        hlayoutActive->addWidget(m_tableCloseCheckBoxesActive.value(closeCheckboxLocation.checkBox));
        hlayoutActive->addStretch();
        wActive->setLayout(hlayoutActive);
        m_ui->closeButtonBehaviourTableActive->setCellWidget(closeCheckboxLocation.row, closeCheckboxLocation.column, wActive);

        QHBoxLayout *hlayoutInactive = new QHBoxLayout();
        QWidget *wInactive = new QWidget();
        hlayoutInactive->addStretch();
        hlayoutInactive->addWidget(m_tableCloseCheckBoxesInactive.value(closeCheckboxLocation.checkBox));
        hlayoutInactive->addStretch();
        wInactive->setLayout(hlayoutInactive);
        m_ui->closeButtonBehaviourTableInactive->setCellWidget(closeCheckboxLocation.row, closeCheckboxLocation.column, wInactive);
    }

    // map the comboBoxes to the correct table cells
    struct ComboBoxLocation {
        int row;
        int column;
        TableVaryColorComboBox comboBox;
    };

    struct CloseComboBoxLocation {
        int row;
        int column;
        TableCloseVaryColorComboBox comboBox;
    };

    QList<ComboBoxLocation> normalComboBoxLocations;
    normalComboBoxLocations << ComboBoxLocation{3, 0, TableVaryColorComboBox::varyColorIcon};
    normalComboBoxLocations << ComboBoxLocation{3, 1, TableVaryColorComboBox::varyColorBackground};
    normalComboBoxLocations << ComboBoxLocation{3, 2, TableVaryColorComboBox::varyColorOutline};

    QList<CloseComboBoxLocation> closeComboBoxLocations;
    closeComboBoxLocations << CloseComboBoxLocation{3, 0, TableCloseVaryColorComboBox::varyColorCloseIcon};
    closeComboBoxLocations << CloseComboBoxLocation{3, 1, TableCloseVaryColorComboBox::varyColorCloseBackground};
    closeComboBoxLocations << CloseComboBoxLocation{3, 2, TableCloseVaryColorComboBox::varyColorCloseOutline};

    for (auto &normalComboBoxLocation : normalComboBoxLocations) {
        QHBoxLayout *hlayoutActive = new QHBoxLayout();
        QWidget *wActive = new QWidget();
        hlayoutActive->addStretch();
        hlayoutActive->addWidget(m_tableComboBoxesActive.value(normalComboBoxLocation.comboBox));
        hlayoutActive->addStretch();
        wActive->setLayout(hlayoutActive);
        m_ui->buttonBehaviourTableActive->setCellWidget(normalComboBoxLocation.row, normalComboBoxLocation.column, wActive);

        QHBoxLayout *hlayoutInactive = new QHBoxLayout();
        QWidget *wInactive = new QWidget();
        hlayoutInactive->addStretch();
        hlayoutInactive->addWidget(m_tableComboBoxesInactive.value(normalComboBoxLocation.comboBox));
        hlayoutInactive->addStretch();
        wInactive->setLayout(hlayoutInactive);
        m_ui->buttonBehaviourTableInactive->setCellWidget(normalComboBoxLocation.row, normalComboBoxLocation.column, wInactive);
    }

    for (auto &closeComboBoxLocation : closeComboBoxLocations) {
        QHBoxLayout *hlayoutActive = new QHBoxLayout();
        QWidget *wActive = new QWidget();
        hlayoutActive->addStretch();
        hlayoutActive->addWidget(m_tableCloseComboBoxesActive.value(closeComboBoxLocation.comboBox));
        hlayoutActive->addStretch();
        wActive->setLayout(hlayoutActive);
        m_ui->closeButtonBehaviourTableActive->setCellWidget(closeComboBoxLocation.row, closeComboBoxLocation.column, wActive);

        QHBoxLayout *hlayoutInactive = new QHBoxLayout();
        QWidget *wInactive = new QWidget();
        hlayoutInactive->addStretch();
        hlayoutInactive->addWidget(m_tableCloseComboBoxesInactive.value(closeComboBoxLocation.comboBox));
        hlayoutInactive->addStretch();
        wInactive->setLayout(hlayoutInactive);
        m_ui->closeButtonBehaviourTableInactive->setCellWidget(closeComboBoxLocation.row, closeComboBoxLocation.column, wInactive);
    }

    // track ui changes
    auto i = m_tableCheckBoxesActive.begin();
    while (i != m_tableCheckBoxesActive.end()) {
        connect(i.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(i.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromNormalToCloseActive, Qt::ConnectionType::DirectConnection);
        connect(i.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromActiveToInactive, Qt::ConnectionType::DirectConnection);
        ++i;
    }

    auto j = m_tableCloseCheckBoxesActive.begin();
    while (j != m_tableCloseCheckBoxesActive.end()) {
        connect(j.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(j.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromCloseToNormalActive, Qt::ConnectionType::DirectConnection);
        connect(j.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromActiveToInactive, Qt::ConnectionType::DirectConnection);
        ++j;
    }

    auto k = m_tableCheckBoxesInactive.begin();
    while (k != m_tableCheckBoxesInactive.end()) {
        connect(k.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(k.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromNormalToCloseInactive, Qt::ConnectionType::DirectConnection);
        connect(k.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromInactiveToActive, Qt::ConnectionType::DirectConnection);
        ++k;
    }

    auto l = m_tableCloseCheckBoxesInactive.begin();
    while (l != m_tableCloseCheckBoxesInactive.end()) {
        connect(l.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(l.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromCloseToNormalInactive, Qt::ConnectionType::DirectConnection);
        connect(l.value(), &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromInactiveToActive, Qt::ConnectionType::DirectConnection);
        ++l;
    }

    auto m = m_tableComboBoxesActive.begin();
    while (m != m_tableComboBoxesActive.end()) {
        connect(m.value(), qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(m.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromNormalToCloseActive,
                Qt::ConnectionType::DirectConnection);
        connect(m.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromActiveToInactive,
                Qt::ConnectionType::DirectConnection);
        ++m;
    }

    auto n = m_tableCloseComboBoxesActive.begin();
    while (n != m_tableCloseComboBoxesActive.end()) {
        connect(n.value(), qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(n.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromCloseToNormalActive,
                Qt::ConnectionType::DirectConnection);
        connect(n.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromActiveToInactive,
                Qt::ConnectionType::DirectConnection);
        ++n;
    }

    auto o = m_tableComboBoxesInactive.begin();
    while (o != m_tableComboBoxesInactive.end()) {
        connect(o.value(), qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(o.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromNormalToCloseInactive,
                Qt::ConnectionType::DirectConnection);
        connect(o.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromInactiveToActive,
                Qt::ConnectionType::DirectConnection);
        ++o;
    }

    auto p = m_tableCloseComboBoxesInactive.begin();
    while (p != m_tableCloseComboBoxesInactive.end()) {
        connect(p.value(), qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
        connect(p.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromCloseToNormalInactive,
                Qt::ConnectionType::DirectConnection);
        connect(p.value(),
                qOverload<int>(&QComboBox::currentIndexChanged),
                this,
                &ButtonBehaviour::copyComboStatusFromInactiveToActive,
                Qt::ConnectionType::DirectConnection);
        ++p;
    }

    auto setIndexOfOtherIfLocked = [this](QComboBox *other, const int i) {
        if (m_loading || m_processingDefaults || !m_ui->lockButtonBehaviourActive->isChecked())
            return; // only do this for user interactions where the active/inactive lock is checked
        other->setCurrentIndex(i);
    };

    connect(m_ui->buttonStateCheckedActive, qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged);
    connect(m_ui->buttonStateCheckedInactive, qOverload<int>(&QComboBox::currentIndexChanged), this, &ButtonBehaviour::updateChanged);
    connect(
        m_ui->buttonStateCheckedActive,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [=, this](const int i) {
            setIndexOfOtherIfLocked(m_ui->buttonStateCheckedInactive, i);
        },
        Qt::ConnectionType::DirectConnection);
    connect(
        m_ui->buttonStateCheckedInactive,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [=, this](const int i) {
            setIndexOfOtherIfLocked(m_ui->buttonStateCheckedActive, i);
        },
        Qt::ConnectionType::DirectConnection);

    connect(m_ui->lockButtonBehaviourActive, &QAbstractButton::toggled, m_ui->lockButtonBehaviourInactive, &QAbstractButton::setChecked);
    connect(m_ui->lockButtonBehaviourInactive, &QAbstractButton::toggled, m_ui->lockButtonBehaviourActive, &QAbstractButton::setChecked);
    connect(m_ui->lockButtonBehaviourActive, &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockCloseButtonBehaviourActive, &QAbstractButton::toggled, this, &ButtonBehaviour::updateChanged, Qt::ConnectionType::DirectConnection);
    connect(m_ui->lockCloseButtonBehaviourActive, &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromActiveToInactive);
    connect(m_ui->lockCloseButtonBehaviourInactive, &QAbstractButton::toggled, this, &ButtonBehaviour::copyCheckedStatusFromInactiveToActive);

    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &ButtonBehaviour::defaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &ButtonBehaviour::load);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ButtonBehaviour::saveAndReloadKWinConfig);
    setApplyButtonState(false);
}

ButtonBehaviour::~ButtonBehaviour()
{
    delete m_ui;
}

void ButtonBehaviour::loadMain(const bool assignUiValuesOnly)
{
    if (!assignUiValuesOnly) {
        m_loading = true;

        // create internal settings and load from rc files
        m_internalSettings = InternalSettingsPtr(new InternalSettings());
        m_internalSettings->load();
    }

    m_tableCheckBoxesActive.value(TableCheckBox::showIconOnPress)->setChecked(m_internalSettings->showIconOnPress(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showIconOnHover)->setChecked(m_internalSettings->showIconOnHover(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showIconNormally)->setChecked(m_internalSettings->showIconNormally(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnPress)->setChecked(m_internalSettings->showBackgroundOnPress(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnHover)->setChecked(m_internalSettings->showBackgroundOnHover(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundNormally)->setChecked(m_internalSettings->showBackgroundNormally(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnPress)->setChecked(m_internalSettings->showOutlineOnPress(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnHover)->setChecked(m_internalSettings->showOutlineOnHover(true));
    m_tableCheckBoxesActive.value(TableCheckBox::showOutlineNormally)->setChecked(m_internalSettings->showOutlineNormally(true));

    m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorIcon)->setCurrentIndex(m_internalSettings->varyColorIcon(true));
    m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorBackground)->setCurrentIndex(m_internalSettings->varyColorBackground(true));
    m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorOutline)->setCurrentIndex(m_internalSettings->varyColorOutline(true));

    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnPress)->setChecked(m_internalSettings->showCloseIconOnPress(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnHover)->setChecked(m_internalSettings->showCloseIconOnHover(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconNormally)->setChecked(m_internalSettings->showCloseIconNormally(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->setChecked(m_internalSettings->showCloseBackgroundOnPress(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->setChecked(m_internalSettings->showCloseBackgroundOnHover(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundNormally)->setChecked(m_internalSettings->showCloseBackgroundNormally(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnPress)->setChecked(m_internalSettings->showCloseOutlineOnPress(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnHover)->setChecked(m_internalSettings->showCloseOutlineOnHover(true));
    m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineNormally)->setChecked(m_internalSettings->showCloseOutlineNormally(true));

    m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->setCurrentIndex(m_internalSettings->varyColorCloseIcon(true));
    m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)
        ->setCurrentIndex(m_internalSettings->varyColorCloseBackground(true));
    m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->setCurrentIndex(m_internalSettings->varyColorCloseOutline(true));

    m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnPress)->setChecked(m_internalSettings->showIconOnPress(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnHover)->setChecked(m_internalSettings->showIconOnHover(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showIconNormally)->setChecked(m_internalSettings->showIconNormally(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnPress)->setChecked(m_internalSettings->showBackgroundOnPress(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnHover)->setChecked(m_internalSettings->showBackgroundOnHover(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundNormally)->setChecked(m_internalSettings->showBackgroundNormally(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnPress)->setChecked(m_internalSettings->showOutlineOnPress(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnHover)->setChecked(m_internalSettings->showOutlineOnHover(false));
    m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineNormally)->setChecked(m_internalSettings->showOutlineNormally(false));

    m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorIcon)->setCurrentIndex(m_internalSettings->varyColorIcon(false));
    m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorBackground)->setCurrentIndex(m_internalSettings->varyColorBackground(false));
    m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorOutline)->setCurrentIndex(m_internalSettings->varyColorOutline(false));

    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnPress)->setChecked(m_internalSettings->showCloseIconOnPress(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnHover)->setChecked(m_internalSettings->showCloseIconOnHover(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconNormally)->setChecked(m_internalSettings->showCloseIconNormally(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->setChecked(m_internalSettings->showCloseBackgroundOnPress(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->setChecked(m_internalSettings->showCloseBackgroundOnHover(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundNormally)->setChecked(m_internalSettings->showCloseBackgroundNormally(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnPress)->setChecked(m_internalSettings->showCloseOutlineOnPress(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnHover)->setChecked(m_internalSettings->showCloseOutlineOnHover(false));
    m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineNormally)->setChecked(m_internalSettings->showCloseOutlineNormally(false));

    m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->setCurrentIndex(m_internalSettings->varyColorCloseIcon(false));
    m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)
        ->setCurrentIndex(m_internalSettings->varyColorCloseBackground(false));
    m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->setCurrentIndex(m_internalSettings->varyColorCloseOutline(false));

    m_ui->buttonStateCheckedActive->setCurrentIndex(m_internalSettings->buttonStateChecked(true));
    m_ui->buttonStateCheckedInactive->setCurrentIndex(m_internalSettings->buttonStateChecked(false));

    m_ui->lockCloseButtonBehaviourActive->setChecked(m_internalSettings->lockCloseButtonBehaviour(true));
    m_ui->lockCloseButtonBehaviourInactive->setChecked(m_internalSettings->lockCloseButtonBehaviour(false));
    m_ui->lockButtonBehaviourActive->setChecked(m_internalSettings->lockButtonBehaviourActiveInactive());

    if (!assignUiValuesOnly) {
        setChanged(false);

        m_loading = false;
        m_loaded = true;
    }
}

void ButtonBehaviour::save(const bool reloadKwinConfig)
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setShowIconOnPress(true, m_tableCheckBoxesActive.value(TableCheckBox::showIconOnPress)->isChecked());
    m_internalSettings->setShowIconOnHover(true, m_tableCheckBoxesActive.value(TableCheckBox::showIconOnHover)->isChecked());
    m_internalSettings->setShowIconNormally(true, m_tableCheckBoxesActive.value(TableCheckBox::showIconNormally)->isChecked());
    m_internalSettings->setShowBackgroundOnPress(true, m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnPress)->isChecked());
    m_internalSettings->setShowBackgroundOnHover(true, m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnHover)->isChecked());
    m_internalSettings->setShowBackgroundNormally(true, m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundNormally)->isChecked());
    m_internalSettings->setShowOutlineOnPress(true, m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnPress)->isChecked());
    m_internalSettings->setShowOutlineOnHover(true, m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnHover)->isChecked());
    m_internalSettings->setShowOutlineNormally(true, m_tableCheckBoxesActive.value(TableCheckBox::showOutlineNormally)->isChecked());

    m_internalSettings->setVaryColorIcon(true, m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorIcon)->currentIndex());
    m_internalSettings->setVaryColorBackground(true, m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorBackground)->currentIndex());
    m_internalSettings->setVaryColorOutline(true, m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorOutline)->currentIndex());

    m_internalSettings->setShowCloseIconOnPress(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnPress)->isChecked());
    m_internalSettings->setShowCloseIconOnHover(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnHover)->isChecked());
    m_internalSettings->setShowCloseIconNormally(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconNormally)->isChecked());
    m_internalSettings->setShowCloseBackgroundOnPress(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->isChecked());
    m_internalSettings->setShowCloseBackgroundOnHover(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->isChecked());
    m_internalSettings->setShowCloseBackgroundNormally(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundNormally)->isChecked());
    m_internalSettings->setShowCloseOutlineOnPress(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnPress)->isChecked());
    m_internalSettings->setShowCloseOutlineOnHover(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnHover)->isChecked());
    m_internalSettings->setShowCloseOutlineNormally(true, m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineNormally)->isChecked());

    m_internalSettings->setVaryColorCloseIcon(true, m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->currentIndex());
    m_internalSettings->setVaryColorCloseBackground(true,
                                                    m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)->currentIndex());
    m_internalSettings->setVaryColorCloseOutline(true, m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->currentIndex());

    m_internalSettings->setShowIconOnPress(false, m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnPress)->isChecked());
    m_internalSettings->setShowIconOnHover(false, m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnHover)->isChecked());
    m_internalSettings->setShowIconNormally(false, m_tableCheckBoxesInactive.value(TableCheckBox::showIconNormally)->isChecked());
    m_internalSettings->setShowBackgroundOnPress(false, m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnPress)->isChecked());
    m_internalSettings->setShowBackgroundOnHover(false, m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnHover)->isChecked());
    m_internalSettings->setShowBackgroundNormally(false, m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundNormally)->isChecked());
    m_internalSettings->setShowOutlineOnPress(false, m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnPress)->isChecked());
    m_internalSettings->setShowOutlineOnHover(false, m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnHover)->isChecked());
    m_internalSettings->setShowOutlineNormally(false, m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineNormally)->isChecked());

    m_internalSettings->setVaryColorIcon(false, m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorIcon)->currentIndex());
    m_internalSettings->setVaryColorBackground(false, m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorBackground)->currentIndex());
    m_internalSettings->setVaryColorOutline(false, m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorOutline)->currentIndex());

    m_internalSettings->setShowCloseIconOnPress(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnPress)->isChecked());
    m_internalSettings->setShowCloseIconOnHover(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnHover)->isChecked());
    m_internalSettings->setShowCloseIconNormally(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconNormally)->isChecked());
    m_internalSettings->setShowCloseBackgroundOnPress(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->isChecked());
    m_internalSettings->setShowCloseBackgroundOnHover(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->isChecked());
    m_internalSettings->setShowCloseBackgroundNormally(false,
                                                       m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundNormally)->isChecked());
    m_internalSettings->setShowCloseOutlineOnPress(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnPress)->isChecked());
    m_internalSettings->setShowCloseOutlineOnHover(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnHover)->isChecked());
    m_internalSettings->setShowCloseOutlineNormally(false, m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineNormally)->isChecked());

    m_internalSettings->setVaryColorCloseIcon(false, m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->currentIndex());
    m_internalSettings->setVaryColorCloseBackground(
        false,
        m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)->currentIndex());
    m_internalSettings->setVaryColorCloseOutline(false,
                                                 m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->currentIndex());

    m_internalSettings->setButtonStateChecked(true, m_ui->buttonStateCheckedActive->currentIndex());
    m_internalSettings->setButtonStateChecked(false, m_ui->buttonStateCheckedInactive->currentIndex());

    m_internalSettings->setLockCloseButtonBehaviour(true, m_ui->lockCloseButtonBehaviourActive->isChecked());
    m_internalSettings->setLockCloseButtonBehaviour(false, m_ui->lockCloseButtonBehaviourInactive->isChecked());
    m_internalSettings->setLockButtonBehaviourActiveInactive(m_ui->lockButtonBehaviourActive->isChecked());

    m_internalSettings->save();
    setChanged(false);
    Q_EMIT saved();

    if (reloadKwinConfig) {
        DBusMessages::updateDecorationColorCache();
        DBusMessages::kwinReloadConfig();
        // DBusMessages::kstyleReloadDecorationConfig(); //should reload anyway

        static_cast<ConfigWidget *>(m_parent)->generateSystemIcons();
    }
}

void ButtonBehaviour::defaults()
{
    m_processingDefaults = true;
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    loadMain(true);

    setChanged(!isDefaults());

    m_processingDefaults = false;
    m_defaultsPressed = true;
}

bool ButtonBehaviour::isDefaults()
{
    bool isDefaults = true;

    QString groupName(QStringLiteral("ButtonBehaviour"));
    if (m_configuration->hasGroup(groupName)) {
        KConfigGroup group = m_configuration->group(groupName);
        if (group.keyList().count())
            isDefaults = false;
    }

    return isDefaults;
}

void ButtonBehaviour::setChanged(bool value)
{
    m_changed = value;
    setApplyButtonState(value);
    Q_EMIT changed(value);
}

void ButtonBehaviour::accept()
{
    save();
    QDialog::accept();
}

void ButtonBehaviour::reject()
{
    load();
    QDialog::reject();
}

void ButtonBehaviour::updateChanged()
{
    // check configuration
    if (!m_internalSettings)
        return;

    if (m_loading)
        return; // only check if the user has made a change to the UI, or user has pressed defaults

    // track modifications
    bool modified(false);

    if (m_tableCheckBoxesActive.value(TableCheckBox::showIconOnPress)->isChecked() != m_internalSettings->showIconOnPress(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showIconOnHover)->isChecked() != m_internalSettings->showIconOnHover(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showIconNormally)->isChecked() != m_internalSettings->showIconNormally(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnPress)->isChecked() != m_internalSettings->showBackgroundOnPress(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundOnHover)->isChecked() != m_internalSettings->showBackgroundOnHover(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showBackgroundNormally)->isChecked() != m_internalSettings->showBackgroundNormally(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnPress)->isChecked() != m_internalSettings->showOutlineOnPress(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showOutlineOnHover)->isChecked() != m_internalSettings->showOutlineOnHover(true))
        modified = true;
    else if (m_tableCheckBoxesActive.value(TableCheckBox::showOutlineNormally)->isChecked() != m_internalSettings->showOutlineNormally(true))
        modified = true;

    else if (m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorIcon)->currentIndex() != m_internalSettings->varyColorIcon(true))
        modified = true;
    else if (m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorBackground)->currentIndex() != m_internalSettings->varyColorBackground(true))
        modified = true;
    else if (m_tableComboBoxesActive.value(TableVaryColorComboBox::varyColorOutline)->currentIndex() != m_internalSettings->varyColorOutline(true))
        modified = true;

    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnPress)->isChecked() != m_internalSettings->showCloseIconOnPress(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconOnHover)->isChecked() != m_internalSettings->showCloseIconOnHover(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseIconNormally)->isChecked() != m_internalSettings->showCloseIconNormally(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->isChecked()
             != m_internalSettings->showCloseBackgroundOnPress(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->isChecked()
             != m_internalSettings->showCloseBackgroundOnHover(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseBackgroundNormally)->isChecked()
             != m_internalSettings->showCloseBackgroundNormally(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnPress)->isChecked() != m_internalSettings->showCloseOutlineOnPress(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineOnHover)->isChecked() != m_internalSettings->showCloseOutlineOnHover(true))
        modified = true;
    else if (m_tableCloseCheckBoxesActive.value(TableCloseCheckBox::showCloseOutlineNormally)->isChecked()
             != m_internalSettings->showCloseOutlineNormally(true))
        modified = true;

    else if (m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->currentIndex()
             != m_internalSettings->varyColorCloseIcon(true))
        modified = true;
    else if (m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)->currentIndex()
             != m_internalSettings->varyColorCloseBackground(true))
        modified = true;
    else if (m_tableCloseComboBoxesActive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->currentIndex()
             != m_internalSettings->varyColorCloseOutline(true))
        modified = true;

    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnPress)->isChecked() != m_internalSettings->showIconOnPress(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showIconOnHover)->isChecked() != m_internalSettings->showIconOnHover(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showIconNormally)->isChecked() != m_internalSettings->showIconNormally(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnPress)->isChecked() != m_internalSettings->showBackgroundOnPress(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundOnHover)->isChecked() != m_internalSettings->showBackgroundOnHover(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showBackgroundNormally)->isChecked() != m_internalSettings->showBackgroundNormally(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnPress)->isChecked() != m_internalSettings->showOutlineOnPress(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineOnHover)->isChecked() != m_internalSettings->showOutlineOnHover(false))
        modified = true;
    else if (m_tableCheckBoxesInactive.value(TableCheckBox::showOutlineNormally)->isChecked() != m_internalSettings->showOutlineNormally(false))
        modified = true;

    else if (m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorIcon)->currentIndex() != m_internalSettings->varyColorIcon(false))
        modified = true;
    else if (m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorBackground)->currentIndex() != m_internalSettings->varyColorBackground(false))
        modified = true;
    else if (m_tableComboBoxesInactive.value(TableVaryColorComboBox::varyColorOutline)->currentIndex() != m_internalSettings->varyColorOutline(false))
        modified = true;

    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnPress)->isChecked() != m_internalSettings->showCloseIconOnPress(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconOnHover)->isChecked() != m_internalSettings->showCloseIconOnHover(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseIconNormally)->isChecked() != m_internalSettings->showCloseIconNormally(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnPress)->isChecked()
             != m_internalSettings->showCloseBackgroundOnPress(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundOnHover)->isChecked()
             != m_internalSettings->showCloseBackgroundOnHover(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseBackgroundNormally)->isChecked()
             != m_internalSettings->showCloseBackgroundNormally(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnPress)->isChecked()
             != m_internalSettings->showCloseOutlineOnPress(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineOnHover)->isChecked()
             != m_internalSettings->showCloseOutlineOnHover(false))
        modified = true;
    else if (m_tableCloseCheckBoxesInactive.value(TableCloseCheckBox::showCloseOutlineNormally)->isChecked()
             != m_internalSettings->showCloseOutlineNormally(false))
        modified = true;

    else if (m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseIcon)->currentIndex()
             != m_internalSettings->varyColorCloseIcon(false))
        modified = true;
    else if (m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseBackground)->currentIndex()
             != m_internalSettings->varyColorCloseBackground(false))
        modified = true;
    else if (m_tableCloseComboBoxesInactive.value(TableCloseVaryColorComboBox::varyColorCloseOutline)->currentIndex()
             != m_internalSettings->varyColorCloseOutline(false))
        modified = true;

    else if (m_ui->buttonStateCheckedActive->currentIndex() != m_internalSettings->buttonStateChecked(true))
        modified = true;
    else if (m_ui->buttonStateCheckedInactive->currentIndex() != m_internalSettings->buttonStateChecked(false))
        modified = true;

    else if (m_ui->lockCloseButtonBehaviourActive->isChecked() != m_internalSettings->lockCloseButtonBehaviour(true))
        modified = true;
    else if (m_ui->lockCloseButtonBehaviourInactive->isChecked() != m_internalSettings->lockCloseButtonBehaviour(false))
        modified = true;
    else if (m_ui->lockButtonBehaviourActive->isChecked() != m_internalSettings->lockButtonBehaviourActiveInactive())
        modified = true;

    setChanged(modified);
}

void ButtonBehaviour::copyCheckedStatusFromNormalToCloseActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QCheckBox *sourceCheckBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (!sourceCheckBox)
        return;

    int checkBoxIndex = static_cast<int>(m_tableCheckBoxesActive.key(sourceCheckBox));
    QCheckBox *destinationCheckBox = m_tableCloseCheckBoxesActive.value(static_cast<TableCloseCheckBox>(checkBoxIndex));

    destinationCheckBox->setChecked(sourceCheckBox->isChecked());
}

void ButtonBehaviour::copyCheckedStatusFromNormalToCloseInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourInactive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QCheckBox *sourceCheckBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (!sourceCheckBox)
        return;

    int checkBoxIndex = static_cast<int>(m_tableCheckBoxesInactive.key(sourceCheckBox));
    QCheckBox *destinationCheckBox = m_tableCloseCheckBoxesInactive.value(static_cast<TableCloseCheckBox>(checkBoxIndex));

    destinationCheckBox->setChecked(sourceCheckBox->isChecked());
}

void ButtonBehaviour::copyCheckedStatusFromCloseToNormalActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QCheckBox *sourceCheckBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (!sourceCheckBox)
        return;

    int checkBoxIndex = static_cast<int>(m_tableCloseCheckBoxesActive.key(sourceCheckBox));
    QCheckBox *destinationCheckBox = m_tableCheckBoxesActive.value(static_cast<TableCheckBox>(checkBoxIndex));

    destinationCheckBox->setChecked(sourceCheckBox->isChecked());
}

void ButtonBehaviour::copyCheckedStatusFromCloseToNormalInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourInactive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QCheckBox *sourceCheckBox = qobject_cast<QCheckBox *>(QObject::sender());
    if (!sourceCheckBox)
        return;

    int checkBoxIndex = static_cast<int>(m_tableCloseCheckBoxesInactive.key(sourceCheckBox));
    QCheckBox *destinationCheckBox = m_tableCheckBoxesInactive.value(static_cast<TableCheckBox>(checkBoxIndex));

    destinationCheckBox->setChecked(sourceCheckBox->isChecked());
}

void ButtonBehaviour::copyCheckedStatusFromActiveToInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the active/inactive lock is checked

    QAbstractButton *sourceCheckButton = qobject_cast<QAbstractButton *>(QObject::sender());
    if (!sourceCheckButton)
        return;

    int i;
    if (sourceCheckButton == m_ui->lockCloseButtonBehaviourActive) {
        m_ui->lockCloseButtonBehaviourInactive->setChecked(m_ui->lockCloseButtonBehaviourActive->isChecked());
    } else if ((i = static_cast<int>(m_tableCheckBoxesActive.key(static_cast<QCheckBox *>(sourceCheckButton), TableCheckBox::COUNT)))
               < static_cast<int>(TableCheckBox::COUNT)) {
        QCheckBox *destinationCheckBox = m_tableCheckBoxesInactive.value(static_cast<TableCheckBox>(i));
        destinationCheckBox->setChecked(sourceCheckButton->isChecked());
    } else if ((i = static_cast<int>(m_tableCloseCheckBoxesActive.key(static_cast<QCheckBox *>(sourceCheckButton), TableCloseCheckBox::COUNT)))
               < static_cast<int>(TableCloseCheckBox::COUNT)) {
        QCheckBox *destinationCheckBox = m_tableCloseCheckBoxesInactive.value(static_cast<TableCloseCheckBox>(i));
        destinationCheckBox->setChecked(sourceCheckButton->isChecked());
    }
}

void ButtonBehaviour::copyCheckedStatusFromInactiveToActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the active/inactive lock is checked

    QAbstractButton *sourceCheckButton = qobject_cast<QAbstractButton *>(QObject::sender());
    if (!sourceCheckButton)
        return;

    int i;
    if (sourceCheckButton == m_ui->lockCloseButtonBehaviourInactive) {
        m_ui->lockCloseButtonBehaviourActive->setChecked(m_ui->lockCloseButtonBehaviourInactive->isChecked());
    } else if ((i = static_cast<int>(m_tableCheckBoxesInactive.key(static_cast<QCheckBox *>(sourceCheckButton), TableCheckBox::COUNT)))
               < static_cast<int>(TableCheckBox::COUNT)) {
        QCheckBox *destinationCheckBox = m_tableCheckBoxesActive.value(static_cast<TableCheckBox>(i));
        destinationCheckBox->setChecked(sourceCheckButton->isChecked());
    } else if ((i = static_cast<int>(m_tableCloseCheckBoxesInactive.key(static_cast<QCheckBox *>(sourceCheckButton), TableCloseCheckBox::COUNT)))
               < static_cast<int>(TableCloseCheckBox::COUNT)) {
        QCheckBox *destinationCheckBox = m_tableCloseCheckBoxesActive.value(static_cast<TableCloseCheckBox>(i));
        destinationCheckBox->setChecked(sourceCheckButton->isChecked());
    }
}

void ButtonBehaviour::copyComboStatusFromNormalToCloseActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int comboBoxIndex = static_cast<int>(m_tableComboBoxesActive.key(sourceComboBox));
    QComboBox *destinationComboBox = m_tableCloseComboBoxesActive.value(static_cast<TableCloseVaryColorComboBox>(comboBoxIndex));

    destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
}

void ButtonBehaviour::copyComboStatusFromNormalToCloseInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourInactive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int comboBoxIndex = static_cast<int>(m_tableComboBoxesInactive.key(sourceComboBox));
    QComboBox *destinationComboBox = m_tableCloseComboBoxesInactive.value(static_cast<TableCloseVaryColorComboBox>(comboBoxIndex));

    destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
}

void ButtonBehaviour::copyComboStatusFromCloseToNormalActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int comboBoxIndex = static_cast<int>(m_tableCloseComboBoxesActive.key(sourceComboBox));
    QComboBox *destinationComboBox = m_tableComboBoxesActive.value(static_cast<TableVaryColorComboBox>(comboBoxIndex));

    destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
}

void ButtonBehaviour::copyComboStatusFromCloseToNormalInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockCloseButtonBehaviourInactive->isChecked())
        return; // only do this for user interactions where the normal/close lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int comboBoxIndex = static_cast<int>(m_tableCloseComboBoxesInactive.key(sourceComboBox));
    QComboBox *destinationComboBox = m_tableComboBoxesInactive.value(static_cast<TableVaryColorComboBox>(comboBoxIndex));

    destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
}

void ButtonBehaviour::copyComboStatusFromActiveToInactive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the active/inactive lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int i;
    if ((i = static_cast<int>(m_tableComboBoxesActive.key(static_cast<QComboBox *>(sourceComboBox), TableVaryColorComboBox::COUNT)))
        < static_cast<int>(TableVaryColorComboBox::COUNT)) {
        QComboBox *destinationComboBox = m_tableComboBoxesInactive.value(static_cast<TableVaryColorComboBox>(i));
        destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
    } else if ((i = static_cast<int>(m_tableCloseComboBoxesActive.key(static_cast<QComboBox *>(sourceComboBox), TableCloseVaryColorComboBox::COUNT)))
               < static_cast<int>(TableCloseVaryColorComboBox::COUNT)) {
        QComboBox *destinationComboBox = m_tableCloseComboBoxesInactive.value(static_cast<TableCloseVaryColorComboBox>(i));
        destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
    }
}

void ButtonBehaviour::copyComboStatusFromInactiveToActive()
{
    if (m_loading || m_processingDefaults || !m_ui->lockButtonBehaviourActive->isChecked())
        return; // only do this for user interactions where the active/inactive lock is checked

    QComboBox *sourceComboBox = qobject_cast<QComboBox *>(QObject::sender());
    if (!sourceComboBox)
        return;

    int i;
    if ((i = static_cast<int>(m_tableComboBoxesInactive.key(static_cast<QComboBox *>(sourceComboBox), TableVaryColorComboBox::COUNT)))
        < static_cast<int>(TableVaryColorComboBox::COUNT)) {
        QComboBox *destinationComboBox = m_tableComboBoxesActive.value(static_cast<TableVaryColorComboBox>(i));
        destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
    } else if ((i = static_cast<int>(m_tableCloseComboBoxesInactive.key(static_cast<QComboBox *>(sourceComboBox), TableCloseVaryColorComboBox::COUNT)))
               < static_cast<int>(TableCloseVaryColorComboBox::COUNT)) {
        QComboBox *destinationComboBox = m_tableCloseComboBoxesActive.value(static_cast<TableCloseVaryColorComboBox>(i));
        destinationComboBox->setCurrentIndex(sourceComboBox->currentIndex());
    }
}

void ButtonBehaviour::setApplyButtonState(const bool on)
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(on);
}

void ButtonBehaviour::updateLockIcons()
{
    m_ui->lockButtonBehaviourActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockCloseButtonBehaviourActive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockButtonBehaviourInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
    m_ui->lockCloseButtonBehaviourInactive->setIcon(static_cast<ConfigWidget *>(m_parent)->lockIcon(LockIconState::Bistate));
}
}
