/**********************************************************************************

Copyright (c) 2012, Tan Yu Sheng
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/

#include <QFile>
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <QDateTime>
#include "mainwindow.h"
#include "configwindow.h"
#include <QtDebug>
#include <QShortcut>
#include <QMessageBox>
#include <QComboBox>
#include <QAction>
#include <QLabel>
#include <QInputDialog>
#include <QToolButton>
#include <QMenu>
#include <QSpinBox>
#include <QLineEdit>
#include <QSignalMapper>
#include <QPainter>
#include <QToolbar>
#include <QPushButton>
#include "csv.h"

MainWindow::MainWindow()
{

    if (objectName().isEmpty())
                setObjectName(QStringLiteral("MainWindow"));

    //Set up signalmappers
    resetMapper = new QSignalMapper (this) ;
    swapMapper = new QSignalMapper (this) ;

    //some defaults
    resize(50, 50);
    setFixedSize(50, 50);

    QIcon icon;
    icon.addFile(QStringLiteral(":/StreamControl/icons/r1logo2trans-crop.png"), QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);
    setAnimated(true);
    setWindowFlags(windowFlags()^Qt::WindowMaximizeButtonHint);

    cWindow = new ConfigWindow(this);

    QToolBar* toolBar = new QToolBar(this);
    toolBar->setObjectName(QStringLiteral("toolBar"));
    toolBar->setLayoutDirection(Qt::LeftToRight);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QAction* actionSave = new QAction(this);
    actionSave->setObjectName(QStringLiteral("actionSave"));
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/StreamControl/icons/fugue/bonus/icons-24/disk.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionSave->setIcon(icon1);
    actionSave->setShortcuts(QKeySequence::Save);
    connect(actionSave,SIGNAL( triggered() ),this,SLOT( saveData() ));

    QMenu *configMenu = new QMenu();
    QAction *actionConfig = new QAction("Configuration", this);
    configMenu->addAction(actionConfig);
    QAction *actionAlwaysOnTop = new QAction("Always on top", this);
    actionAlwaysOnTop->setCheckable(true);
    configMenu->addAction(actionAlwaysOnTop);


    QToolButton* configButton = new QToolButton();
    configButton->setMenu(configMenu);
    configButton->setPopupMode(QToolButton::InstantPopup);
    QIcon configIcon;
    configIcon.addFile(QString::fromUtf8(":/StreamControl/icons/fugue/bonus/icons-24/gear.png"), QSize(), QIcon::Normal, QIcon::Off);
    configButton->setIcon(configIcon);
    connect(actionConfig,SIGNAL( triggered() ),this,SLOT( openConfig() ));
    connect(actionAlwaysOnTop,SIGNAL(toggled(bool)),this,SLOT( toggleAlwaysOnTop(bool) ));



    //code to add non buttons to toolbar
    gameComboBox = new QComboBox(this);
    gameComboBox->setMinimumContentsLength(20);
    QLabel* spaceLabel = new QLabel("   ");
    QLabel* gameLabel = new QLabel("Game ");

    QAction* actionAddGame = new QAction(this);
    actionAddGame->setObjectName(QString("actionAddGame"));
    QIcon addIcon;
    addIcon.addFile(QString::fromUtf8(":/StreamControl/icons/fugue/bonus/icons-24/plus.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionAddGame->setIcon(addIcon);
    connect(actionAddGame,SIGNAL( triggered() ),this,SLOT( addGame() ));

    QAction* actionDelGame = new QAction(this);
    actionDelGame->setObjectName(QString("actionDelGame"));
    QIcon delIcon;
    delIcon.addFile(QString::fromUtf8(":/StreamControl/icons/fugue/bonus/icons-24/minus.png"), QSize(), QIcon::Normal, QIcon::Off);
    actionDelGame->setIcon(delIcon);
    connect(actionDelGame,SIGNAL( triggered() ),this,SLOT( delGame() ));


    toolBar->addAction(actionSave);
    toolBar->addWidget(configButton);
    toolBar->addWidget(spaceLabel);
    toolBar->addWidget(gameLabel);
    toolBar->addWidget(gameComboBox);
    toolBar->addAction(actionAddGame);
    toolBar->addAction(actionDelGame);
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

    loadSettings();
    loadLayout();
    loadData();

}

MainWindow::~MainWindow()
{
    //delete ui;
}

void MainWindow::loadSettings() {

    QFile file("settings.xml");
    QString xsplitPath;
    QString layoutPath;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QDomDocument doc;
        doc.setContent(&file);

        file.close();

        QDomElement settingsXML = doc.namedItem("settings").toElement();

        QDomElement xsplitPathE = settingsXML.namedItem("xsplitPath").toElement();
        QDomElement layoutPathE = settingsXML.namedItem("layoutPath").toElement();

        xsplitPath = xsplitPathE.text();
        layoutPath = layoutPathE.text();

        QDomElement useCDATAE = settingsXML.namedItem("useCDATA").toElement();

        if (useCDATAE.text() == "1") {
            useCDATA = true;
            settings["useCDATA"] = "1";
        } else {
            useCDATA = false;
            settings["useCDATA"] = "0";
        }

        QDomElement gamesE = settingsXML.namedItem("games").toElement();

        for(QDomNode n = gamesE.firstChild(); !n.isNull(); n = n.nextSibling())
        {
           QDomElement gameE = n.toElement();
           gameComboBox->addItem(gameE.text());
        }

        settings["xsplitPath"] = xsplitPath;
        settings["layoutPath"] = layoutPath;


    } else {
        QFile xsplitExe32("C:\\Program Files\\SplitMediaLabs\\XSplit\\XSplit.Core.exe");
        QFile xsplitExe("C:\\Program Files (x86)\\SplitMediaLabs\\XSplit\\XSplit.Core.exe");
        if (xsplitExe.exists() || xsplitExe32.exists()) {
            if (xsplitExe.exists()) {
                xsplitPath = "C:\\Program Files (x86)\\SplitMediaLabs\\XSplit\\";
            } else {
                xsplitPath = "C:\\Program Files\\SplitMediaLabs\\XSplit\\";
            }
            QMessageBox msgBox;

            msgBox.setText("XSplit Installation detected at default location. Saving settings.");
            msgBox.exec();

            settings["xsplitPath"] = xsplitPath;

        } else {
            QMessageBox msgBox;
            msgBox.setText("Please be sure to configure StreamControl before you start.");
            msgBox.exec();
            xsplitPath = "";
        }


        //set some default games if none already set
        gameComboBox->addItem("Super Street Fighter IV");
        gameComboBox->addItem("Ultimate Marvel vs Capcom 3");
        gameComboBox->addItem("Persona 4: Arena");
        gameComboBox->addItem("Tekken Tag Tournament 2");
        gameComboBox->addItem("Soul Calibur V");
        gameComboBox->addItem("King of Fighters XIII");

        saveSettings();
    }


}

void MainWindow::saveSettings() {
    QFile file("settings.xml");
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QDomDocument doc ("StreamControl");

    QDomElement settingsXML = doc.createElement("settings");
    doc.appendChild(settingsXML);

    QDomElement xsplitPathE = doc.createElement("xsplitPath");
    settingsXML.appendChild(xsplitPathE);

    QDomCDATASection xsplitPathT = doc.createCDATASection(settings["xsplitPath"]);
    xsplitPathE.appendChild(xsplitPathT);

    QDomElement layoutPathE = doc.createElement("layoutPath");
    settingsXML.appendChild(layoutPathE);

    QDomCDATASection layoutPathT = doc.createCDATASection(settings["layoutPath"]);
    layoutPathE.appendChild(layoutPathT);

    QDomElement useCDATAE = doc.createElement("useCDATA");
    settingsXML.appendChild(useCDATAE);

    QDomText useCDATAT = doc.createTextNode(settings["useCDATA"]);
    useCDATAE.appendChild(useCDATAT);

    QDomElement gamesE = doc.createElement("games");
    settingsXML.appendChild(gamesE);


    for (int i = 0; i < gameComboBox->count(); ++i) {
        QDomElement gameE = doc.createElement("game");
        gamesE.appendChild(gameE);
        QDomCDATASection gameText = doc.createCDATASection(gameComboBox->itemText(i));
        gameE.appendChild(gameText);
    }


    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString();
    file.close();
}

void MainWindow::loadData()
{
    QFile file(settings["xsplitPath"] + "streamcontrol.xml");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QDomDocument doc;
    doc.setContent(&file);

    file.close();

    QDomElement items = doc.namedItem("items").toElement();

    QDomElement game = items.namedItem("game").toElement();

    QMapIterator<QString, QWidget *> i(widgetList);
    while (i.hasNext()) {
        i.next();
        QString wType = widgetType[i.key()];
        QDomElement currElement = items.namedItem(i.key()).toElement();

        if (wType == "lineEdit") {
            ((QLineEdit*)widgetList[i.key()])->setText(currElement.text());
        } else if (wType == "spinBox") {
            ((QSpinBox*)widgetList[i.key()])->setValue(currElement.text().toInt());
        }
    }

    int gameIndex = gameComboBox->findText(game.text());
    if (gameIndex != -1) {
        gameComboBox->setCurrentIndex(gameIndex);
    } else if (game.text() != "") {
        gameComboBox->addItem(game.text());
        gameComboBox->setCurrentIndex(gameComboBox->findText(game.text()));
        saveSettings();
    }

}


void MainWindow::saveData()
{
    QFile file(settings["xsplitPath"] + "streamcontrol.xml");
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QDomDocument doc ("StreamControl");

    QDomElement items = doc.createElement("items");
    doc.appendChild(items);


    QDateTime current = QDateTime::currentDateTime();
    uint timestamp_t = current.toTime_t();

    QDomElement timestamp = doc.createElement("timestamp");
    items.appendChild(timestamp);

    QDomText timestampt = doc.createTextNode(QString::number(timestamp_t));;
    timestamp.appendChild(timestampt);

    QMapIterator<QString, QWidget *> i(widgetList);
    while (i.hasNext()) {
        i.next();
        QString wType = widgetType[i.key()];
        QDomElement newItem = doc.createElement(i.key());
        items.appendChild(newItem);

        if (wType == "lineEdit") {
            if (useCDATA) {
                QDomCDATASection newItemt = doc.createCDATASection(((QLineEdit*)widgetList[i.key()])->text());
                newItem.appendChild(newItemt);
            } else {
                QDomText newItemt = doc.createTextNode(((QLineEdit*)widgetList[i.key()])->text());
                newItem.appendChild(newItemt);
            }
        } else if (wType == "spinBox") {
            QDomText newItemt = doc.createTextNode(((QSpinBox*)widgetList[i.key()])->text());
            newItem.appendChild(newItemt);
        }
    }


    QDomElement gameE = doc.createElement("game");
    items.appendChild(gameE);

    if (useCDATA) {
        QDomCDATASection gameT = doc.createCDATASection(gameComboBox->currentText());
        gameE.appendChild(gameT);
    } else {
        QDomText gameT = doc.createTextNode(gameComboBox->currentText());
        gameE.appendChild(gameT);
    }


    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString();
    file.close();

}

void MainWindow::resetFields(QString widget)
{

    QListIterator<QString> i(resetList[widget]);
    while (i.hasNext()) {
        QString key = i.next();
        if (widgetType[key] == "spinBox") {
            ((QSpinBox*)widgetList[key])->setValue(0);
        } else if (widgetType[key] == "lineEdit") {
            ((QLineEdit*)widgetList[key])->setText("");
        }
    }
}

void MainWindow::swapFields(QString widget)
{
    QString swSet1 = swapSets[widget][0];
    QString swSet2 = swapSets[widget][1];


    QList<QString> swList1 = swapList[swSet1];
    QList<QString> swList2 = swapList[swSet2];

    for (int i = 0; i < swList1.size(); ++i) {
        QString currField = swList1[i];
        QString newField = swList2[i];
        QString tempData;

        if (widgetType[currField] == "lineEdit"){
            tempData = ((QLineEdit*)widgetList[currField])->text();
            ((QLineEdit*)widgetList[currField])->setText(((QLineEdit*)widgetList[newField])->text());
            ((QLineEdit*)widgetList[newField])->setText(tempData);
        } else if (widgetType[currField] == "spinBox") {
            tempData = ((QSpinBox*)widgetList[currField])->text();
            ((QSpinBox*)widgetList[currField])->setValue(((QSpinBox*)widgetList[newField])->text().toInt());
            ((QSpinBox*)widgetList[newField])->setValue(tempData.toInt());
        }
     }

}


void MainWindow::openConfig() {
    cWindow->setConfig(settings);
    cWindow->show();

    if (cWindow->exec() == 1) {
        QMap<QString, QString> configSettings = cWindow->getConfig();

        settings["xsplitPath"] = configSettings["xsplitPath"];
        settings["layoutPath"] = configSettings["layoutPath"];
        settings["useCDATA"] = configSettings["useCDATA"];

        if (settings["useCDATA"] == "1") {
            useCDATA = true;
        } else {
            useCDATA = false;
        }

        saveSettings();

        loadLayout();

        loadData();
    }
}

void MainWindow::addGame() {
    bool ok;
    QString game = QInputDialog::getText(this, tr("Input"),
                                              tr("Game:"), QLineEdit::Normal,
                                              "", &ok);
    if (ok && !game.isEmpty()) {
             gameComboBox->addItem(game);
             gameComboBox->setCurrentIndex(gameComboBox->findText(game));
             saveSettings();
    }
}

void MainWindow::delGame() {
    gameComboBox->removeItem(gameComboBox->currentIndex());
    saveSettings();
}

void MainWindow::toggleAlwaysOnTop(bool checked) {
    Qt::WindowFlags flags = this->windowFlags();
    if (checked)
    {
        this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        this->show();
    } else {
        this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        this->show();
    }
}

void MainWindow::loadLayout() {

    layoutIterator = 0;

    QDomDocument doc;

    QFile file(settings["layoutPath"]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        doc.setContent(&file);

        file.close();
    } else {

        doc = getDefaultLayout();

    }

    QDomElement layout = doc.namedItem("layout").toElement();

    //Set up the main window
    int layoutWidth = layout.attribute("width").toInt();
    int layoutHeight = layout.attribute("height").toInt();

    if (layoutWidth < 50) {
        layoutWidth = 50;
    }
    if (layoutHeight < 50) {
        layoutHeight = 50;
    }


    resize(layoutWidth, layoutHeight);
    setFixedSize(layoutWidth, layoutHeight);

    if(layout.attribute("tabbed") == "1") {
        centralWidget = new QTabWidget(this);
    } else {
        centralWidget = new QWidget(this);
    }

    setCentralWidget(centralWidget);


    if(layout.attribute("tabbed") == "1"){
        parseTabLayout(layout,centralWidget);
    } else {
        parseLayout(layout, centralWidget);
    }

}


void MainWindow::parseLayout(QDomElement element, QWidget *parent) {
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QString tagName = child.toElement().tagName();
        if(tagName == "label") {
            addLabel(child.toElement(), parent);
        } else if (tagName == "button") {
            addButton(child.toElement(), parent);
        } else if (tagName == "lineEdit") {
            addLineEdit(child.toElement(), parent);
        } else if (tagName == "spinBox") {
            addSpinBox(child.toElement(), parent);
        } else if (tagName == "tabSet") {
            QString newTabSet = addTabWidget(child.toElement(), parent);
            parseTabLayout(child.toElement(), visualList[newTabSet]);
        }

        child = child.nextSibling();
    }
}

void MainWindow::parseTabLayout(QDomElement element, QWidget *parent) {
    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        QString tagName = child.toElement().tagName();
        if(tagName == "tab") {
            QString newTabName = child.toElement().attribute("name");
            QString newTab = "tab"+QString::number(layoutIterator);
            visualList[newTab] =  new QWidget(parent);
            parseLayout(child.toElement(),visualList[newTab]);

            ((QTabWidget*)parent)->addTab(visualList[newTab],newTabName);

            layoutIterator++;
        }

        child = child.nextSibling();
    }
}

void MainWindow::addLabel(QDomElement element, QWidget *parent) {

    QString newLabel = "label"+QString::number(layoutIterator);
    visualList[newLabel] = new QLabel(parent);
    visualList[newLabel]->setObjectName(newLabel);
    visualList[newLabel]->setGeometry(QRect(element.attribute("x").toInt(),
                                                          element.attribute("y").toInt(),
                                                          element.attribute("width").toInt(),
                                                          element.attribute("height").toInt()));
    ((QLabel*)visualList[newLabel])->setText(element.text());

    layoutIterator++;
}

void MainWindow::addButton(QDomElement element, QWidget *parent) {
    if (element.attribute("type") == "reset") {

        QString newButton = element.attribute("id");

        QList<QString> resetL = CSV::parseFromString(element.attribute("reset"))[0];

        resetList.insert(newButton,resetL);

        visualList[newButton] = new QPushButton(parent);
        visualList[newButton]->setObjectName(newButton);
        visualList[newButton]->setGeometry(QRect(element.attribute("x").toInt(),
                                           element.attribute("y").toInt(),
                                           element.attribute("width").toInt(),
                                           element.attribute("height").toInt()));
        ((QPushButton*)visualList[newButton])->setText(element.text());

        connect(((QPushButton*)visualList[newButton]), SIGNAL(clicked()), resetMapper, SLOT(map()));
        resetMapper -> setMapping (((QPushButton*)visualList[newButton]), newButton) ;
        connect (resetMapper, SIGNAL(mapped(QString)), this, SLOT(resetFields(QString))) ;


    } else if (element.attribute("type") == "swap") {

        QString newButton = element.attribute("id");

        QList<QString> swapl1 = CSV::parseFromString(element.attribute("swapSet1"))[0];
        QList<QString> swapl2 = CSV::parseFromString(element.attribute("swapSet2"))[0];

        QList<QString> swapset;
        swapset.insert(0,newButton + "1");
        swapset.insert(1,newButton + "2");

        swapSets.insert(newButton,swapset);

        swapList.insert(newButton + "1",swapl1);

        swapList.insert(newButton + "2",swapl2);

        visualList[newButton] = new QPushButton(parent);
        visualList[newButton]->setObjectName(newButton);
        visualList[newButton]->setGeometry(QRect(element.attribute("x").toInt(),
                                                 element.attribute("y").toInt(),
                                                 element.attribute("width").toInt(),
                                                 element.attribute("height").toInt()));
        ((QPushButton*)visualList[newButton])->setText(element.text());

        connect(((QPushButton*)visualList[newButton]), SIGNAL(clicked()), swapMapper, SLOT(map()));
        swapMapper -> setMapping (((QPushButton*)visualList[newButton]), newButton) ;
        connect (swapMapper, SIGNAL(mapped(QString)), this, SLOT(swapFields(QString))) ;



    }
}

void MainWindow::addLineEdit(QDomElement element, QWidget *parent) {

    QString newLineEdit = element.attribute("id");

    widgetList[newLineEdit] = new QLineEdit(parent);
    widgetList[newLineEdit]->setObjectName(newLineEdit);
    widgetList[newLineEdit]->setGeometry(QRect(element.attribute("x").toInt(),
                                             element.attribute("y").toInt(),
                                             element.attribute("width").toInt(),
                                             element.attribute("height").toInt()));
    widgetType[newLineEdit] = "lineEdit";

}

void MainWindow::addSpinBox(QDomElement element, QWidget *parent) {

    QString newSpinBox = element.attribute("id");

    widgetList[newSpinBox] = new QSpinBox(parent);
    widgetList[newSpinBox]->setObjectName(newSpinBox);
    widgetList[newSpinBox]->setGeometry(QRect(element.attribute("x").toInt(),
                                              element.attribute("y").toInt(),
                                              element.attribute("width").toInt(),
                                              element.attribute("height").toInt()));
    if(!element.attribute("maximum").isEmpty()) {
        ((QSpinBox*)widgetList[newSpinBox])->setMaximum(element.attribute("maximum").toInt());
    }
    widgetType[newSpinBox] = "spinBox";

}

QString MainWindow::addTabWidget(QDomElement element, QWidget *parent) {

    QString tabSet = "tabSet"+QString::number(layoutIterator);
    visualList[tabSet] = new QTabWidget(parent);
    visualList[tabSet]->setObjectName(tabSet);
    visualList[tabSet]->setGeometry(QRect(element.attribute("x").toInt(),
                                                          element.attribute("y").toInt(),
                                                          element.attribute("width").toInt(),
                                                          element.attribute("height").toInt()));

    layoutIterator++;
    return tabSet;
}

QDomDocument MainWindow::getDefaultLayout() {
    QString xmlcontent = "<!DOCTYPE StreamControlLayout>\r\n<layout width=\"400\" height=\"140\" tabbed=\"1\">\r\n <tab name=\"Match Info\">\r\n  <label x=\"10\" y=\"10\" width=\"46\" height=\"13\">Player 1</label>\r\n  <label x=\"10\" y=\"40\" width=\"46\" height=\"13\">Player 2</label>\r\n  <label x=\"300\" y=\"10\" width=\"46\" height=\"13\">Rounds</label>\r\n  <lineEdit id=\"pName1\" x=\"60\" y=\"10\" width=\"171\" height=\"20\" dataset=\"players.txt\" />\r\n  <lineEdit id=\"pName2\" x=\"60\" y=\"40\" width=\"171\" height=\"20\" dataset=\"players.txt\" />\r\n  <spinBox id=\"pScore1\" x=\"250\" y=\"10\" width=\"42\" height=\"22\" maximum=\"999\" />\r\n  <spinBox id=\"pScore2\" x=\"250\" y=\"40\" width=\"42\" height=\"22\" maximum=\"999\" />\r\n  <spinBox id=\"rounds\" x=\"340\" y=\"10\" width=\"42\" height=\"22\" maximum=\"999\" />\r\n  <button type=\"reset\" x=\"300\" y=\"40\" width=\"41\" height=\"23\" tooltip=\"Reset the Scores\" id=\"reset\" reset=\"pScore1,pScore2\">Reset</button>\r\n  <button type=\"swap\" x=\"340\" y=\"40\" width=\"41\" height=\"23\" tooltip=\"Swap the Scores\" id=\"swap1\" swapSet1=\"pName1,pScore1\" swapSet2=\"pName2,pScore2\">Swap</button>\r\n </tab>\r\n <tab name=\"Commentary\">\r\n  <label x=\"10\" y=\"10\" width=\"46\" height=\"13\">Title 1</label>\r\n  <label x=\"10\" y=\"40\" width=\"46\" height=\"13\">Title 2</label>\r\n  <lineEdit id=\"cTitle1\" x=\"60\" y=\"10\" width=\"321\" height=\"20\" />\r\n  <lineEdit id=\"cTitle2\" x=\"60\" y=\"40\" width=\"321\" height=\"20\" />\r\n </tab>\r\n <tab name=\"Misc 1\">\r\n  <label x=\"10\" y=\"10\" width=\"46\" height=\"13\">mText 1</label>\r\n  <label x=\"10\" y=\"40\" width=\"46\" height=\"13\">mText 2</label>\r\n  <lineEdit id=\"mText1\" x=\"60\" y=\"10\" width=\"321\" height=\"20\" />\r\n  <lineEdit id=\"mText2\" x=\"60\" y=\"40\" width=\"321\" height=\"20\" />\r\n </tab>\r\n <tab name=\"Misc 2\">\r\n  <label x=\"10\" y=\"10\" width=\"46\" height=\"13\">mText 3</label>\r\n  <label x=\"10\" y=\"40\" width=\"46\" height=\"13\">mText 4</label>\r\n  <lineEdit id=\"mText3\" x=\"60\" y=\"10\" width=\"321\" height=\"20\" />\r\n  <lineEdit id=\"mText4\" x=\"60\" y=\"40\" width=\"321\" height=\"20\" />\r\n </tab>\r\n</layout>\r\n";
    QDomDocument doc;
    doc.setContent(xmlcontent);
    return doc;
}
