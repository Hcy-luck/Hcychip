#include "workstattionlocation.h"
#include "ui_workstattionlocation.h"
#include "common/entry.h"
#include "../chippage.h"
#include "../xchip_helper.h"
#include "util/synctool.h""
#include "xservice.h"
#include <QLabel>

using namespace xchip;
constexpr int POINT_INDEX  = 0;
constexpr int POINT_NAME   = 1;
constexpr int POINT_TYPE   = 2;
constexpr int POINT_TOOL   = 3;
constexpr int POINT_WOBJ   = 4;
constexpr int POINT_POS    = 5;
constexpr int POINT_DESC   = 6;


IMOL_VER_REG("xchip.workstationlocation", 1)
WorkStattionLocation::WorkStattionLocation(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "workstationlocation",tr("WorkStattionLocation"), parent),
    ui(new Ui::WorkStattionLocation),
    m_defpoint_mobj(defaultNode()->c("workpoints")),
    m_pointpos_mobj(workpointsNode()),
    m_editDialog(new EditPoseDialog(this->wgt()))
{
    ui->setupUi(wgt());
    initNode();
    initTable();
    isadd = true;
    connect(ui->btnPointUpdate, &QPushButton::clicked, this, [=]{
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        if(!helper().execMsgOk("info",tr("Are you sure to update the work station points?"))) return;
        updatePoint(r);
        saveWorkpointstoRC();
    });

    connect(ui->btnMoveto,&QPushButton::clicked,this,[=]{
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        bool is_abs, is_joint = false;
        if(!m_pointpos_mobj->c(r)->c("type")->getInt() == 0){
            is_joint = true;
        }
        moveToPointWithDialog(m_pointpos_mobj->c(r)->c("pos"), is_joint, is_abs, wgt()->parentWidget(),m_pointpos_mobj->c(r)->c("name")->getString());
    });

    connect(ui->btnDelete,&QPushButton::clicked,this,[=]{
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        if(!helper().execMsgOk("info",tr("Are you sure to delete the current points?"))) return;
        ui->tableWidget->removeRow(r);
        m_pointpos_mobj->remove(this,m_pointpos_mobj->c(r));
        helper().throwLog(this, "info", tr("Successfully deleted point!"));
        refreshTable();
        saveWorkpointstoRC();
    });

    connect(ui->btnAdd,&QPushButton::clicked,this,[=]{
        isadd = true;
        if(m_editDialog->isVisible())
            return;

        ix::Node* point_node = m_defpoint_mobj;
        ClearPointNode(point_node);
        m_editDialog->setPointNode(point_node);
        m_editDialog->show();
    });
    connect(m_editDialog,&QDialog::accepted,this,[=]{
        if(isadd){
            ix::Node* point_node = m_defpoint_mobj;
            m_editDialog->setPointNode(point_node);
            m_pointpos_mobj->append(this)->copyFrom(this,point_node);
        }else{
            int r = ui->tableWidget->currentRow();
            ix::Node* point_node = m_pointpos_mobj->c(r);
            m_editDialog->setPointNode(point_node);
            m_pointpos_mobj->c(r)->copyFrom(this,point_node);
        }
        saveWorkpointstoRC();
        //connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(onTableWidgetCellClicked(int,int))

        refreshTable();
    });

    connect(ui->tableWidget->selectionModel(),&QItemSelectionModel::currentRowChanged,this,[=]{
        int currentRow = ui->tableWidget->currentRow();
        if(currentRow == -1){
            xchip::switchToToolset(this,"wobj0","tool0");
        }else{
            xchip::switchToToolset(this,m_pointpos_mobj->c(currentRow)->c("wobj")->getString(),m_pointpos_mobj->c(currentRow)->c("tool")->getString());
        }
    });

    connect(ui->btnEdit,&QPushButton::clicked,this,[=]{
        isadd = false;
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        if(m_editDialog->isVisible())
            return;
        ix::Node* point_node = m_pointpos_mobj->c(r);
        m_editDialog->setPointNode(point_node);
        m_editDialog->show();
    });
}

WorkStattionLocation::~WorkStattionLocation()
{
    delete ui;
}

void WorkStattionLocation::initPage()
{
    refreshTable();
}

void WorkStattionLocation::updatePoint(const int r){
    if(r < 0){
        helper().throwLog(this,"error", tr("please select a point!"));
        return;
    }
//    QTableWidgetItem* nameItem = ui->tableWidget->item(r,POINT_NAME);
//    QString name = nameItem->text();

    //更新settingnode的pos
    ix::Node* point_node = m_pointpos_mobj->c(r)->c("pos");
    xservice()->getAt(this, "sys.posture", point_node);

    QString l = "";
    if(m_pointpos_mobj->c(r)->c("type")->getInt() == 0){
        l = cartTostring(point_node);
    }else if(m_pointpos_mobj->c(r)->c("type")->getInt() == 1){
        l = jointTostring(point_node);
    }

    QTableWidgetItem* poseItem = ui->tableWidget->item(r,POINT_POS);
    if(poseItem) poseItem->setText(l);
}

QString WorkStattionLocation::cartTostring(ix::Node *posmobj){
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    QString cart =  tr("(x:%1,y:%2,z:%3,A:%4,B:%5,C:%6)").arg(QString::number(posmobj->c("trans")->c(0)->getDouble()*transK,'f',2))
                                                                     .arg(QString::number(posmobj->c("trans")->c(1)->getDouble()*transK,'f',2))
                                                                     .arg(QString::number(posmobj->c("trans")->c(2)->getDouble()*transK,'f',2))
                                                                     .arg(QString::number(posmobj->c("rot")->c(0)->getDouble()*transR,'f',2))
                                                                     .arg(QString::number(posmobj->c("rot")->c(1)->getDouble()*transR,'f',2))
                                                                     .arg(QString::number(posmobj->c("rot")->c(2)->getDouble()*transR,'f',2)) ;
    return cart;
}

QString WorkStattionLocation::jointTostring(ix::Node *posmobj){
    double transR = 180/3.1415926;
    QString joint =  tr("[%1 ,%2 ,%3 ,%4 ,%5 ,%6 ,%7]").arg(QString::number(posmobj->c("near")->c(0)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(1)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(2)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(3)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(4)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(5)->getDouble()*transR,'f',3))
                                               .arg(QString::number(posmobj->c("near")->c(6)->getDouble()*transR,'f',3));
    return joint;
}

void WorkStattionLocation::initNode(){

}

void WorkStattionLocation::initTable(){
    ui->tableWidget->verticalHeader()->setHidden(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidget->setColumnWidth(POINT_INDEX, 30);
    ui->tableWidget->setColumnWidth(POINT_NAME, 70);
    ui->tableWidget->setColumnWidth(POINT_TYPE, 70);
    //ui->tableWidget->setColumnWidth(POINT_DESC, 70);
    ui->tableWidget->setColumnWidth(POINT_TOOL, 70);
    ui->tableWidget->setColumnWidth(POINT_WOBJ, 70);
    ui->tableWidget->setColumnWidth(POINT_POS, 460);
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    refreshTable();
}

//void WorkStattionLocation::onTableWidgetCellClicked(int row, int column){
//    int currentRow = ui->tableWidget->currentRow();
//    qDebug()<<"111111111";
//    if(currentRow == -1){
//        xchip::switchToToolset(this,"wobj0","tool0");
//    }else{
//        xchip::switchToToolset(this,m_pointpos_mobj->c(currentRow)->c("wobj")->getString(),m_pointpos_mobj->c(currentRow)->c("tool")->getString());
//    }
//}

void WorkStattionLocation::refreshTable(){
    //ui->tableWidget->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(m_pointpos_mobj->cmobjCount());
    int index=0;
    foreach (ix::Node* point_node, m_pointpos_mobj->cmobjs()){
        ix::Node *point_pos = point_node->c("pos");

        QTableWidgetItem *IndexItem = new QTableWidgetItem(QString::number(index+1));
        IndexItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, POINT_INDEX, IndexItem);

        QTableWidgetItem *NameItem = new QTableWidgetItem(point_node->c("name")->getString());
        NameItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, POINT_NAME, NameItem);

        QTableWidgetItem *DecItem = new QTableWidgetItem(point_node->c("dec")->getString());
        DecItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, POINT_DESC, DecItem);

        if(point_node->c("type")->getInt() == 0){
            QTableWidgetItem *TypeItem = new QTableWidgetItem(tr("Cart"));
            TypeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(index, POINT_TYPE, TypeItem);

            QTableWidgetItem *PosItem = new QTableWidgetItem(cartTostring(point_pos));
            PosItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(index, POINT_POS, PosItem);
        }else if(point_node->c("type")->getInt() == 1){
            QTableWidgetItem *TypeItem = new QTableWidgetItem(tr("Joint"));
            TypeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(index, POINT_TYPE, TypeItem);

            QTableWidgetItem *PosItem = new QTableWidgetItem(jointTostring(point_pos));
            PosItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(index, POINT_POS, PosItem);
        }

        QTableWidgetItem *ToolItem = new QTableWidgetItem(point_node->c("tool")->getString());
        ToolItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, POINT_TOOL, ToolItem);

        QTableWidgetItem *WobjItem = new QTableWidgetItem(point_node->c("wobj")->getString());
        WobjItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, POINT_WOBJ, WobjItem);
        index++;
    }
}

void WorkStattionLocation::saveWorkpointstoRC(){
    //保存到hmi的setting
    QString source_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/workpoints.xml";
    imol::ModuleObject temp_workpoints("workpoints");
    temp_workpoints.append(this,"workpoints")->copyFrom(this, xchip::settingNode()->c("workpoints"));
    m().writeToXmlFile(source_path, &temp_workpoints);

    //rsync到控制器
    QString path = "./internal/" + m("xcore.r.id")->getString() + "/settings/";
    SyncTool::instance()->pushFile(path,"user_settings");
}

void WorkStattionLocation::ClearPointNode(ix::Node *point_node){
    point_node->c("name")->set(this,"");
    point_node->c("dec")->set(this,"");
    point_node->c("type")->set(this,0);
    point_node->c("tool")->set(this,"");
    point_node->c("wobj")->set(this,"");
    point_node->r("pos.trans.#0")->set(this,0);
    point_node->r("pos.trans.#1")->set(this,0);
    point_node->r("pos.trans.#2")->set(this,0);
    point_node->r("pos.rot.#0")->set(this,0);
    point_node->r("pos.rot.#1")->set(this,0);
    point_node->r("pos.rot.#2")->set(this,0);
    point_node->r("pos.near.#0")->set(this,0);
    point_node->r("pos.near.#1")->set(this,0);
    point_node->r("pos.near.#2")->set(this,0);
    point_node->r("pos.near.#3")->set(this,0);
    point_node->r("pos.near.#4")->set(this,0);
    point_node->r("pos.near.#5")->set(this,0);
}
