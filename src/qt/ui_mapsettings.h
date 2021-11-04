/********************************************************************************
** Form generated from reading UI file 'mapsettings.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAPSETTINGS_H
#define UI_MAPSETTINGS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MapSettings
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *shapefileLabel;
    QPushButton *shapefile;
    QLabel *gcpList;
    QPushButton *gcp;
    QPushButton *color;
    QLabel *colorLabel;
    QPushButton *startButton;
    QVBoxLayout *logLayout;
    QLabel *logLabel;
    QPlainTextEdit *logWindow;

    void setupUi(QDialog *MapSettings)
    {
        if (MapSettings->objectName().isEmpty())
            MapSettings->setObjectName(QString::fromUtf8("MapSettings"));
        MapSettings->resize(618, 514);
        QIcon icon;
        icon.addFile(QString::fromUtf8("../../logo128.png"), QSize(), QIcon::Normal, QIcon::Off);
        MapSettings->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(MapSettings);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        shapefileLabel = new QLabel(MapSettings);
        shapefileLabel->setObjectName(QString::fromUtf8("shapefileLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, shapefileLabel);

        shapefile = new QPushButton(MapSettings);
        shapefile->setObjectName(QString::fromUtf8("shapefile"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(shapefile->sizePolicy().hasHeightForWidth());
        shapefile->setSizePolicy(sizePolicy);

        formLayout->setWidget(1, QFormLayout::FieldRole, shapefile);

        gcpList = new QLabel(MapSettings);
        gcpList->setObjectName(QString::fromUtf8("gcpList"));

        formLayout->setWidget(0, QFormLayout::LabelRole, gcpList);

        gcp = new QPushButton(MapSettings);
        gcp->setObjectName(QString::fromUtf8("gcp"));
        sizePolicy.setHeightForWidth(gcp->sizePolicy().hasHeightForWidth());
        gcp->setSizePolicy(sizePolicy);

        formLayout->setWidget(0, QFormLayout::FieldRole, gcp);

        color = new QPushButton(MapSettings);
        color->setObjectName(QString::fromUtf8("color"));
        sizePolicy.setHeightForWidth(color->sizePolicy().hasHeightForWidth());
        color->setSizePolicy(sizePolicy);
        color->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255)"));

        formLayout->setWidget(2, QFormLayout::FieldRole, color);

        colorLabel = new QLabel(MapSettings);
        colorLabel->setObjectName(QString::fromUtf8("colorLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, colorLabel);


        verticalLayout->addLayout(formLayout);

        startButton = new QPushButton(MapSettings);
        startButton->setObjectName(QString::fromUtf8("startButton"));
        startButton->setEnabled(false);

        verticalLayout->addWidget(startButton);

        logLayout = new QVBoxLayout();
        logLayout->setObjectName(QString::fromUtf8("logLayout"));
        logLabel = new QLabel(MapSettings);
        logLabel->setObjectName(QString::fromUtf8("logLabel"));

        logLayout->addWidget(logLabel);

        logWindow = new QPlainTextEdit(MapSettings);
        logWindow->setObjectName(QString::fromUtf8("logWindow"));
        logWindow->setReadOnly(true);

        logLayout->addWidget(logWindow);


        verticalLayout->addLayout(logLayout);


        retranslateUi(MapSettings);

        QMetaObject::connectSlotsByName(MapSettings);
    } // setupUi

    void retranslateUi(QDialog *MapSettings)
    {
        MapSettings->setWindowTitle(QCoreApplication::translate("MapSettings", "Map Settings", nullptr));
        shapefileLabel->setText(QCoreApplication::translate("MapSettings", "Shapefile", nullptr));
        shapefile->setText(QCoreApplication::translate("MapSettings", "Select...", nullptr));
        gcpList->setText(QCoreApplication::translate("MapSettings", "GCP File", nullptr));
        gcp->setText(QCoreApplication::translate("MapSettings", "(From TLE)", nullptr));
        color->setText(QString());
        colorLabel->setText(QCoreApplication::translate("MapSettings", "Color", nullptr));
        startButton->setText(QCoreApplication::translate("MapSettings", "Start", nullptr));
        logLabel->setText(QCoreApplication::translate("MapSettings", "Output", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MapSettings: public Ui_MapSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAPSETTINGS_H
