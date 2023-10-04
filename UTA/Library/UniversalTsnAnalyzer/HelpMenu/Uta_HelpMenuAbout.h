#ifndef UTA_HELPMENUABOUT_H
#define UTA_HELPMENUABOUT_H

#include <QWidget>

namespace Ui {
class Uta_HelpMenuAbout;
}

class Uta_HelpMenuAbout : public QWidget
{
    Q_OBJECT
public:
    Uta_HelpMenuAbout();
    ~Uta_HelpMenuAbout();

    Ui::Uta_HelpMenuAbout *ui;

private slots:
    // About screen
    void about_ok_button_clicked(void);
};

#endif // UTA_HELPMENUABOUT_H
