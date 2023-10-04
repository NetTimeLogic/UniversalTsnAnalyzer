//*****************************************************************************************
// Project:     UniversalConfigurationManager
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

#ifndef UTA_TSN_H
#define UTA_TSN_H

#include <pcap.h>
#include <time.h>

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QValueAxis>
#include <Uta_UniversalTsnAnalyzer.h>
#include <Uta_TsnPrioScreen.h>

class Uta_UniversalTsnAnalyzer;
class Uta_TsnPrioScreen;

namespace Ui {
class Uta_TsnTab;
}

using namespace QtCharts;

class Uta_TsnTab : public QWidget
{
    Q_OBJECT

public:
    Uta_TsnTab(Uta_UniversalTsnAnalyzer* parent);
    ~Uta_TsnTab();

    int tsn_resize(int height, int width);

public:
    Ui::Uta_TsnTab *ui;
    Uta_UniversalTsnAnalyzer* uta;
    QList<pcap_if_t*> tsn_interfaces;
    QLineSeries* tsn_frame_series;
    QLineSeries* tsn_frame_prio_series[8];

    timeval measure_start;

    unsigned int tsn_raw_frame_count;
    unsigned int tsn_frame_count;
    unsigned int tsn_byte_count;
    unsigned int tsn_measurement_count;
    unsigned int tsn_frame_prio_count[8];
    unsigned int tsn_byte_prio_count[8];
    unsigned int tsn_byte_counts[100000];
    unsigned int tsn_byte_prio_counts[8][100000];
    unsigned int tsn_frame_counts[100000];
    unsigned int tsn_frame_prio_counts[8][100000];
    unsigned int tsn_prios[9];
    unsigned int tsn_hw_ts;
    unsigned int tsn_align;
    unsigned int tsn_persistence;
    unsigned int tsn_clear_persistence;
    unsigned int tsn_byte_mode;
    unsigned int tsn_zoom_factor;

private:
    Uta_TsnPrioScreen* ui_prio;

    // TSN Hc tab
    QTimer* tsn_timer;

    QChart* tsn_frame_chart;
    QChartView* tsn_frame_chart_view;
    QValueAxis* tsn_frame_chart_x_axis;
    QValueAxis* tsn_frame_chart_y_axis;
    
    pcap_if_t *all_net_ports;

    void tsn_measure_values(void);

private slots:
    // TSN tab
    void tsn_measure_button_clicked(void);
    void tsn_measure_values_timer(void);
    void tsn_auto_refresh_button_clicked(void);
    void tsn_save_button_clicked(void);
    void tsn_config_prios_button_clicked(void);
    void tsn_zoom_in_button_clicked(void);
    void tsn_zoom_out_button_clicked(void);
};

#endif // UTA_TSN_H
