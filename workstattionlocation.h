#ifndef WORKSTATTIONLOCATION_H
#define WORKSTATTIONLOCATION_H

#include "basesubpage.h"
#include "xnode.h"
#include "dialog/editposedialog.h"

namespace Ui {
class WorkStattionLocation;
}

class WorkStattionLocation : public BaseSubPage
{
    Q_OBJECT

public:
    explicit WorkStattionLocation(QWidget *parent_wgt, QObject *parent = nullptr);
    ~WorkStattionLocation();
    void initPage() override;

private:
    void updatePoint(const int r);
    QString cartTostring(ix::Node *posmobj);
    QString jointTostring(ix::Node *posmobj);
    //void saveToRc();
    void initNode();
    void initTable();
    void refreshTable();
    void saveWorkpointstoRC();
    void ClearPointNode(ix::Node *point_node);

//private slots:
//    void onTableWidgetCellClicked(int row, int column);

private:
    Ui::WorkStattionLocation *ui;
    ix::Node *m_defpoint_mobj;
    ix::Node *m_pointpos_mobj;
    EditPoseDialog *m_editDialog;
    bool isadd;
};

#endif // WORKSTATTIONLOCATION_H
