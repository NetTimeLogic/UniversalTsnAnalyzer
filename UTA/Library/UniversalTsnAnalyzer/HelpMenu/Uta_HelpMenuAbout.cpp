#include <Uta_HelpMenuAbout.h>
#include <ui_Uta_HelpMenuAbout.h>
#include <Uta_UniversalTsnAnalyzer.h>

Uta_HelpMenuAbout::Uta_HelpMenuAbout() : QWidget()
{
    QString temp_version;

    ui = new Ui::Uta_HelpMenuAbout();
    ui->setupUi(this);

    QPalette pal;
    pal.setColor(QPalette::Background, Qt::white);
    setPalette(pal);

    temp_version.append("Version: ");
    temp_version.append(Uta_Version_Nr);
    temp_version.append(" Build time: ");
    temp_version.append(__TIME__);
    temp_version.append(" ");
    temp_version.append(__DATE__);
    ui->AboutVersionValue->setText(temp_version);

    connect(ui->AboutOkButton, SIGNAL(clicked()), this, SLOT(about_ok_button_clicked()));
}

Uta_HelpMenuAbout::~Uta_HelpMenuAbout()
{
    delete ui;
}

void Uta_HelpMenuAbout::about_ok_button_clicked(void)
{
    close();
}
