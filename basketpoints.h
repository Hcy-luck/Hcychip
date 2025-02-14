#ifndef BASKETPOINTS_H
#define BASKETPOINTS_H

#include "basesubpage.h"
#include "xnode.h"

namespace Ui {
class BasketPoints;
}

class BasketPoints : public BaseSubPage
{
    Q_OBJECT

public:
    explicit BasketPoints(QWidget *parent_wgt, QObject *parent = nullptr);
    ~BasketPoints();
    void initPage() override;

private:
    void updatePoint(const int r, bool is_update);
    void initNode();
    void initTable();
    void refreshTable();
    void saveBacketpointstoRC();
    QString cartTostring(ix::Node *posmobj);

private:
    Ui::BasketPoints *ui;
    ix::Node *m_defbasketpoint_mobj;
    ix::Node *m_basketset_mobj;
    ix::Node *m_basketpoints_mobj;
};

#endif // BASKETPOINTS_H
