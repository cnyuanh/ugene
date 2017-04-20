/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QToolButton>
#include <QVBoxLayout>

#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GroupedComboboxDelegate.h>
#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

#include "MSAHighlightingTab.h"

namespace U2 {

static const int ITEMS_SPACING = 6;
static const int TITLE_SPACING = 1;

static inline QVBoxLayout * initVBoxLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    w->setLayout(layout);
    return layout;
}

static inline QHBoxLayout * initHBoxLayout(QWidget * w) {
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    w->setLayout(layout);
    return layout;
}

QWidget* MSAHighlightingTab::createColorGroup(){
    QWidget * group = new QWidget(this);

    QVBoxLayout * layout = initVBoxLayout(group);
    colorScheme = new QComboBox();
    colorScheme->setObjectName("colorScheme");
    colorScheme->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    layout->addSpacing(TITLE_SPACING);
    layout->addWidget(colorScheme);
    layout->addSpacing(ITEMS_SPACING);

    return group;
}

QWidget* MSAHighlightingTab::createHighlightingGroup() {
    QWidget * group = new QWidget(this);

    QVBoxLayout * layout = initVBoxLayout(group);
    highlightingScheme = new QComboBox();
    highlightingScheme->setObjectName("highlightingScheme");

    hint = new QLabel("");
    hint->setWordWrap(true);
    hint->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    useDots = new QCheckBox(tr("Use dots"));
    useDots->setObjectName("useDots");

    exportHighlightning = new QToolButton();
    exportHighlightning->setText(tr("Export"));
    exportHighlightning->setObjectName("exportHighlightning");

    QWidget *buttonAndSpacer = new QWidget(this);
    QHBoxLayout * layout2 = initHBoxLayout(buttonAndSpacer);
    layout2->addWidget(exportHighlightning);
    layout2->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    lessMoreLabel = new QLabel(tr("Highlight characters with conservation level:"));
    lessMoreLabel->setWordWrap(true);

    thresholdMoreRb = new QRadioButton(QString::fromWCharArray(L"\x2265") + tr(" threshold"));
    thresholdLessRb = new QRadioButton(QString::fromWCharArray(L"\x2264") + tr(" threshold"));
    thresholdMoreRb->setObjectName("thresholdMoreRb");
    thresholdLessRb->setObjectName("thresholdLessRb");

    thresholdSlider = new QSlider(Qt::Horizontal, this);
    thresholdSlider->setMinimum(0);
    thresholdSlider->setMaximum(100);
    thresholdSlider->setValue(50);
    thresholdSlider->setTickPosition(QSlider::TicksRight);
    thresholdSlider->setObjectName("thresholdSlider");

    thresholdLabel = new QLabel(tr("Threshold: %1%").arg(thresholdSlider->value()), this);

    layout->setSpacing(ITEMS_SPACING);
    layout->addSpacing(TITLE_SPACING);
    layout->addWidget(highlightingScheme);
    layout->addWidget(thresholdLabel);
    layout->addWidget(thresholdSlider);
    layout->addWidget(lessMoreLabel);
    layout->addWidget(thresholdLessRb);
    layout->addWidget(thresholdMoreRb);
    layout->addWidget(useDots);

#ifdef Q_OS_MAC
    layout->addSpacerItem(new QSpacerItem(40, 8, QSizePolicy::Expanding, QSizePolicy::Minimum));
#endif
    layout->addWidget(buttonAndSpacer);
    layout->addWidget(hint);

    return group;
}

MSAHighlightingTab::MSAHighlightingTab(MSAEditor* m)
    : msa(m), savableTab(this, GObjectViewUtils::findViewByName(m->getName()))
{
    setObjectName("HighlightingOptionsPanelWidget");
    QVBoxLayout* mainLayout = initVBoxLayout(this);
    mainLayout->setSpacing(0);

    QWidget * colorGroup = new ShowHideSubgroupWidget("COLOR", tr("Color"), createColorGroup(), true);
    mainLayout->addWidget(colorGroup);

    QWidget * highlightingGroup = new ShowHideSubgroupWidget("HIGHLIGHTING", tr("Highlighting"), createHighlightingGroup(), true);
    mainLayout->addWidget(highlightingGroup);

    seqArea = msa->getUI()->getSequenceArea();

    savableTab.disableSavingForWidgets(QStringList() << thresholdSlider->objectName() << highlightingScheme->objectName()
        << colorScheme->objectName() << highlightingScheme->objectName());
    U2WidgetStateStorage::restoreWidgetState(savableTab);

    colorScheme->setItemDelegate(new GroupedComboBoxDelegate(colorScheme));
    highlightingScheme->setItemDelegate(new GroupedComboBoxDelegate(highlightingScheme));

    initColorCB();
    sl_sync();

    connect(colorScheme, SIGNAL(currentIndexChanged(int)), SLOT(sl_colorSchemeIndexChanged(int)));
    connect(highlightingScheme, SIGNAL(currentIndexChanged(int)), SLOT(sl_highlightingSchemeIndexChanged(int)));
    connect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_triggerUseDots()));

    connect(seqArea, SIGNAL(si_highlightingChanged()), SLOT(sl_sync()));

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    connect(msaColorSchemeRegistry, SIGNAL(si_customSettingsChanged()), SLOT(sl_customSchemesListChanged()));

    connect(m, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_updateHint()));
    connect(m->getMaObject(), SIGNAL(si_alphabetChanged(MaModificationInfo, const DNAAlphabet *)), SLOT(sl_customSchemesListChanged()));

    connect(highlightingScheme, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_updateHint()));
    connect(exportHighlightning, SIGNAL(clicked()), SLOT(sl_exportHighlightningClicked()));
    connect(thresholdSlider, SIGNAL(valueChanged(int)), SLOT(sl_highlightingParametersChanged()));
    connect(thresholdMoreRb, SIGNAL(toggled(bool)), SLOT(sl_highlightingParametersChanged()));
    connect(thresholdLessRb, SIGNAL(toggled(bool)), SLOT(sl_highlightingParametersChanged()));

    sl_updateHint();
    sl_highlightingParametersChanged();
}

void MSAHighlightingTab::initColorCB() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;

    colorScheme->blockSignals(true);
    highlightingScheme->blockSignals(true);

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory *> colorSchemesFactories = msaColorSchemeRegistry->getAllMsaColorSchemes(msa->getMaObject()->getAlphabet()->getType());

    colorScheme->clear();
    if (isAlphabetRaw) {
        fillColorCbWithGrouping(colorSchemesFactories);
    } else {
        foreach(MsaColorSchemeFactory *factory, colorSchemesFactories) {
            colorScheme->addItem(factory->getName(), factory->getId());
        }
    }

    MsaHighlightingSchemeRegistry *msaHighlightingSchemeRegistry = AppContext::getMsaHighlightingSchemeRegistry();
    QList<MsaHighlightingSchemeFactory *> highlightingSchemesFactories = msaHighlightingSchemeRegistry->getMsaHighlightingSchemes(msa->getMaObject()->getAlphabet()->getType());

    highlightingScheme->clear();
    if (isAlphabetRaw) {
        fillHighlightingCbWithGrouping(highlightingSchemesFactories);
    } else {
        foreach(MsaHighlightingSchemeFactory *factory, highlightingSchemesFactories) {
            highlightingScheme->addItem(factory->getName(), factory->getId());
        }
    }

    colorScheme->blockSignals(false);
    highlightingScheme->blockSignals(false);
}

void MSAHighlightingTab::fillColorCbWithGrouping(QList<MsaColorSchemeFactory *> colorSchemesFactories) {
    QList<MsaColorSchemeFactory *> rawColorSchemesFactories;
    QList<MsaColorSchemeFactory *> aminoColorSchemesFactories;
    QList<MsaColorSchemeFactory *> nucleotideColorSchemesFactories;
    foreach(MsaColorSchemeFactory *factory, colorSchemesFactories) {
        if (factory->isAlphabetFit(DNAAlphabet_RAW)) {
            rawColorSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_AMINO)) {
            aminoColorSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_NUCL)) {
            nucleotideColorSchemesFactories.append(factory);
        }
    }
    GroupedComboBoxDelegate *colorSchemeDelegate = qobject_cast<GroupedComboBoxDelegate*>(colorScheme->itemDelegate());
    QStandardItemModel *colorSchemeModel = qobject_cast<QStandardItemModel*>(colorScheme->model());
    CHECK(colorSchemeDelegate != NULL, );
    CHECK(colorSchemeModel != NULL, );
    colorSchemeDelegate->addParentItem(colorSchemeModel, tr("RAW alphabet"));
    foreach(MsaColorSchemeFactory *factory, rawColorSchemesFactories) {
        colorSchemeDelegate->addChildItem(colorSchemeModel, factory->getName(), factory->getId());
    }
    colorSchemeDelegate->addParentItem(colorSchemeModel, tr("Amino acid alphabet"));
    foreach(MsaColorSchemeFactory *factory, aminoColorSchemesFactories) {
        colorSchemeDelegate->addChildItem(colorSchemeModel, factory->getName(), factory->getId());
    }
    colorSchemeDelegate->addParentItem(colorSchemeModel, tr("Nucleotide alphabet"));
    foreach(MsaColorSchemeFactory *factory, nucleotideColorSchemesFactories) {
        colorSchemeDelegate->addChildItem(colorSchemeModel, factory->getName(), factory->getId());
    }
}

void MSAHighlightingTab::fillHighlightingCbWithGrouping(QList<MsaHighlightingSchemeFactory *> highlightingSchemesFactories) {
    QList<MsaHighlightingSchemeFactory *> commonHighlightSchemesFactories;
    QList<MsaHighlightingSchemeFactory *> rawHighlightSchemesFactories;
    QList<MsaHighlightingSchemeFactory *> aminoHighlightSchemesFactories;
    QList<MsaHighlightingSchemeFactory *> nucleotideHighlightSchemesFactories;
    foreach(MsaHighlightingSchemeFactory *factory, highlightingSchemesFactories) {
        if (factory->isAlphabetFit(DNAAlphabet_AMINO) &&
            factory->isAlphabetFit(DNAAlphabet_NUCL) &&
            factory->isAlphabetFit(DNAAlphabet_RAW)) {
            commonHighlightSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_RAW)) {
            rawHighlightSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_AMINO)) {
            aminoHighlightSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_NUCL)) {
            nucleotideHighlightSchemesFactories.append(factory);
        }
    }
    GroupedComboBoxDelegate *highlightingSchemeDelegate = qobject_cast<GroupedComboBoxDelegate*>(highlightingScheme->itemDelegate());
    QStandardItemModel *highlightingSchemeModel = qobject_cast<QStandardItemModel*>(highlightingScheme->model());
    CHECK(highlightingSchemeDelegate != NULL, );
    CHECK(highlightingSchemeModel != NULL, );

    foreach(MsaHighlightingSchemeFactory *factory, commonHighlightSchemesFactories) {
        highlightingSchemeDelegate->addChildItem(highlightingSchemeModel, factory->getName(), factory->getId());
    }
    
    if (!rawHighlightSchemesFactories.isEmpty()) {
        highlightingSchemeDelegate->addParentItem(highlightingSchemeModel, tr("RAW alphabet"));
        foreach(MsaHighlightingSchemeFactory *factory, rawHighlightSchemesFactories) {
            highlightingSchemeDelegate->addChildItem(highlightingSchemeModel, factory->getName(), factory->getId());
        }
    }
    
    if (!aminoHighlightSchemesFactories.isEmpty()) {
        highlightingSchemeDelegate->addParentItem(highlightingSchemeModel, tr("Amino acid alphabet"));
        foreach(MsaHighlightingSchemeFactory *factory, aminoHighlightSchemesFactories) {
            highlightingSchemeDelegate->addChildItem(highlightingSchemeModel, factory->getName(), factory->getId());
        }
    }
    
    if (!nucleotideHighlightSchemesFactories.isEmpty()) {
        highlightingSchemeDelegate->addParentItem(highlightingSchemeModel, tr("Nucleotide alphabet"));
        foreach(MsaHighlightingSchemeFactory *factory, nucleotideHighlightSchemesFactories) {
            highlightingSchemeDelegate->addChildItem(highlightingSchemeModel, factory->getName(), factory->getId());
        }
    }
}

void MSAHighlightingTab::sl_sync() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;
    MsaColorScheme *s = seqArea->getCurrentColorScheme();
    SAFE_POINT(s != NULL, "Current scheme is NULL", );
    SAFE_POINT(s->getFactory() != NULL, "Current scheme color factory is NULL", );

    colorScheme->blockSignals(true);
    colorScheme->setCurrentIndex(colorScheme->findData(s->getFactory()->getId()));
    colorScheme->blockSignals(false);

    MsaHighlightingScheme *sh = seqArea->getCurrentHighlightingScheme();
    SAFE_POINT(sh != NULL, "Current highlighting scheme is NULL!", );
    SAFE_POINT(sh->getFactory() != NULL, "Current highlighting scheme factory is NULL!", );

    highlightingScheme->blockSignals(true);
    highlightingScheme->setCurrentIndex(highlightingScheme->findData(sh->getFactory()->getId()));
    highlightingScheme->blockSignals(false);

    useDots->blockSignals(true);
    useDots->setChecked(seqArea->getUseDotsCheckedState());
    useDots->blockSignals(false);

    sl_updateHint();
}

void MSAHighlightingTab::sl_updateHint() {
    MsaHighlightingScheme *s = seqArea->getCurrentHighlightingScheme();
    SAFE_POINT(s->getFactory() != NULL, "Highlighting factory is NULL!", );

    QVariantMap highlightingSettings;
    if(s->getFactory()->isNeedThreshold()){
        thresholdLabel->show();
        thresholdSlider->show();
        thresholdLessRb->show();
        thresholdMoreRb->show();
        lessMoreLabel->show();
        bool ok = false;
        int thresholdValue = s->getSettings().value(MsaHighlightingScheme::THRESHOLD_PARAMETER_NAME).toInt(&ok);
        assert(ok);
        thresholdSlider->setValue(thresholdValue);
        bool lessThenThreshold = s->getSettings().value(MsaHighlightingScheme::LESS_THAN_THRESHOLD_PARAMETER_NAME, thresholdLessRb->isChecked()).toBool();
        thresholdLessRb->setChecked(lessThenThreshold);
        thresholdMoreRb->setChecked(!lessThenThreshold);
        highlightingSettings.insert(MsaHighlightingScheme::THRESHOLD_PARAMETER_NAME, thresholdValue);
        highlightingSettings.insert(MsaHighlightingScheme::LESS_THAN_THRESHOLD_PARAMETER_NAME, lessThenThreshold);
    }else{
        thresholdLabel->hide();
        thresholdSlider->hide();
        thresholdLessRb->hide();
        thresholdMoreRb->hide();
        lessMoreLabel->hide();
    }
    if (U2MsaRow::INVALID_ROW_ID == msa->getReferenceRowId()
        && !seqArea->getCurrentHighlightingScheme()->getFactory()->isRefFree())
    {
        hint->setText(tr("Hint: select a reference above"));
        hint->setStyleSheet(
            "color: green;"
            "font: bold;");
        exportHighlightning->setDisabled(true);
        return;
    }
    hint->setText("");
    if(s->getFactory()->isRefFree()){
        exportHighlightning->setDisabled(true);
    }else{
        exportHighlightning->setEnabled(true);
    }
    s->applySettings(highlightingSettings);
}

void MSAHighlightingTab::sl_exportHighlightningClicked(){
    msa->exportHighlighted();
}

void MSAHighlightingTab::sl_highlightingParametersChanged() {
    QVariantMap highlightingSettings;
    thresholdLabel->setText(tr("Threshold: %1%").arg(thresholdSlider->value()));
    MsaHighlightingScheme *s = seqArea->getCurrentHighlightingScheme();
    highlightingSettings.insert(MsaHighlightingScheme::THRESHOLD_PARAMETER_NAME, thresholdSlider->value());
    highlightingSettings.insert(MsaHighlightingScheme::LESS_THAN_THRESHOLD_PARAMETER_NAME, thresholdLessRb->isChecked());
    s->applySettings(highlightingSettings);
    seqArea->sl_changeColorSchemeOutside(colorScheme->currentData().toString());
}

void MSAHighlightingTab::sl_customSchemesListChanged() {
    initColorCB();
    sl_sync();
}

void MSAHighlightingTab::sl_colorSchemeIndexChanged(int index) {
    QString id = colorScheme->itemData(index).toString();
    seqArea->sl_changeColorSchemeOutside(colorScheme->itemData(index).toString());
}

void MSAHighlightingTab::sl_highlightingSchemeIndexChanged(int index) {
    seqArea->sl_changeColorSchemeOutside(highlightingScheme->itemData(index).toString());
}

}//ns
