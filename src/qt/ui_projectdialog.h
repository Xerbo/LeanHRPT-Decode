/********************************************************************************
** Form generated from reading UI file 'projectdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProjectDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *settings;
    QLabel *sourceLabel;
    QComboBox *source;
    QLabel *projectionLabel;
    QComboBox *projection;
    QLabel *interpolationLabel;
    QComboBox *interpolation;
    QLabel *gcpLabel;
    QPushButton *gcp;
    QLabel *outputLabel;
    QPushButton *output;
    QPushButton *startButton;
    QVBoxLayout *logLayout;
    QLabel *logLabel;
    QPlainTextEdit *logWindow;

    void setupUi(QDialog *ProjectDialog)
    {
        if (ProjectDialog->objectName().isEmpty())
            ProjectDialog->setObjectName(QString::fromUtf8("ProjectDialog"));
        ProjectDialog->resize(618, 514);
        QIcon icon;
        icon.addFile(QString::fromUtf8("logo128.png"), QSize(), QIcon::Normal, QIcon::Off);
        ProjectDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(ProjectDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        settings = new QFormLayout();
        settings->setObjectName(QString::fromUtf8("settings"));
        sourceLabel = new QLabel(ProjectDialog);
        sourceLabel->setObjectName(QString::fromUtf8("sourceLabel"));

        settings->setWidget(0, QFormLayout::LabelRole, sourceLabel);

        source = new QComboBox(ProjectDialog);
        source->addItem(QString());
        source->addItem(QString());
        source->setObjectName(QString::fromUtf8("source"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(source->sizePolicy().hasHeightForWidth());
        source->setSizePolicy(sizePolicy);

        settings->setWidget(0, QFormLayout::FieldRole, source);

        projectionLabel = new QLabel(ProjectDialog);
        projectionLabel->setObjectName(QString::fromUtf8("projectionLabel"));

        settings->setWidget(1, QFormLayout::LabelRole, projectionLabel);

        projection = new QComboBox(ProjectDialog);
        projection->addItem(QString());
        projection->addItem(QString());
        projection->addItem(QString());
        projection->setObjectName(QString::fromUtf8("projection"));
        sizePolicy.setHeightForWidth(projection->sizePolicy().hasHeightForWidth());
        projection->setSizePolicy(sizePolicy);

        settings->setWidget(1, QFormLayout::FieldRole, projection);

        interpolationLabel = new QLabel(ProjectDialog);
        interpolationLabel->setObjectName(QString::fromUtf8("interpolationLabel"));

        settings->setWidget(2, QFormLayout::LabelRole, interpolationLabel);

        interpolation = new QComboBox(ProjectDialog);
        interpolation->addItem(QString());
        interpolation->addItem(QString());
        interpolation->addItem(QString());
        interpolation->addItem(QString());
        interpolation->setObjectName(QString::fromUtf8("interpolation"));
        sizePolicy.setHeightForWidth(interpolation->sizePolicy().hasHeightForWidth());
        interpolation->setSizePolicy(sizePolicy);

        settings->setWidget(2, QFormLayout::FieldRole, interpolation);

        gcpLabel = new QLabel(ProjectDialog);
        gcpLabel->setObjectName(QString::fromUtf8("gcpLabel"));

        settings->setWidget(3, QFormLayout::LabelRole, gcpLabel);

        gcp = new QPushButton(ProjectDialog);
        gcp->setObjectName(QString::fromUtf8("gcp"));
        sizePolicy.setHeightForWidth(gcp->sizePolicy().hasHeightForWidth());
        gcp->setSizePolicy(sizePolicy);

        settings->setWidget(3, QFormLayout::FieldRole, gcp);

        outputLabel = new QLabel(ProjectDialog);
        outputLabel->setObjectName(QString::fromUtf8("outputLabel"));

        settings->setWidget(4, QFormLayout::LabelRole, outputLabel);

        output = new QPushButton(ProjectDialog);
        output->setObjectName(QString::fromUtf8("output"));
        sizePolicy.setHeightForWidth(output->sizePolicy().hasHeightForWidth());
        output->setSizePolicy(sizePolicy);

        settings->setWidget(4, QFormLayout::FieldRole, output);


        verticalLayout->addLayout(settings);

        startButton = new QPushButton(ProjectDialog);
        startButton->setObjectName(QString::fromUtf8("startButton"));
        startButton->setEnabled(false);

        verticalLayout->addWidget(startButton);

        logLayout = new QVBoxLayout();
        logLayout->setObjectName(QString::fromUtf8("logLayout"));
        logLabel = new QLabel(ProjectDialog);
        logLabel->setObjectName(QString::fromUtf8("logLabel"));

        logLayout->addWidget(logLabel);

        logWindow = new QPlainTextEdit(ProjectDialog);
        logWindow->setObjectName(QString::fromUtf8("logWindow"));
        logWindow->setReadOnly(true);

        logLayout->addWidget(logWindow);


        verticalLayout->addLayout(logLayout);


        retranslateUi(ProjectDialog);

        interpolation->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(ProjectDialog);
    } // setupUi

    void retranslateUi(QDialog *ProjectDialog)
    {
        ProjectDialog->setWindowTitle(QCoreApplication::translate("ProjectDialog", "Projector", nullptr));
        sourceLabel->setText(QCoreApplication::translate("ProjectDialog", "Source", nullptr));
        source->setItemText(0, QCoreApplication::translate("ProjectDialog", "Viewport", nullptr));
        source->setItemText(1, QCoreApplication::translate("ProjectDialog", "Raw Channels", nullptr));

#if QT_CONFIG(tooltip)
        source->setToolTip(QCoreApplication::translate("ProjectDialog", "The source of the data, \"Raw Channels\" creates a file containing all channels with units", nullptr));
#endif // QT_CONFIG(tooltip)
        projectionLabel->setText(QCoreApplication::translate("ProjectDialog", "Target Projection", nullptr));
        projection->setItemText(0, QCoreApplication::translate("ProjectDialog", "EPSG:4326 (WGS 84)", nullptr));
        projection->setItemText(1, QCoreApplication::translate("ProjectDialog", "EPSG:3995 (Artic Polar)", nullptr));
        projection->setItemText(2, QCoreApplication::translate("ProjectDialog", "EPSG:3857 (Web Mercator)", nullptr));

        interpolationLabel->setText(QCoreApplication::translate("ProjectDialog", "Interpolation", nullptr));
        interpolation->setItemText(0, QCoreApplication::translate("ProjectDialog", "Nearest Neighbor", nullptr));
        interpolation->setItemText(1, QCoreApplication::translate("ProjectDialog", "Bilinear", nullptr));
        interpolation->setItemText(2, QCoreApplication::translate("ProjectDialog", "Cubic", nullptr));
        interpolation->setItemText(3, QCoreApplication::translate("ProjectDialog", "Lanczos", nullptr));

#if QT_CONFIG(tooltip)
        interpolation->setToolTip(QCoreApplication::translate("ProjectDialog", "In order of speed, fastest -> slowest (where slowest is the highest quality)", nullptr));
#endif // QT_CONFIG(tooltip)
        gcpLabel->setText(QCoreApplication::translate("ProjectDialog", "GCP File", nullptr));
#if QT_CONFIG(tooltip)
        gcp->setToolTip(QCoreApplication::translate("ProjectDialog", "Use to project an old pass where TLEs have drifted too much", nullptr));
#endif // QT_CONFIG(tooltip)
        gcp->setText(QCoreApplication::translate("ProjectDialog", "(From TLE)", nullptr));
        outputLabel->setText(QCoreApplication::translate("ProjectDialog", "Output File", nullptr));
        output->setText(QCoreApplication::translate("ProjectDialog", "Select...", nullptr));
        startButton->setText(QCoreApplication::translate("ProjectDialog", "Start", nullptr));
        logLabel->setText(QCoreApplication::translate("ProjectDialog", "Output", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProjectDialog: public Ui_ProjectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROJECTDIALOG_H
