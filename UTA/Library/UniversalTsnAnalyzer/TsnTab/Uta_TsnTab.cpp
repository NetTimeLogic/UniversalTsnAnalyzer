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

#include <QStandardPaths>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QTime>

#include <Uta_TsnTab.h>
#include <ui_Uta_TsnTab.h>
#include <Uta_TsnPrioScreen.h>
#include <ui_Uta_TsnPrioScreen.h>

using namespace std;

#include <pcap.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <streambuf>
#include <string>
#include <tchar.h>

void tsn_paket_handler(unsigned char *user, const struct pcap_pkthdr *phdr, const unsigned char *pdata)
{
    Uta_TsnTab* tsn_tab = (Uta_TsnTab*)user;
    long long int ts_delta;
    unsigned int ts_idx;
    unsigned int temp_zoom;
    unsigned int temp_data;
    unsigned int temp_nanosecond;
    unsigned int temp_vlan;
    unsigned int temp_offset;
    unsigned long long temp_fraction;
    timeval temp_ts;

    if (tsn_tab->tsn_hw_ts == 2)
    {
        temp_offset = 28;
        temp_data = 0;
        for (unsigned int i = 15; i >= 12; i--)
        {
            temp_data = temp_data << 8;
            temp_data += pdata[i];
        }
        temp_ts.tv_sec = temp_data;

        temp_data = 0;
        for (unsigned int i = 19; i >= 16; i--)
        {
            temp_data = temp_data << 8;
            temp_data += pdata[i];
        }
        temp_fraction = ((unsigned long long)temp_data);

        temp_data = 0;
        for (unsigned int i = 15; i >= 12; i--)
        {
            temp_data = temp_data << 8;
            temp_data += pdata[i];
        }
        temp_fraction += (((unsigned long long)temp_data) * 0x100000000);

        temp_ts.tv_sec = (temp_fraction / 1000000000);
        temp_ts.tv_usec = ((temp_fraction % 1000000000) / 1000);
        temp_nanosecond = ((temp_fraction % 1000000000) % 1000);
    }
    else if (tsn_tab->tsn_hw_ts == 1)
    {
        temp_offset = 0;
        temp_data = 0;
        for (unsigned int i = (phdr->caplen - 10); i < (phdr->caplen - 6); i++)
        {
            temp_data = temp_data << 8;
            temp_data += pdata[i];
        }
        temp_ts.tv_sec = temp_data;

        temp_data = 0;
        for (unsigned int i = (phdr->caplen - 6); i < (phdr->caplen - 2); i++)
        {
            temp_data = temp_data << 8;
            temp_data += pdata[i];
        }
        temp_data &= 0x3FFFFFFF;
        temp_ts.tv_usec = temp_data / 1000;
        temp_nanosecond = temp_data % 1000;
    }
    else
    {
        temp_offset = 0;
        temp_ts = phdr->ts;
        temp_nanosecond = 0;
    }

    if ((pdata[temp_offset+12] == 0x81) && (pdata[temp_offset+13] == 0x00))
    {
        temp_vlan = pdata[temp_offset+14];
        temp_vlan = temp_vlan >> 5;
        temp_vlan = temp_vlan & 0x07;
    }
    else
    {
        temp_vlan = 8; // not a possible VLAN Prio
    }

    if (tsn_tab->tsn_raw_frame_count == 0)
    {
        tsn_tab->measure_start = temp_ts;
        if (tsn_tab->tsn_align != 0)
        {
            tsn_tab->measure_start.tv_usec -= (temp_ts.tv_usec % 10000);
            tsn_tab->measure_start.tv_usec += 10000;
            if (tsn_tab->measure_start.tv_usec >= 1000000)
            {
                tsn_tab->measure_start.tv_usec -= 1000000;
                tsn_tab->measure_start.tv_sec++;
            }
        }
    }

    tsn_tab->tsn_raw_frame_count++;
    ts_delta = ((temp_ts.tv_sec - tsn_tab->measure_start.tv_sec) * 1000000000) + ((temp_ts.tv_usec - tsn_tab->measure_start.tv_usec) * 1000) + temp_nanosecond;

    if (ts_delta >= 0)
    {
        if (tsn_tab->tsn_zoom_factor >= 100)
        {
            temp_zoom = tsn_tab->tsn_zoom_factor/100;
            ts_idx = ts_delta/10000;
        }
        else if (tsn_tab->tsn_zoom_factor >= 10)
        {
            temp_zoom = tsn_tab->tsn_zoom_factor/10;
            ts_idx = ts_delta/1000;
        }
        else
        {
            temp_zoom = 1;
            ts_idx = ts_delta/100;
        }

        if (ts_idx < (1000*temp_zoom))
        {
            tsn_tab->tsn_byte_counts[ts_idx] = tsn_tab->tsn_byte_counts[ts_idx] + phdr->caplen;
            tsn_tab->tsn_frame_counts[ts_idx] = tsn_tab->tsn_frame_counts[ts_idx] + 1;
            tsn_tab->tsn_byte_count = tsn_tab->tsn_byte_count + phdr->caplen;
            tsn_tab->tsn_frame_count = tsn_tab->tsn_frame_count + 1;

            for (int i = 0; i < 8; i++)
            {
                if (tsn_tab->tsn_prios[temp_vlan] == (i+1))
                {
                    tsn_tab->tsn_byte_prio_counts[i][ts_idx] = tsn_tab->tsn_byte_prio_counts[i][ts_idx] + phdr->caplen;
                    tsn_tab->tsn_frame_prio_counts[i][ts_idx] = tsn_tab->tsn_frame_prio_counts[i][ts_idx] + 1;
                    tsn_tab->tsn_byte_prio_count[i] = tsn_tab->tsn_byte_prio_count[i] + phdr->caplen + 4 + 8 + 12; //+ CRC + PREAMBLE + ITF
                    tsn_tab->tsn_frame_prio_count[i] = tsn_tab->tsn_frame_prio_count[i] + 1;
                }
            }
        }
    }

}

int load_npcap_dlls()
{
    TCHAR npcap_dir[512];
    unsigned int len;
    len = GetSystemDirectory(npcap_dir, 480);
    if (!len)
    {
        cout << "ERROR: " << "error in GetSystemDirectory: " << GetLastError() << endl;
        return -1;
    }
    _tcscat_s(npcap_dir, 512, TEXT("\\Npcap"));
    if (SetDllDirectory(npcap_dir) == 0)
    {
        cout << "ERROR: " << "error in SetDllDirectory:" << GetLastError() << endl;
        return -1;
    }
    cout << "INFO: " << "NPCAP Dlls loaded" << endl;
    return 0;
}

Uta_TsnTab::Uta_TsnTab(Uta_UniversalTsnAnalyzer *parent) : QWidget()
{
    pcap_if_t *net_port;
    pcap_addr_t *net_port_addr;
    char err[PCAP_ERRBUF_SIZE];

    uta = parent;

    ui = new Ui::Uta_TsnTab();
    ui->setupUi(this);

    if (0 != load_npcap_dlls())
    {
        cout << "ERROR: " << "could not load Npcap" << endl;
        exit(-1);
    }
    if (pcap_findalldevs(&all_net_ports, err) != -1)
    {
        for(net_port = all_net_ports; net_port; net_port = net_port->next)
        {
            for(net_port_addr=net_port->addresses; net_port_addr; net_port_addr = net_port_addr->next)
            {
                if (net_port_addr->addr->sa_family == AF_INET)
                {
                    cout << "==========================" << endl;
                    cout << "Description: " << net_port->description << endl;
                    cout << "Name: " << net_port->name << endl;
                    unsigned char* temp_addr = (unsigned char*)(&((struct sockaddr_in*)net_port_addr->addr)->sin_addr.s_addr);
                    char temp_addr_string[3*4+3+1];
                    sprintf(temp_addr_string, "%d.%d.%d.%d", temp_addr[0], temp_addr[1], temp_addr[2], temp_addr[3]);
                    cout << "Addr:" << temp_addr_string << endl;
                    QString temp_port_name;
                    temp_port_name.append(net_port->description);
                    temp_port_name.append(":");
                    temp_port_name.append(temp_addr_string);
                    ui->TsnEthPortComboBox->addItem(temp_port_name);
                    tsn_interfaces.append(net_port);

                }
            }
        }
    }

    tsn_byte_mode = 0;
    tsn_zoom_factor = 100;
    tsn_align = 0;
    tsn_persistence = 0;
    tsn_clear_persistence = 1;

    ui->TsnZoomInButton->setEnabled(true); // now we can zoom in for sure
    ui->TsnZoomOutButton->setEnabled(true); // now we can zoom out for sure

    tsn_timer = new QTimer(this);
    tsn_timer->stop();

    connect(ui->TsnMeasureButton, SIGNAL(clicked()), this, SLOT(tsn_measure_button_clicked()));
    connect(ui->TsnSaveButton, SIGNAL(clicked()), this, SLOT(tsn_save_button_clicked()));
    connect(ui->TsnAutoRefreshButton, SIGNAL(clicked()), this, SLOT(tsn_auto_refresh_button_clicked()));
    connect(ui->TsnConfigPriosButton, SIGNAL(clicked()), this, SLOT(tsn_config_prios_button_clicked()));
    connect(ui->TsnZoomInButton, SIGNAL(clicked()), this, SLOT(tsn_zoom_in_button_clicked()));
    connect(ui->TsnZoomOutButton, SIGNAL(clicked()), this, SLOT(tsn_zoom_out_button_clicked()));
    connect(tsn_timer, SIGNAL(timeout()), this, SLOT(tsn_measure_values_timer()));

    tsn_frame_series = new QLineSeries();
    if (uta->use_open_gl == 1)
    {
        tsn_frame_series->setUseOpenGL(true);
    }
    tsn_frame_series->setName("All");
    tsn_frame_series->setColor(QColor(128, 128, 128, 255));

    for (int i = 0; i < 8; i++)
    {
        QString temp_name;
        temp_name.append("Prio ");
        temp_name.append(QString::number(i));
        tsn_frame_prio_series[i] = new QLineSeries();
        if (uta->use_open_gl == 1)
        {
            tsn_frame_prio_series[i]->setUseOpenGL(true);
        }
        tsn_frame_prio_series[i]->setName(temp_name);
    }
    tsn_frame_prio_series[0]->setColor(QColor(200, 0, 0, 255));
    tsn_frame_prio_series[1]->setColor(QColor(220, 150, 10, 255));
    tsn_frame_prio_series[2]->setColor(QColor(220, 220, 0, 255));
    tsn_frame_prio_series[3]->setColor(QColor(10, 200, 10, 255));
    tsn_frame_prio_series[4]->setColor(QColor(0, 200, 200, 255));
    tsn_frame_prio_series[5]->setColor(QColor(0, 0, 200, 255));
    tsn_frame_prio_series[6]->setColor(QColor(100, 0, 220, 255));
    tsn_frame_prio_series[7]->setColor(QColor(220, 0, 220, 255));

    tsn_frame_chart = new QChart();
    tsn_frame_chart->setContentsMargins(0, 0, 0, 0);
    tsn_frame_chart->setBackgroundRoundness(0);
    tsn_frame_chart->setBackgroundBrush(Qt::white);
    tsn_frame_chart->addSeries(tsn_frame_series);

    tsn_frame_chart_x_axis = new QValueAxis;
    tsn_frame_chart_x_axis->setLabelFormat("%i");
    tsn_frame_chart_x_axis->setTitleText("Time in 10us steps");
    tsn_frame_chart_x_axis->setTickCount(11);
    tsn_frame_chart_x_axis->setMin(0);
    tsn_frame_chart_x_axis->setMax(1000);
    tsn_frame_chart->addAxis(tsn_frame_chart_x_axis, Qt::AlignBottom);
    tsn_frame_series->attachAxis(tsn_frame_chart_x_axis);

    tsn_frame_chart_y_axis = new QValueAxis;
    tsn_frame_chart_y_axis->setLabelFormat("%i");
    tsn_frame_chart_y_axis->setTitleText("Nr. of Frames");
    tsn_frame_chart_y_axis->setTickCount(11);
    tsn_frame_chart_y_axis->setMin(0);
    tsn_frame_chart_y_axis->setMax(20);
    tsn_frame_chart->addAxis(tsn_frame_chart_y_axis, Qt::AlignLeft);
    tsn_frame_series->attachAxis(tsn_frame_chart_y_axis);

    tsn_frame_chart_view = new QChartView(tsn_frame_chart);
    tsn_frame_chart_view->setRenderHint(QPainter::Antialiasing);

    ui->TsnFrameChartLayout->addWidget(tsn_frame_chart_view, 0, 0);

    tsn_frame_chart->legend()->setVisible(true);

    memset(tsn_prios, 0, sizeof(tsn_prios));
    ui_prio = new Uta_TsnPrioScreen(this);

}

Uta_TsnTab::~Uta_TsnTab()
{
    tsn_timer->stop();
    delete ui;
    ui_prio->close();
    delete ui_prio;
    delete tsn_timer;
    delete tsn_frame_series;
    for (int i = 0; i < 8; i++)
    {
        delete tsn_frame_prio_series[i];
    }
    delete tsn_frame_chart_view;
    delete tsn_frame_chart_x_axis;
    delete tsn_frame_chart_y_axis;
    tsn_interfaces.clear();
    pcap_freealldevs(all_net_ports);
}

int Uta_TsnTab::tsn_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    height_delta = (height-Uta_MainHeight);
    width_delta = (width-Uta_MainWidth);

    ui->TsnFrameChartValue->setFixedHeight(700+height_delta);
    ui->TsnFrameChartValue->setFixedWidth(1140+width_delta);

    updateGeometry();

    return 0;
}

void Uta_TsnTab::tsn_measure_values(void)
{
    QElapsedTimer temp_timer;
    QString temp_string;
    unsigned int temp_zoom;
    unsigned int temp_frame_count;
    unsigned int temp_byte_count;
    double temp_bandwidth;
    unsigned int temp_bandwidth_div;
    pcap_t* net_port_fd;
    char err[PCAP_ERRBUF_SIZE];
    int pcount = 0;
    unsigned int temp_max_y;

    gettimeofday(&measure_start, 0);

    if ((net_port_fd = pcap_create(tsn_interfaces.at(ui->TsnEthPortComboBox->currentIndex())->name, // name of the device
                                 err        // error buffer
                                 )) == NULL)
    {
        cout << "ERROR: " << "could not open ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
        return;
    }
    else
    {
        //cout << "INFO: " << "opened ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
    }

    if (0 != pcap_set_buffer_size(net_port_fd, 64*1024*1024))
    {
        cout << "ERROR: " << "could not set buffer size to 16MB on ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
        return;
    }
    else
    {
        //cout << "INFO: " << "changed buffer size to 16MB on ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
    }

    if (0 != pcap_set_promisc(net_port_fd, 1))
    {
        cout << "ERROR: " << "could not set promiscous mode on ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
        return;
    }
    else
    {
        //cout << "INFO: " << "set promiscous mode on ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
    }


    if (0 != pcap_setnonblock(net_port_fd, 1, err))
    {
        cout << "ERROR: " << "could not set to non-blocking ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
        return;
    }
    else
    {
        //cout << "INFO: " << "set to non-blocking ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
    }

    if (0 > pcap_activate(net_port_fd))
    {
        cout << "ERROR: " << "could not activate ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
        return;
    }
    else
    {
        //cout << "INFO: " << "activated ethernet interface:" << ui->TsnEthPortComboBox->currentText().toLatin1().constData() << endl;
    }

    if (ui->TsnPsTsCheckBox->isChecked() == true)
    {
        tsn_hw_ts = 2;
    }
    else if (ui->TsnVssTsCheckBox->isChecked() == true)
    {
        tsn_hw_ts = 1;
    }
    else
    {
        tsn_hw_ts = 0;
    }

    if (ui->TsnAlignCheckBox->isChecked() == true)
    {
        tsn_align = 1;
    }
    else
    {
        tsn_align = 0;
    }

    if (ui->TsnBytesCheckBox->isChecked() == true)
    {
        tsn_byte_mode = 1;
    }
    else
    {
        tsn_byte_mode = 0;
    }

    if (ui->TsnPersistenceCheckBox->isChecked() == true)
    {
        tsn_persistence = 1;
        tsn_align = 1; // we need this
    }
    else
    {
        tsn_persistence = 0;
    }


    tsn_prios[0] = ui_prio->ui->TsnVlanPrio0ComboBox->currentIndex();
    tsn_prios[1] = ui_prio->ui->TsnVlanPrio1ComboBox->currentIndex();
    tsn_prios[2] = ui_prio->ui->TsnVlanPrio2ComboBox->currentIndex();
    tsn_prios[3] = ui_prio->ui->TsnVlanPrio3ComboBox->currentIndex();
    tsn_prios[4] = ui_prio->ui->TsnVlanPrio4ComboBox->currentIndex();
    tsn_prios[5] = ui_prio->ui->TsnVlanPrio5ComboBox->currentIndex();
    tsn_prios[6] = ui_prio->ui->TsnVlanPrio6ComboBox->currentIndex();
    tsn_prios[7] = ui_prio->ui->TsnVlanPrio7ComboBox->currentIndex();
    tsn_prios[8] = ui_prio->ui->TsnNoVlanComboBox->currentIndex();

    tsn_raw_frame_count = 0;

    if ((tsn_clear_persistence != 0) || (tsn_persistence == 0))
    {
        tsn_measurement_count = 0;
        tsn_clear_persistence = 0;
        tsn_frame_count = 0;
        memset(&tsn_byte_counts, 0, sizeof(tsn_byte_counts));
        memset(&tsn_frame_counts, 0, sizeof(tsn_frame_counts));
        for (int i = 0; i < 8; i++)
        {
            tsn_byte_prio_count[i] = 0;
            tsn_frame_prio_count[i] = 0;
            memset(&tsn_byte_prio_counts[i], 0, sizeof(tsn_byte_prio_counts[i]));
            memset(&tsn_frame_prio_counts[i], 0, sizeof(tsn_frame_prio_counts[i]));
        }
    }

    temp_timer.start();

    if (tsn_zoom_factor >= 100)
    {
        temp_zoom = tsn_zoom_factor/100;
    }
    else if (tsn_zoom_factor >= 10)
    {
        temp_zoom = tsn_zoom_factor/10;
    }
    else
    {
        temp_zoom = 1;
    }

    tsn_measurement_count++;
    while(!temp_timer.hasExpired(20+(10*temp_zoom))) // 20ms + zoom
    {
        if ((pcount = pcap_dispatch(net_port_fd, -1, &tsn_paket_handler, (unsigned char*)(this))) < 0)
        {
            cout << "ERROR: " << "dispatching failed: " << pcap_geterr(net_port_fd) << endl;
        }
    }
    pcap_close(net_port_fd);

    // disable updates
    tsn_frame_chart_view->setUpdatesEnabled(false);

    //remove all Series
    foreach (QAbstractSeries *i, tsn_frame_chart->series())
    {
        tsn_frame_chart->removeSeries(i);
    }

    tsn_frame_series->clear();
    for (int i = 0; i < 8; i++)
    {
        tsn_frame_prio_series[i]->clear();
    }

    if (tsn_byte_mode == 1)
    {
        temp_max_y = (200*4)/5;
    }
    else
    {
        temp_max_y = (10*4)/5;
    }
    for (unsigned int i = 0; i < (1000*temp_zoom); i++)
    {
        if (tsn_byte_mode == 1)
        {
            tsn_frame_series->append(i, tsn_byte_counts[i]);
            if (temp_max_y < tsn_byte_counts[i])
            {
                temp_max_y = tsn_byte_counts[i];
            }
            for (int j = 0; j < 8; j++)
            {
                tsn_frame_prio_series[j]->append(i, tsn_byte_prio_counts[j][i]);
            }
        }
        else
        {
            tsn_frame_series->append(i, tsn_frame_counts[i]);
            if (temp_max_y < tsn_frame_counts[i])
            {
                temp_max_y = tsn_frame_counts[i];
            }
            for (int j = 0; j < 8; j++)
            {
                tsn_frame_prio_series[j]->append(i, tsn_frame_prio_counts[j][i]);
            }
        }

    }
    if (true == ui_prio->ui->TsnAllFramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_series);
    }
    if (true == ui_prio->ui->TsnPrio0FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[0]);
    }
    if (true == ui_prio->ui->TsnPrio1FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[1]);
    }
    if (true == ui_prio->ui->TsnPrio2FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[2]);
    }
    if (true == ui_prio->ui->TsnPrio3FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[3]);
    }
    if (true == ui_prio->ui->TsnPrio4FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[4]);
    }
    if (true == ui_prio->ui->TsnPrio5FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[5]);
    }
    if (true == ui_prio->ui->TsnPrio6FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[6]);
    }
    if (true == ui_prio->ui->TsnPrio7FramesGraphCheckBox->isChecked())
    {
        tsn_frame_chart->addSeries(tsn_frame_prio_series[7]);
    }

    tsn_frame_chart->removeAxis(tsn_frame_chart_x_axis);
    tsn_frame_chart_x_axis->setLabelFormat("%i");
    tsn_frame_chart_x_axis->setTickCount(11);

    if (tsn_zoom_factor >= 100)
    {
        tsn_frame_chart_x_axis->setTitleText("Time in 10us steps");
        if (tsn_zoom_factor > 1000)
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/1000)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
        else
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/100)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
    }
    else
    {   if (tsn_zoom_factor == 1)
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 100ns steps");
            tsn_frame_chart_x_axis->setMinorTickCount(0);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000);
        }
        else
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 1us steps");
            tsn_frame_chart_x_axis->setMinorTickCount(9);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/10));
        }
    }

    tsn_frame_chart->addAxis(tsn_frame_chart_x_axis, Qt::AlignBottom);
    foreach (QAbstractSeries *i, tsn_frame_chart->series())
    {
        i->attachAxis(tsn_frame_chart_x_axis);
    }

    tsn_frame_chart->removeAxis(tsn_frame_chart_y_axis);
    tsn_frame_chart_y_axis->setLabelFormat("%i");

    tsn_frame_chart_y_axis->setMax(((temp_max_y*5)/4)); // leave some gap
    //if (tsn_byte_mode == 1)
    //{
    //    tsn_frame_chart_y_axis->setMin(0);
    //    if (tsn_persistence == 0)
    //    {
    //        tsn_frame_chart_y_axis->setMax(2000); // this is more than the max possible at 1g
    //    }
    //    else
    //    {
    //        tsn_frame_chart_y_axis->setMax(((temp_max_y*5)/4)); // leave some gap
    //    }
    //    tsn_frame_chart_y_axis->setTitleText("Nr. of Bytes");
    //}
    //else
    //{
    //    tsn_frame_chart_y_axis->setMin(0);
    //    if (tsn_persistence == 0)
    //    {
    //        tsn_frame_chart_y_axis->setMax(20); // this is more than the max possible at 1g
    //    }
    //    else
    //    {
    //        tsn_frame_chart_y_axis->setMax(((temp_max_y*5)/4)); // leave some gap
    //    }
    //    tsn_frame_chart_y_axis->setTitleText("Nr. of Frames");
    //}
    tsn_frame_chart_y_axis->setTickCount(11);
    tsn_frame_chart->addAxis(tsn_frame_chart_y_axis, Qt::AlignLeft);
    foreach (QAbstractSeries *i, tsn_frame_chart->series())
    {
        i->attachAxis(tsn_frame_chart_y_axis);
    }

    temp_byte_count = 0;
    temp_frame_count = 0;
    //if (true == ui_prio->ui->TsnAllFramesGraphCheckBox->isChecked())
    //{
    //    temp_byte_count += tsn_byte_count;
    //    temp_frame_count += tsn_frame_count;
    //}
    //else
    //{
        if (true == ui_prio->ui->TsnPrio0FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[0];
            temp_frame_count += tsn_frame_prio_count[0];
        }
        if (true == ui_prio->ui->TsnPrio1FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[1];
            temp_frame_count += tsn_frame_prio_count[1];
        }
        if (true == ui_prio->ui->TsnPrio2FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[2];
            temp_frame_count += tsn_frame_prio_count[2];
        }
        if (true == ui_prio->ui->TsnPrio3FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[3];
            temp_frame_count += tsn_frame_prio_count[3];
        }
        if (true == ui_prio->ui->TsnPrio4FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[4];
            temp_frame_count += tsn_frame_prio_count[4];
        }
        if (true == ui_prio->ui->TsnPrio5FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[5];
            temp_frame_count += tsn_frame_prio_count[5];
        }
        if (true == ui_prio->ui->TsnPrio6FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[6];
            temp_frame_count += tsn_frame_prio_count[6];
        }
        if (true == ui_prio->ui->TsnPrio7FramesGraphCheckBox->isChecked())
        {
            temp_byte_count += tsn_byte_prio_count[7];
            temp_frame_count += tsn_frame_prio_count[7];
        }
    //}

    temp_string.clear();
    temp_string.append(QString("%1").arg(temp_frame_count, 10, 10, QLatin1Char('0')));
    ui->TsnNrOfFramesValue->setText(temp_string);

    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[0], 10, 10, QLatin1Char('0')));
    ui->TsnPrio0NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[1], 10, 10, QLatin1Char('0')));
    ui->TsnPrio1NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[2], 10, 10, QLatin1Char('0')));
    ui->TsnPrio2NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[3], 10, 10, QLatin1Char('0')));
    ui->TsnPrio3NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[4], 10, 10, QLatin1Char('0')));
    ui->TsnPrio4NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[5], 10, 10, QLatin1Char('0')));
    ui->TsnPrio5NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[6], 10, 10, QLatin1Char('0')));
    ui->TsnPrio6NrOfFramesValue->setText(temp_string);
    temp_string.clear();
    temp_string.append(QString("%1").arg(tsn_frame_prio_count[7], 10, 10, QLatin1Char('0')));
    ui->TsnPrio7NrOfFramesValue->setText(temp_string);

    if (tsn_zoom_factor >= 100)
    {
        // 10us steps
        temp_bandwidth_div = temp_zoom * 1000 * 10000;
    }
    else if (tsn_zoom_factor > 1)
    {
        // 1us steps
        temp_bandwidth_div = temp_zoom * 1000 * 1000;
    }
    else
    {
        // 100ns steps
        temp_bandwidth_div = temp_zoom * 1000 * 100;
    }

    temp_bandwidth = temp_byte_count/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnBandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));

    temp_bandwidth = tsn_byte_prio_count[0]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio0BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[1]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio1BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[2]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio2BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[3]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio3BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[4]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio4BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[5]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio5BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[6]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio6BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));
    temp_bandwidth = tsn_byte_prio_count[7]/tsn_measurement_count;
    temp_bandwidth = temp_bandwidth*8;
    temp_bandwidth = temp_bandwidth/(1024.0*1024.0);
    temp_bandwidth = temp_bandwidth/temp_bandwidth_div;
    temp_bandwidth = temp_bandwidth*1000000000.0;
    ui->TsnPrio7BandwidthValue->setText(QString::number(temp_bandwidth, 'f', 4));

    temp_string.clear();
    if (temp_frame_count == 0)
    {
        temp_string.append("NA");
    }
    else
    {
        temp_string.append(QString("%1").arg(measure_start.tv_sec, 10, 10, QLatin1Char('0')));
        temp_string.append("s");
    }
    ui->TsnStartTimeSecondsValue->setText(temp_string);

    temp_string.clear();
    if (temp_frame_count == 0)
    {
        temp_string.append("NA");
    }
    else
    {
        temp_string.append(QString("%1").arg(measure_start.tv_usec, 6, 10, QLatin1Char('0')));
        temp_string.append("us");
    }
    ui->TsnStartTimeMicrosecondsValue->setText(temp_string);

    // enable updates
    tsn_frame_chart_view->setUpdatesEnabled(true);

    tsn_frame_chart->show();
}

void Uta_TsnTab::tsn_measure_button_clicked(void)
{
    tsn_clear_persistence = 1;
    tsn_measure_values();
}

void Uta_TsnTab::tsn_save_button_clicked(void)
{

    QDateTime temp_time = QDateTime::currentDateTime();
    QString temp_string = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    temp_string.append("/tsn_chart_");
    temp_string.append(temp_time.toString("dd-MM-yyyy_hh-mm-ss"));

    QString temp_filename =  QFileDialog::getSaveFileName(this, "save chart", temp_string, "PNG(*.png);; TIFF(*.tiff *.tif);; JPEG(*.jpg *.jpeg)");

    ui->TsnFrameChartValue->grab().save(temp_filename);

}

void Uta_TsnTab::tsn_measure_values_timer(void)
{
    tsn_measure_values();
}

void Uta_TsnTab::tsn_auto_refresh_button_clicked(void)
{
    if (ui->TsnAutoRefreshButton->text() == "Start Refresh")
    {
        ui->TsnAutoRefreshButton->setEnabled(false);

        ui->TsnMeasureButton->setEnabled(false);
        ui->TsnEthPortComboBox->setEnabled(false);
        ui->TsnVssTsCheckBox->setEnabled(false);
        ui->TsnPsTsCheckBox->setEnabled(false);
        ui->TsnPersistenceCheckBox->setEnabled(false);
        if (ui->TsnPersistenceCheckBox->isChecked() == true)
        {
            ui->TsnAlignCheckBox->setEnabled(false);
        }

        tsn_clear_persistence = 1;
        tsn_timer->start(10);
        if (ui->TsnPersistenceCheckBox->isChecked() == true)
        {
            tsn_timer->start(10);
        
        }
        else
        {
            tsn_timer->start(100);
        }

        ui->TsnAutoRefreshButton->setText("Stop Refresh");
        ui->TsnAutoRefreshButton->setEnabled(true);
    }
    else
    {
        ui->TsnAutoRefreshButton->setEnabled(false);

        tsn_timer->stop();

        ui->TsnMeasureButton->setEnabled(true);
        ui->TsnEthPortComboBox->setEnabled(true);
        ui->TsnVssTsCheckBox->setEnabled(true);
        ui->TsnPsTsCheckBox->setEnabled(true);
        ui->TsnPersistenceCheckBox->setEnabled(true);
        ui->TsnAlignCheckBox->setEnabled(true);

        ui->TsnAutoRefreshButton->setText("Start Refresh");
        ui->TsnAutoRefreshButton->setEnabled(true);
    }
}

void Uta_TsnTab::tsn_config_prios_button_clicked(void)
{
    ui_prio->setModal(true);
    ui_prio->exec();
}

void Uta_TsnTab::tsn_zoom_in_button_clicked(void)
{
    tsn_clear_persistence = 1;

    switch (tsn_zoom_factor)
    {
    case 10000:
        tsn_zoom_factor = 8000;
        break;

    case 8000:
        tsn_zoom_factor = 6000;
        break;

    case 6000:
        tsn_zoom_factor = 4000;
        break;

    case 4000:
        tsn_zoom_factor = 2000;
        break;

    case 2000:
        tsn_zoom_factor = 1000;
        break;

    case 1000:
        tsn_zoom_factor = 800;
        break;

    case 800:
        tsn_zoom_factor = 600;
        break;

    case 600:
        tsn_zoom_factor = 400;
        break;

    case 400:
        tsn_zoom_factor = 200;
        break;

    case 200:
        tsn_zoom_factor = 100;
        break;

    case 100:
        tsn_zoom_factor = 50;
        break;

    case 50:
        tsn_zoom_factor = 20;
        break;

    case 20:
        tsn_zoom_factor = 10;
        break;

    case 10:
        tsn_zoom_factor = 1;
        ui->TsnZoomInButton->setEnabled(false);
        break;

    case 1:
        tsn_zoom_factor = 1;
        ui->TsnZoomInButton->setEnabled(false);
        break;

    default:
        tsn_zoom_factor = 1;
        ui->TsnZoomInButton->setEnabled(false);
        break;
    }

    ui->TsnZoomOutButton->setEnabled(true); // now we can zoom out for sure

    tsn_frame_chart->removeAxis(tsn_frame_chart_x_axis);
    tsn_frame_chart_x_axis->setLabelFormat("%i");
    tsn_frame_chart_x_axis->setTickCount(11);
    if (tsn_zoom_factor >= 100)
    {
        tsn_frame_chart_x_axis->setTitleText("Time in 10us steps");
        if (tsn_zoom_factor > 1000)
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/1000)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
        else
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/100)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
    }
    else
    {   if (tsn_zoom_factor == 1)
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 100ns steps");
            tsn_frame_chart_x_axis->setMinorTickCount(0);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000);
        }
        else
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 1us steps");
            tsn_frame_chart_x_axis->setMinorTickCount(9);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/10));
        }
    }
    tsn_frame_chart->addAxis(tsn_frame_chart_x_axis, Qt::AlignBottom);

    if ((tsn_zoom_factor == 50) || (tsn_zoom_factor == 1))
    {
         //remove all Series
         foreach (QAbstractSeries *i, tsn_frame_chart->series())
         {
             tsn_frame_chart->removeSeries(i);
         }
    }
    else
    {
        foreach (QAbstractSeries *i, tsn_frame_chart->series())
        {
            i->attachAxis(tsn_frame_chart_x_axis);
        }
    }

    tsn_frame_chart->show();

}
void Uta_TsnTab::tsn_zoom_out_button_clicked(void)
{
    tsn_clear_persistence = 1;

    switch (tsn_zoom_factor)
    {
    case 10000:
        tsn_zoom_factor = 10000;
        ui->TsnZoomOutButton->setEnabled(false);
        break;

    case 8000:
        tsn_zoom_factor = 10000;
        ui->TsnZoomOutButton->setEnabled(false);
        break;

    case 6000:
        tsn_zoom_factor = 8000;
        break;

    case 4000:
        tsn_zoom_factor = 6000;
        break;

    case 2000:
        tsn_zoom_factor = 4000;
        break;

    case 1000:
        tsn_zoom_factor = 2000;
        break;

    case 800:
        tsn_zoom_factor = 1000;
        break;

    case 600:
        tsn_zoom_factor = 800;
        break;

    case 400:
        tsn_zoom_factor = 600;
        break;

    case 200:
        tsn_zoom_factor = 400;
        break;

    case 100:
        tsn_zoom_factor = 200;
        break;

    case 50:
        tsn_zoom_factor = 100;
        break;

    case 20:
        tsn_zoom_factor = 50;
        break;

    case 10:
        tsn_zoom_factor = 20;
        break;

    case 1:
        tsn_zoom_factor = 10;
        break;

    default:
        tsn_zoom_factor = 10000;
        ui->TsnZoomOutButton->setEnabled(false);
        break;
    }

    ui->TsnZoomInButton->setEnabled(true); // now we can zoom out for sure

    tsn_frame_chart->removeAxis(tsn_frame_chart_x_axis);
    tsn_frame_chart_x_axis->setLabelFormat("%i");
    tsn_frame_chart_x_axis->setTickCount(11);
    if (tsn_zoom_factor >= 100)
    {
        tsn_frame_chart_x_axis->setTitleText("Time in 10us steps");
        if (tsn_zoom_factor > 1000)
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/1000)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
        else
        {
            tsn_frame_chart_x_axis->setMinorTickCount((tsn_zoom_factor/100)-1);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/100));
        }
    }
    else
    {   if (tsn_zoom_factor == 1)
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 100ns steps");
            tsn_frame_chart_x_axis->setMinorTickCount(0);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000);
        }
        else
        {
            tsn_frame_chart_x_axis->setTitleText("Time in 1us steps");
            tsn_frame_chart_x_axis->setMinorTickCount(9);
            tsn_frame_chart_x_axis->setMin(0);
            tsn_frame_chart_x_axis->setMax(1000*(tsn_zoom_factor/10));
        }
    }
    tsn_frame_chart->addAxis(tsn_frame_chart_x_axis, Qt::AlignBottom);

    if ((tsn_zoom_factor == 100) || (tsn_zoom_factor == 10))
    {
         //remove all Series
         foreach (QAbstractSeries *i, tsn_frame_chart->series())
         {
             tsn_frame_chart->removeSeries(i);
         }
    }
    else
    {
        foreach (QAbstractSeries *i, tsn_frame_chart->series())
        {
            i->attachAxis(tsn_frame_chart_x_axis);
        }
    }

    tsn_frame_chart->show();

}
