/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSave_Current_Image;
    QAction *actionSave_All_Channels;
    QAction *actionFlip;
    QAction *actionDocumentation;
    QAction *actionIssue_Tracker;
    QAction *actionAbout_LeanHRPT;
    QAction *actionAbout_Qt;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QWidget *optionsSidebar;
    QVBoxLayout *options;
    QStackedWidget *stackedOptions;
    QWidget *channelPage;
    QVBoxLayout *verticalLayout_7;
    QGroupBox *channelSelectorBox;
    QFormLayout *formLayout_2;
    QComboBox *channelSelector;
    QWidget *compositePage;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *compoisteSelectorBox;
    QFormLayout *formLayout;
    QComboBox *redSelector;
    QLabel *redLabel;
    QLabel *greenLabel;
    QComboBox *greenSelector;
    QLabel *blueLabel;
    QComboBox *blueSelector;
    QWidget *ndviPage;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QRadioButton *equalisationNone;
    QRadioButton *equalisationStretch;
    QRadioButton *equalisationHistogram;
    QGroupBox *zoomSelectorBox;
    QVBoxLayout *verticalLayout_2;
    QComboBox *zoomSelector;
    QTabWidget *imageTabs;
    QWidget *channelTab;
    QVBoxLayout *verticalLayout_5;
    QGraphicsView *channelView;
    QWidget *compositeTab;
    QVBoxLayout *verticalLayout_6;
    QGraphicsView *compositeView;
    QWidget *ndviTab;
    QVBoxLayout *verticalLayout_4;
    QGraphicsView *ndviView;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuOptions;
    QMenu *menuHelp;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1200, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8("../logo64.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionSave_Current_Image = new QAction(MainWindow);
        actionSave_Current_Image->setObjectName(QString::fromUtf8("actionSave_Current_Image"));
        actionSave_All_Channels = new QAction(MainWindow);
        actionSave_All_Channels->setObjectName(QString::fromUtf8("actionSave_All_Channels"));
        actionFlip = new QAction(MainWindow);
        actionFlip->setObjectName(QString::fromUtf8("actionFlip"));
        actionFlip->setCheckable(true);
        actionDocumentation = new QAction(MainWindow);
        actionDocumentation->setObjectName(QString::fromUtf8("actionDocumentation"));
        actionIssue_Tracker = new QAction(MainWindow);
        actionIssue_Tracker->setObjectName(QString::fromUtf8("actionIssue_Tracker"));
        actionAbout_LeanHRPT = new QAction(MainWindow);
        actionAbout_LeanHRPT->setObjectName(QString::fromUtf8("actionAbout_LeanHRPT"));
        actionAbout_Qt = new QAction(MainWindow);
        actionAbout_Qt->setObjectName(QString::fromUtf8("actionAbout_Qt"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        optionsSidebar = new QWidget(centralwidget);
        optionsSidebar->setObjectName(QString::fromUtf8("optionsSidebar"));
        optionsSidebar->setMaximumSize(QSize(250, 16777215));
        options = new QVBoxLayout(optionsSidebar);
        options->setObjectName(QString::fromUtf8("options"));
        options->setContentsMargins(-1, 0, -1, 0);
        stackedOptions = new QStackedWidget(optionsSidebar);
        stackedOptions->setObjectName(QString::fromUtf8("stackedOptions"));
        channelPage = new QWidget();
        channelPage->setObjectName(QString::fromUtf8("channelPage"));
        verticalLayout_7 = new QVBoxLayout(channelPage);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        channelSelectorBox = new QGroupBox(channelPage);
        channelSelectorBox->setObjectName(QString::fromUtf8("channelSelectorBox"));
        formLayout_2 = new QFormLayout(channelSelectorBox);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        channelSelector = new QComboBox(channelSelectorBox);
        channelSelector->setObjectName(QString::fromUtf8("channelSelector"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(channelSelector->sizePolicy().hasHeightForWidth());
        channelSelector->setSizePolicy(sizePolicy);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, channelSelector);


        verticalLayout_7->addWidget(channelSelectorBox);

        stackedOptions->addWidget(channelPage);
        compositePage = new QWidget();
        compositePage->setObjectName(QString::fromUtf8("compositePage"));
        verticalLayout_3 = new QVBoxLayout(compositePage);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        compoisteSelectorBox = new QGroupBox(compositePage);
        compoisteSelectorBox->setObjectName(QString::fromUtf8("compoisteSelectorBox"));
        formLayout = new QFormLayout(compoisteSelectorBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        redSelector = new QComboBox(compoisteSelectorBox);
        redSelector->setObjectName(QString::fromUtf8("redSelector"));
        sizePolicy.setHeightForWidth(redSelector->sizePolicy().hasHeightForWidth());
        redSelector->setSizePolicy(sizePolicy);

        formLayout->setWidget(0, QFormLayout::FieldRole, redSelector);

        redLabel = new QLabel(compoisteSelectorBox);
        redLabel->setObjectName(QString::fromUtf8("redLabel"));

        formLayout->setWidget(0, QFormLayout::LabelRole, redLabel);

        greenLabel = new QLabel(compoisteSelectorBox);
        greenLabel->setObjectName(QString::fromUtf8("greenLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, greenLabel);

        greenSelector = new QComboBox(compoisteSelectorBox);
        greenSelector->setObjectName(QString::fromUtf8("greenSelector"));
        sizePolicy.setHeightForWidth(greenSelector->sizePolicy().hasHeightForWidth());
        greenSelector->setSizePolicy(sizePolicy);

        formLayout->setWidget(1, QFormLayout::FieldRole, greenSelector);

        blueLabel = new QLabel(compoisteSelectorBox);
        blueLabel->setObjectName(QString::fromUtf8("blueLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, blueLabel);

        blueSelector = new QComboBox(compoisteSelectorBox);
        blueSelector->setObjectName(QString::fromUtf8("blueSelector"));
        sizePolicy.setHeightForWidth(blueSelector->sizePolicy().hasHeightForWidth());
        blueSelector->setSizePolicy(sizePolicy);

        formLayout->setWidget(2, QFormLayout::FieldRole, blueSelector);


        verticalLayout_3->addWidget(compoisteSelectorBox);

        stackedOptions->addWidget(compositePage);
        ndviPage = new QWidget();
        ndviPage->setObjectName(QString::fromUtf8("ndviPage"));
        stackedOptions->addWidget(ndviPage);

        options->addWidget(stackedOptions);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        options->addItem(verticalSpacer);

        groupBox = new QGroupBox(optionsSidebar);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        equalisationNone = new QRadioButton(groupBox);
        equalisationNone->setObjectName(QString::fromUtf8("equalisationNone"));
        equalisationNone->setChecked(true);

        verticalLayout->addWidget(equalisationNone);

        equalisationStretch = new QRadioButton(groupBox);
        equalisationStretch->setObjectName(QString::fromUtf8("equalisationStretch"));

        verticalLayout->addWidget(equalisationStretch);

        equalisationHistogram = new QRadioButton(groupBox);
        equalisationHistogram->setObjectName(QString::fromUtf8("equalisationHistogram"));

        verticalLayout->addWidget(equalisationHistogram);


        options->addWidget(groupBox);

        zoomSelectorBox = new QGroupBox(optionsSidebar);
        zoomSelectorBox->setObjectName(QString::fromUtf8("zoomSelectorBox"));
        verticalLayout_2 = new QVBoxLayout(zoomSelectorBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        zoomSelector = new QComboBox(zoomSelectorBox);
        zoomSelector->addItem(QString());
        zoomSelector->addItem(QString());
        zoomSelector->addItem(QString());
        zoomSelector->addItem(QString());
        zoomSelector->setObjectName(QString::fromUtf8("zoomSelector"));

        verticalLayout_2->addWidget(zoomSelector);


        options->addWidget(zoomSelectorBox);


        horizontalLayout->addWidget(optionsSidebar);

        imageTabs = new QTabWidget(centralwidget);
        imageTabs->setObjectName(QString::fromUtf8("imageTabs"));
        imageTabs->setTabPosition(QTabWidget::West);
        channelTab = new QWidget();
        channelTab->setObjectName(QString::fromUtf8("channelTab"));
        verticalLayout_5 = new QVBoxLayout(channelTab);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        channelView = new QGraphicsView(channelTab);
        channelView->setObjectName(QString::fromUtf8("channelView"));
        channelView->setFrameShape(QFrame::NoFrame);
        channelView->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        channelView->setDragMode(QGraphicsView::ScrollHandDrag);

        verticalLayout_5->addWidget(channelView);

        imageTabs->addTab(channelTab, QString());
        compositeTab = new QWidget();
        compositeTab->setObjectName(QString::fromUtf8("compositeTab"));
        verticalLayout_6 = new QVBoxLayout(compositeTab);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        compositeView = new QGraphicsView(compositeTab);
        compositeView->setObjectName(QString::fromUtf8("compositeView"));
        compositeView->setFrameShape(QFrame::NoFrame);
        compositeView->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        compositeView->setDragMode(QGraphicsView::ScrollHandDrag);

        verticalLayout_6->addWidget(compositeView);

        imageTabs->addTab(compositeTab, QString());
        ndviTab = new QWidget();
        ndviTab->setObjectName(QString::fromUtf8("ndviTab"));
        verticalLayout_4 = new QVBoxLayout(ndviTab);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        ndviView = new QGraphicsView(ndviTab);
        ndviView->setObjectName(QString::fromUtf8("ndviView"));
        ndviView->setFrameShape(QFrame::NoFrame);
        ndviView->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        ndviView->setDragMode(QGraphicsView::ScrollHandDrag);

        verticalLayout_4->addWidget(ndviView);

        imageTabs->addTab(ndviTab, QString());

        horizontalLayout->addWidget(imageTabs);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1200, 29));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuOptions = new QMenu(menubar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuOptions->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addSeparator();
        menuFile->addAction(actionSave_Current_Image);
        menuFile->addAction(actionSave_All_Channels);
        menuOptions->addAction(actionFlip);
        menuHelp->addAction(actionDocumentation);
        menuHelp->addAction(actionIssue_Tracker);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout_LeanHRPT);
        menuHelp->addAction(actionAbout_Qt);

        retranslateUi(MainWindow);
        QObject::connect(imageTabs, SIGNAL(currentChanged(int)), stackedOptions, SLOT(setCurrentIndex(int)));

        stackedOptions->setCurrentIndex(0);
        zoomSelector->setCurrentIndex(2);
        imageTabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LeanHRPT Decode", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        actionSave_Current_Image->setText(QCoreApplication::translate("MainWindow", "Save Current Image", nullptr));
        actionSave_All_Channels->setText(QCoreApplication::translate("MainWindow", "Save All Channels", nullptr));
        actionFlip->setText(QCoreApplication::translate("MainWindow", "Flip", nullptr));
        actionDocumentation->setText(QCoreApplication::translate("MainWindow", "Documentation", nullptr));
        actionIssue_Tracker->setText(QCoreApplication::translate("MainWindow", "Issue Tracker", nullptr));
        actionAbout_LeanHRPT->setText(QCoreApplication::translate("MainWindow", "About LeanHRPT", nullptr));
        actionAbout_Qt->setText(QCoreApplication::translate("MainWindow", "About Qt", nullptr));
        channelSelectorBox->setTitle(QCoreApplication::translate("MainWindow", "Channel Selector", nullptr));
        compoisteSelectorBox->setTitle(QCoreApplication::translate("MainWindow", "Composite Channel Selector", nullptr));
        redLabel->setText(QCoreApplication::translate("MainWindow", "Red", nullptr));
        greenLabel->setText(QCoreApplication::translate("MainWindow", "Green", nullptr));
        blueLabel->setText(QCoreApplication::translate("MainWindow", "Blue", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Equalisation", nullptr));
        equalisationNone->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        equalisationStretch->setText(QCoreApplication::translate("MainWindow", "Stretch", nullptr));
        equalisationHistogram->setText(QCoreApplication::translate("MainWindow", "Histogram", nullptr));
        zoomSelectorBox->setTitle(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
        zoomSelector->setItemText(0, QCoreApplication::translate("MainWindow", "25%", nullptr));
        zoomSelector->setItemText(1, QCoreApplication::translate("MainWindow", "50%", nullptr));
        zoomSelector->setItemText(2, QCoreApplication::translate("MainWindow", "100%", nullptr));
        zoomSelector->setItemText(3, QCoreApplication::translate("MainWindow", "200%", nullptr));

        imageTabs->setTabText(imageTabs->indexOf(channelTab), QCoreApplication::translate("MainWindow", "Single Channel", nullptr));
        imageTabs->setTabText(imageTabs->indexOf(compositeTab), QCoreApplication::translate("MainWindow", "Composite", nullptr));
        imageTabs->setTabText(imageTabs->indexOf(ndviTab), QCoreApplication::translate("MainWindow", "NDVI", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuOptions->setTitle(QCoreApplication::translate("MainWindow", "Options", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
