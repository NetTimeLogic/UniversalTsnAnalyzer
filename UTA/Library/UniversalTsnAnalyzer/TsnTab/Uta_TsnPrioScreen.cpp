#include <QTextStream>
#include <Uta_TsnPrioScreen.h>
#include <Uta_TsnTab.h>
#include <ui_Uta_TsnPrioScreen.h>

using namespace std;

#include <sys/time.h>
#include <iostream>
#include <streambuf>
#include <string>

Uta_TsnPrioScreen::Uta_TsnPrioScreen(Uta_TsnTab *parent) : QDialog()
{
    tsn_tab = parent;

    ui = new Ui::Uta_TsnPrioScreen();
    ui->setupUi(this);

    QPalette pal;
    pal.setColor(QPalette::Background, Qt::white);
    setPalette(pal);

    connect(ui->TsnDoneButton, SIGNAL(clicked()), this, SLOT(tsn_done_button_clicked()));
    tsn_load_config();
}

Uta_TsnPrioScreen::~Uta_TsnPrioScreen()
{
    tsn_save_config();
    delete ui;
}

void Uta_TsnPrioScreen::tsn_done_button_clicked(void)
{
    tsn_save_config();
    close();
}

void Uta_TsnPrioScreen::tsn_load_config(void)
{
    unsigned int temp_data;
    int temp_index;
    QFile temp_file("./UtaConfig.cfg");

    if (false == temp_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    while (false == temp_file.atEnd())
    {
        QByteArray temp_line = temp_file.readLine();

        if(true == temp_line.startsWith("VLAN0 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio0ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN1 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio1ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN2 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio2ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN3 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio3ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN4 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio4ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN5 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio5ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN6 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio6ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("VLAN7 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            ui->TsnVlanPrio7ComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("NOVLAN "))
        {
            temp_data = temp_line.mid(7,1).toUInt(nullptr, 10);
            ui->TsnNoVlanComboBox->setCurrentIndex(temp_data);
        }
        else if(true == temp_line.startsWith("ALL "))
        {
            temp_data = temp_line.mid(4,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnAllFramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnAllFramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO0 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio0FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio0FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO1 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio1FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio1FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO2 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio2FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio2FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO3 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio3FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio3FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO4 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio4FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio4FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO5 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio5FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio5FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO6 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio6FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio6FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("PRIO7 "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                ui->TsnPrio7FramesGraphCheckBox->setChecked(false);
            }
            else
            {
                ui->TsnPrio7FramesGraphCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("HW TS "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                tsn_tab->ui->TsnHwTsCheckBox->setChecked(false);
            }
            else
            {
                tsn_tab->ui->TsnHwTsCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("ALIGN "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                tsn_tab->ui->TsnAlignCheckBox->setChecked(false);
            }
            else
            {
                tsn_tab->ui->TsnAlignCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("BYTES "))
        {
            temp_data = temp_line.mid(6,1).toUInt(nullptr, 10);
            if (temp_data == 0)
            {
                tsn_tab->ui->TsnBytesCheckBox->setChecked(false);
            }
            else
            {
                tsn_tab->ui->TsnBytesCheckBox->setChecked(true);
            }
        }
        else if(true == temp_line.startsWith("NET "))
        {
            temp_line.remove(0,4);
            temp_index = -1;
            for (int i = 0; i < tsn_tab->ui->TsnEthPortComboBox->count(); i++)
            {
                if(QString(temp_line).startsWith(tsn_tab->ui->TsnEthPortComboBox->itemText(i)) == true)
                {
                    temp_index = i;
                    break;
                }
            }
            //temp_index = tsn_tab->ui->TsnEthPortComboBox->findText(temp_line);
            if (temp_index >= 0)
            {
                tsn_tab->ui->TsnEthPortComboBox->setCurrentIndex(temp_index);
            }
        }
    }

    temp_file.close();

}

void Uta_TsnPrioScreen::tsn_save_config(void)
{
    QString temp_string;

    temp_string.append("VLAN0 ");
    temp_string.append(QString::number(ui->TsnVlanPrio0ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN1 ");
    temp_string.append(QString::number(ui->TsnVlanPrio1ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN2 ");
    temp_string.append(QString::number(ui->TsnVlanPrio2ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN3 ");
    temp_string.append(QString::number(ui->TsnVlanPrio3ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN4 ");
    temp_string.append(QString::number(ui->TsnVlanPrio4ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN5 ");
    temp_string.append(QString::number(ui->TsnVlanPrio5ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN6 ");
    temp_string.append(QString::number(ui->TsnVlanPrio6ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("VLAN7 ");
    temp_string.append(QString::number(ui->TsnVlanPrio7ComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("NOVLAN ");
    temp_string.append(QString::number(ui->TsnNoVlanComboBox->currentIndex()));
    temp_string.append("\n");
    temp_string.append("NET ");
    temp_string.append(tsn_tab->ui->TsnEthPortComboBox->currentText());
    temp_string.append("\n");

    if (true == ui->TsnAllFramesGraphCheckBox->isChecked())
    {
        temp_string.append("ALL 1\n");
    }
    else
    {
        temp_string.append("ALL 0\n");
    }
    if (true == ui->TsnPrio0FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO0 1\n");
    }
    else
    {
        temp_string.append("PRIO0 0\n");
    }
    if (true == ui->TsnPrio1FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO1 1\n");
    }
    else
    {
        temp_string.append("PRIO1 0\n");
    }
    if (true == ui->TsnPrio2FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO2 1\n");
    }
    else
    {
        temp_string.append("PRIO2 0\n");
    }
    if (true == ui->TsnPrio3FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO3 1\n");
    }
    else
    {
        temp_string.append("PRIO3 0\n");
    }
    if (true == ui->TsnPrio4FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO4 1\n");
    }
    else
    {
        temp_string.append("PRIO4 0\n");
    }
    if (true == ui->TsnPrio5FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO5 1\n");
    }
    else
    {
        temp_string.append("PRIO5 0\n");
    }
    if (true == ui->TsnPrio6FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO6 1\n");
    }
    else
    {
        temp_string.append("PRIO6 0\n");
    }
    if (true == ui->TsnPrio7FramesGraphCheckBox->isChecked())
    {
        temp_string.append("PRIO7 1\n");
    }
    else
    {
        temp_string.append("PRIO7 0\n");
    }
    if (true == tsn_tab->ui->TsnHwTsCheckBox->isChecked())
    {
        temp_string.append("HW TS 1\n");
    }
    else
    {
        temp_string.append("HW TS 0\n");
    }
    if (true == tsn_tab->ui->TsnAlignCheckBox->isChecked())
    {
        temp_string.append("ALIGN 1\n");
    }
    else
    {
        temp_string.append("ALIGN 0\n");
    }
    if (true == tsn_tab->ui->TsnBytesCheckBox->isChecked())
    {
        temp_string.append("BYTES 1\n");
    }
    else
    {
        temp_string.append("BYTES 0\n");
    }

    QFile temp_file("./UtaConfig.cfg");

    if (false == temp_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream temp_stream(&temp_file);

    temp_stream << temp_string << endl;

    temp_file.close();
}
