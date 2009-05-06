/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Wed 6. May 14:59:15 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QToolBox *toolBox;
    QWidget *page;
    QVBoxLayout *verticalLayout;
    QListWidget *listPeers;
    QWidget *page_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QLineEdit *textDestination;
    QSpacerItem *verticalSpacer;
    QProgressBar *progressBar;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QString::fromUtf8("MainWindowClass"));
        MainWindowClass->resize(396, 357);
        MainWindowClass->setMinimumSize(QSize(289, 241));
        MainWindowClass->setAcceptDrops(true);
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setMargin(11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        toolBox = new QToolBox(centralWidget);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        page->setGeometry(QRect(0, 0, 378, 212));
        verticalLayout = new QVBoxLayout(page);
        verticalLayout->setSpacing(6);
        verticalLayout->setMargin(11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        listPeers = new QListWidget(page);
        listPeers->setObjectName(QString::fromUtf8("listPeers"));

        verticalLayout->addWidget(listPeers);

        toolBox->addItem(page, QString::fromUtf8("Available clients"));
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        page_2->setGeometry(QRect(0, 0, 378, 212));
        verticalLayout_3 = new QVBoxLayout(page_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setMargin(11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label = new QLabel(page_2);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_3->addWidget(label);

        textDestination = new QLineEdit(page_2);
        textDestination->setObjectName(QString::fromUtf8("textDestination"));

        verticalLayout_3->addWidget(textDestination);

        verticalSpacer = new QSpacerItem(20, 97, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        toolBox->addItem(page_2, QString::fromUtf8("Manual IP connection"));

        verticalLayout_2->addWidget(toolBox);

        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setTextVisible(false);

        verticalLayout_2->addWidget(progressBar);

        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 396, 22));
        MainWindowClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        retranslateUi(MainWindowClass);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "Dukto", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(page), QApplication::translate("MainWindowClass", "Available clients", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindowClass", "Destination host name or IP address:", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(page_2), QApplication::translate("MainWindowClass", "Manual IP connection", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(MainWindowClass);
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
