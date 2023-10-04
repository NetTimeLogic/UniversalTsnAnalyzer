#ifndef UCM_TSNPRIOSCREEN_H
#define UCM_TSNPRIOSCREEN_H

#include <QDialog>
#include <ui_Uta_TsnTab.h>

class Uta_TsnTab;

namespace Ui {
class Uta_TsnPrioScreen;
}

class Uta_TsnPrioScreen : public QDialog
{
    Q_OBJECT
private:
    // Tsn tab
    Uta_TsnTab* tsn_tab;
    void tsn_save_config(void);
    void tsn_load_config(void);

public:
    Uta_TsnPrioScreen(Uta_TsnTab* parent);
    ~Uta_TsnPrioScreen();

    Ui::Uta_TsnPrioScreen *ui;


private slots:
    // TSN Prio screen
    void tsn_done_button_clicked(void);
};

#endif // UCM_TSNPRIOSCREEN_H
