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

#ifndef UTA_UNIVERSALCONFIGURATIONMANAGER_H
#define UTA_UNIVERSALCONFIGURATIONMANAGER_H

#include <QScreen>
#include <QCloseEvent>
#include <QWidget>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QValueAxis>
#include <ui_Uta_UniversalTsnAnalyzer.h>
#include <TsnTab/Uta_TsnTab.h>
#include <HelpMenu/Uta_HelpMenuAbout.h>

#ifdef WINPCAP
    #include <winsock2.h>
    #include <WpdPack/Include/pcap.h>
#endif

#ifdef NPCAP
    #include <winsock2.h>
    #include <tchar.h>
    #include <NpdPack/Include/pcap.h>
#endif

using namespace QtCharts;

class Uta_TsnTab;
class Uta_HelpMenuAbout;

#define Uta_Version_Nr               "1.1.00"

#define Uta_MainHeight               820
#define Uta_MainWidth                1380

class Uta_UniversalTsnAnalyzer : public QMainWindow, public Ui::Uta_UniversalTsnAnalyzer
{
    Q_OBJECT

    public:
        Uta_UniversalTsnAnalyzer(QMainWindow *parent = 0, int open_gl = 1);
        ~Uta_UniversalTsnAnalyzer();

        int uta_resize(int height, int width);

        // If OpenGl shall be used
        int use_open_gl;
        
        // Tsn tab
        Uta_TsnTab* tsn_tab;

private:
        void resizeEvent(QResizeEvent* event);
        void closeEvent(QCloseEvent *event);

        Uta_HelpMenuAbout about_screen;

private slots:
        // File menu
        void uta_file_exit_clicked(void);

        // Help menu
        void uta_help_about_clicked(void);
};

#endif // UTA_UNIVERSALCONFIGURATIONMANAGER_H
