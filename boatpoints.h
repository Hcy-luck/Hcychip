#ifndef BOATPOINTS_H
#define BOATPOINTS_H

#include "basesubpage.h"
#include "xnode.h"

namespace Ui {
class BoatPoints;
}
class QRadioButton;
class QToolButton;
class QPushButton;
class BoatPoints : public BaseSubPage
{
    Q_OBJECT

public:
    explicit BoatPoints(QWidget *parent_wgt, QObject *parent = nullptr);
    ~BoatPoints();
    void initPage() override;
    void initTable();
    void switchBoat();
    void refrashSlotsPoints(bool is_del_point);
    void refrashPointsFinalValue();
    void saveBoatpointstoRC();
    QList<double> caliSlerp(int start_slot_num,int end_slot_num,ix::Node *start_Node,ix::Node *end_Node,int result_num);
    QString cartTostring(ix::Node *posmobj);

private:
    struct SubBoatCfg
    {
        QString m_title;
        QPushButton* m_tbtn;
        int m_tbtn_num;
        SubBoatCfg(const QString& title) :
            m_title(title),m_tbtn(nullptr),m_tbtn_num(0){}
    };

private:
    Ui::BoatPoints *ui;
    ix::Node *m_pointpos_mobj;
    ix::Node *m_boatset_mobj;
    ix::Node *m_pointdesc_mobj;
    ix::Node *m_record_mobj;
    QList<QRadioButton *> m_slotList;
    QList<SubBoatCfg> m_boatpoint_list;
    QList<QPushButton *> m_boatButtonList;
    QPushButton* m_curr_tb;
    SubBoatCfg* m_curr_cfg;
    int m_curr_tbtn;
    int m_curr_trbtn;
    QString m_curr_trbtn_name;
    QString m_curr_trbtn_en_name;
    ix::Node *m_mobj;
};

#endif // BOATPOINTS_H
