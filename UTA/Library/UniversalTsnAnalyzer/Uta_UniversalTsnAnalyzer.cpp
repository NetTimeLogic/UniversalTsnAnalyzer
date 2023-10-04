//*****************************************************************************************
// Project:     UniversalTsnAnalyzer
//
// Author:      Sven Meier, NetTimeLogic GmbH
//
// License:     Copyright (c) 2022, NetTimeLogic GmbH, Switzerland, Sven Meier <contact@nettimelogic.com>
//              All rights reserved.
//
//              THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND/OR MODIFY
//              IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC LICENSE AS
//              PUBLISHED BY THE FREE SOFTWARE FOUNDATION, VERSION 3.
//
//              THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT
//              WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//              MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. SEE THE GNU
//              LESSER GENERAL LESSER PUBLIC LICENSE FOR MORE DETAILS.
//
//              YOU SHOULD HAVE RECEIVED A COPY OF THE GNU LESSER GENERAL PUBLIC LICENSE
//              ALONG WITH THIS PROGRAM. IF NOT, SEE <http://www.gnu.org/licenses/>.
//
//*****************************************************************************************

#include <Uta_UniversalTsnAnalyzer.h>

using namespace std;
#include <iostream>
#include <streambuf>
#include <string>

Uta_UniversalTsnAnalyzer::Uta_UniversalTsnAnalyzer(QMainWindow *parent, int open_gl) : QMainWindow(parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();

    // set this first
    use_open_gl = open_gl;
    
    cout << "INFO: " << "Screen size: " << width << "x" << height << endl;

    setupUi(this);

    // File menu
    connect(Uta_Exit, SIGNAL(triggered()), this, SLOT(uta_file_exit_clicked()));

    // Help menu
    connect(Uta_About, SIGNAL(triggered()), this, SLOT(uta_help_about_clicked()));

    // Tsn Tab
    tsn_tab = new Uta_TsnTab(this);
    Uta_MainTab->addTab(tsn_tab, "Tsn"); // show this at startup

    if (height > (Uta_MainHeight+2))
    {
        height = (Uta_MainHeight+2);
    }
    if (width > Uta_MainWidth)
    {
        width = Uta_MainWidth;
    }
    this->resize(width, height);
}


Uta_UniversalTsnAnalyzer::~Uta_UniversalTsnAnalyzer()
{
    delete tsn_tab;
}

int Uta_UniversalTsnAnalyzer::uta_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    cout << "INFO: " << "Resize to: " << width << "x" << height << endl;

    height_delta = (height-Uta_MainHeight);
    width_delta = (width-Uta_MainWidth);

    Uta_MainTab->setFixedHeight(Uta_MainHeight-20+height_delta);
    Uta_MainTab->setFixedWidth(Uta_MainWidth+width_delta);

    Uta_MainMenu->setFixedWidth(Uta_MainWidth+width_delta);

    // Tsn Tab
    tsn_tab->tsn_resize(height, width);

    updateGeometry();

    return 0;
}

/******************************************************************/
// Resize
/******************************************************************/
void Uta_UniversalTsnAnalyzer::resizeEvent(QResizeEvent* event)
{
   QSize new_size = this->size();

   uta_resize(new_size.height(), new_size.width());

}

/******************************************************************/
// X Close
/******************************************************************/
void Uta_UniversalTsnAnalyzer::closeEvent(QCloseEvent *event)
{
    delete tsn_tab;
    exit(0);
}

/******************************************************************/
// File menu
/******************************************************************/
void Uta_UniversalTsnAnalyzer::uta_file_exit_clicked(void)
{
    delete tsn_tab;
    exit(0);
}

/******************************************************************/
// Help menu
/******************************************************************/
void Uta_UniversalTsnAnalyzer::uta_help_about_clicked(void)
{
    about_screen.show();
}


