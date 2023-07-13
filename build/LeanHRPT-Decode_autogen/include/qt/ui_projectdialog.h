/********************************************************************************
** Form generated from reading UI file 'projectdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROJECTDIALOG_H
#define UI_PROJECTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProjectDialog
{
public:
    QHBoxLayout *horizontalLayout;
    QGraphicsView *projectionPreview;
    QVBoxLayout *buttonColumn;
    QGroupBox *settingsBox;
    QFormLayout *formLayout;
    QLabel *boundsLabel;
    QComboBox *bounds;
    QLabel *resolutionLabel;
    QDoubleSpinBox *resolution;
    QSpacerItem *verticalSpacer;
    QLabel *details;
    QComboBox *projection;
    QLabel *projectionLabel;
    QHBoxLayout *buttonRow;
    QPushButton *preview;
    QPushButton *render;

    void setupUi(QDialog *ProjectDialog)
    {
        if (ProjectDialog->objectName().isEmpty())
            ProjectDialog->setObjectName(QString::fromUtf8("ProjectDialog"));
        ProjectDialog->resize(970, 444);
        QIcon icon;
        icon.addFile(QString::fromUtf8("logo128.ico"), QSize(), QIcon::Normal, QIcon::Off);
        ProjectDialog->setWindowIcon(icon);
        horizontalLayout = new QHBoxLayout(ProjectDialog);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        projectionPreview = new QGraphicsView(ProjectDialog);
        projectionPreview->setObjectName(QString::fromUtf8("projectionPreview"));

        horizontalLayout->addWidget(projectionPreview);

        buttonColumn = new QVBoxLayout();
        buttonColumn->setObjectName(QString::fromUtf8("buttonColumn"));
        settingsBox = new QGroupBox(ProjectDialog);
        settingsBox->setObjectName(QString::fromUtf8("settingsBox"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(settingsBox->sizePolicy().hasHeightForWidth());
        settingsBox->setSizePolicy(sizePolicy);
        formLayout = new QFormLayout(settingsBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        boundsLabel = new QLabel(settingsBox);
        boundsLabel->setObjectName(QString::fromUtf8("boundsLabel"));

        formLayout->setWidget(0, QFormLayout::LabelRole, boundsLabel);

        bounds = new QComboBox(settingsBox);
        bounds->addItem(QString());
        bounds->addItem(QString());
        bounds->setObjectName(QString::fromUtf8("bounds"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(bounds->sizePolicy().hasHeightForWidth());
        bounds->setSizePolicy(sizePolicy1);

        formLayout->setWidget(0, QFormLayout::FieldRole, bounds);

        resolutionLabel = new QLabel(settingsBox);
        resolutionLabel->setObjectName(QString::fromUtf8("resolutionLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, resolutionLabel);

        resolution = new QDoubleSpinBox(settingsBox);
        resolution->setObjectName(QString::fromUtf8("resolution"));
        sizePolicy1.setHeightForWidth(resolution->sizePolicy().hasHeightForWidth());
        resolution->setSizePolicy(sizePolicy1);
        resolution->setDecimals(1);
        resolution->setMinimum(1.000000000000000);
        resolution->setMaximum(40.000000000000000);

        formLayout->setWidget(2, QFormLayout::FieldRole, resolution);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->setItem(3, QFormLayout::LabelRole, verticalSpacer);

        details = new QLabel(settingsBox);
        details->setObjectName(QString::fromUtf8("details"));

        formLayout->setWidget(4, QFormLayout::SpanningRole, details);

        projection = new QComboBox(settingsBox);
        projection->setObjectName(QString::fromUtf8("projection"));
        sizePolicy1.setHeightForWidth(projection->sizePolicy().hasHeightForWidth());
        projection->setSizePolicy(sizePolicy1);

        formLayout->setWidget(1, QFormLayout::FieldRole, projection);

        projectionLabel = new QLabel(settingsBox);
        projectionLabel->setObjectName(QString::fromUtf8("projectionLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, projectionLabel);


        buttonColumn->addWidget(settingsBox);

        buttonRow = new QHBoxLayout();
        buttonRow->setObjectName(QString::fromUtf8("buttonRow"));
        preview = new QPushButton(ProjectDialog);
        preview->setObjectName(QString::fromUtf8("preview"));

        buttonRow->addWidget(preview);

        render = new QPushButton(ProjectDialog);
        render->setObjectName(QString::fromUtf8("render"));

        buttonRow->addWidget(render);


        buttonColumn->addLayout(buttonRow);


        horizontalLayout->addLayout(buttonColumn);


        retranslateUi(ProjectDialog);

        QMetaObject::connectSlotsByName(ProjectDialog);
    } // setupUi

    void retranslateUi(QDialog *ProjectDialog)
    {
        ProjectDialog->setWindowTitle(QCoreApplication::translate("ProjectDialog", "Projector", nullptr));
        settingsBox->setTitle(QCoreApplication::translate("ProjectDialog", "Settings", nullptr));
        boundsLabel->setText(QCoreApplication::translate("ProjectDialog", "Bounds", nullptr));
        bounds->setItemText(0, QCoreApplication::translate("ProjectDialog", "Auto", nullptr));
        bounds->setItemText(1, QCoreApplication::translate("ProjectDialog", "Whole Earth", nullptr));

        resolutionLabel->setText(QCoreApplication::translate("ProjectDialog", "Resolution", nullptr));
        resolution->setSuffix(QCoreApplication::translate("ProjectDialog", " km", nullptr));
        details->setText(QCoreApplication::translate("ProjectDialog", "Click preview to calculate", nullptr));
        projectionLabel->setText(QCoreApplication::translate("ProjectDialog", "Projection", nullptr));
        preview->setText(QCoreApplication::translate("ProjectDialog", "Preview", nullptr));
        render->setText(QCoreApplication::translate("ProjectDialog", "Render", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProjectDialog: public Ui_ProjectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROJECTDIALOG_H
