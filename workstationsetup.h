#ifndef WORKSTATIONSETUP_H
#define WORKSTATIONSETUP_H

#include "basesubpage.h"
#include "xnode.h"

namespace Ui {
class WorkStationSetup;
}
class QPushButton;

class WorkStationSetup : public BaseSubPage
{
    Q_OBJECT

public:
    explicit WorkStationSetup(QWidget *parent_wgt, QObject *parent = nullptr);
    ~WorkStationSetup();
    void initPage() override;

private:
    void initNode();
    void initTable();
    void refreshTable();
    void selectToolWobj();
    void selectToolLoad();
    void selectPointsNum();
    void getToolWobj(const QString& type,const QString& name);
    void refreshWidget();
    void SetBoat();
    void SetBacket();
    void saveWorksettoRC();
    bool checkCalibrateTool();
    void CaliToolAndWobj(const QString& point_name, const QString& tool_wobj_name);

private:
    Ui::WorkStationSetup *ui;
    ix::Node *m_defboat_mobj;
    ix::Node *m_defbacket_mobj;
    ix::Node *m_boat_mobj;
    ix::Node *m_backet_mobj;
    ix::Node *m_frame_mobj;
    ix::Node *m_boat_points_mobj;
    QList<QPushButton *> pbt_list;
    QList<QPushButton *> pbt_all_list;
    QStringList state_list;
    bool lock_state;
    imol::ModuleObject *m_result_mobj;
};

#endif // WORKSTATIONSETUP_H
