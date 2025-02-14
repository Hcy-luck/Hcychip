#include "basketpoints.h"
#include "ui_basketpoints.h"
#include "common/entry.h"
#include "../chippage.h"
#include "../xchip_helper.h"
#include "util/synctool.h"
#include "xservice.h"
#include <QLabel>

using namespace xchip;
constexpr int POINT_INDEX  = 0;
constexpr int POINT_NAME   = 1;
constexpr int POINT_DEC    = 2;
constexpr int POINT_TYPE   = 3;
constexpr int POINT_POS    = 4;

IMOL_VER_REG("xchip.basketpoints", 1)
BasketPoints::BasketPoints(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "basketpoints",tr("BasketPoints"), parent),
    ui(new Ui::BasketPoints),
    m_defbasketpoint_mobj(defaultNode()->c("hualanpoints")),
    m_basketset_mobj(workstaNode()->c("hualanset")),
    m_basketpoints_mobj(hualanpointsNode())
{
    ui->setupUi(wgt());
    initTable();

    connect(ui->btnPointUpdate, &QPushButton::clicked, this, [=]{
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        if(!helper().execMsgOk("info",tr("Are you sure to update the basket points?"))) return;
        updatePoint(r,true);
        saveBacketpointstoRC();
        //saveToRc();//更新同时保存，后续考虑用rsysn
    });

    connect(ui->btnMoveto,&QPushButton::clicked,this,[=]{
        int r = ui->tableWidget->currentRow();
        if(r < 0){
            helper().throwLog(this,"error", tr("please select a point!"));
            return;
        }
        bool is_abs, is_joint = false;
        if(m("imol.cfg.common.language")->getString() == "cn"){
            moveToPointWithDialog(m_basketpoints_mobj->c(r)->c("pos"), is_joint, is_abs, wgt()->parentWidget(),m_basketpoints_mobj->c(r)->c("name")->getString());
        }else if(m("imol.cfg.common.language")->getString() == "en"){
            moveToPointWithDialog(m_basketpoints_mobj->c(r)->c("pos"), is_joint, is_abs, wgt()->parentWidget(),m_basketpoints_mobj->c(r)->c("name_en")->getString());
        }

    });
}

BasketPoints::~BasketPoints()
{
    delete ui;
}

void BasketPoints::initNode(){

}


void BasketPoints::initTable(){
    ui->tableWidget->setRowCount(m_defbasketpoint_mobj->cmobjCount());
    ui->tableWidget->verticalHeader()->setHidden(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //ui->tableWidget->setShowGrid(false);

    for(int i =0;i<m_defbasketpoint_mobj->cmobjCount();++i){
        imol::ModuleObject *p = m_defbasketpoint_mobj->c(i);
        QString ptype;
        if(p->c("type")->getString() == "Cart"){
            ptype = tr("Cart");
        }


        QTableWidgetItem *Item1 = new QTableWidgetItem(QString::number(i+1));
        Item1->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, POINT_INDEX, Item1);

        QString name = "s_hualan" + QString::number(i+1);
        QString name_en = "s_basket" + QString::number(i+1);
        if(m("imol.cfg.common.language")->getString() == "cn"){
            QTableWidgetItem *Item2 = new QTableWidgetItem(name);
            Item2->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, POINT_NAME, Item2);
        }else if(m("imol.cfg.common.language")->getString() == "en"){
            QTableWidgetItem *Item2 = new QTableWidgetItem(name_en);
            Item2->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, POINT_NAME, Item2);
        }
        m_basketpoints_mobj->c(i)->c("name")->set(this,name);
        m_basketpoints_mobj->c(i)->c("name_en")->set(this,name_en);


        QTableWidgetItem *Item3 = new QTableWidgetItem(tr("basket") + QString::number(i+1) + tr("pos"));
        Item3->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, POINT_DEC, Item3);
        m_basketpoints_mobj->c(i)->c("dec")->set(this,Item3->text());

        QTableWidgetItem *Item4 = new QTableWidgetItem(ptype);
        Item4->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, POINT_TYPE, Item4);
        if(Item4->text() == "Cart"){
            m_basketpoints_mobj->c(i)->c("type")->set(this,0);
        }

        QTableWidgetItem *Item5 = new QTableWidgetItem();
        Item5->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, POINT_POS, Item5);
    }
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    for(int i=0;i<m_basketset_mobj->cmobjCount();i++){
        if(!m_basketset_mobj->c(i)->c("enable")->getBool()){
            for(int j=0;j<5;j++){
                ui->tableWidget->item(i,j)->setTextColor(QColor(Qt::lightGray));
                ui->tableWidget->item(i,j)->setBackground(QBrush(QColor(Qt::lightGray)));
                ui->tableWidget->item(i,j)->setFlags((Qt::ItemFlags)0);
            }
        }
    }
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 80);
    refreshTable();
}

void BasketPoints::initPage()
{
    xchip::switchToToolset(this,"wobj0","s_tool");
    refreshTable();
}

void BasketPoints::updatePoint(const int r, bool is_update)
{
    if(r < 0){
        helper().throwLog(this,"error", tr("please select a point!"));
        return;
    }

    //QString node_name = "#" + QString::number(r);
    ix::Node* point_node = m_basketpoints_mobj->c(r)->c("pos");
    if(is_update) xservice()->getAt(this, "sys.posture", point_node);

    QString l = cartTostring(point_node);
    QTableWidgetItem* poseItem = ui->tableWidget->item(r,POINT_POS);
    if(poseItem) poseItem->setText(l);
}

QString BasketPoints::cartTostring(ix::Node *posmobj)
{
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    QString cart =  tr("(x: %1 ,y: %2 ,z: %3 ,A: %4 ,B: %5 ,C: %6 )").arg(QString::number(posmobj->c("trans")->c(0)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("trans")->c(1)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("trans")->c(2)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(0)->getDouble()*transR,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(1)->getDouble()*transR,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(2)->getDouble()*transR,'f',3)) ;
    return cart;
}

void BasketPoints::refreshTable(){
    for(int i=0;i<m_basketpoints_mobj->cmobjCount();i++){
        updatePoint(i, false);
    }
}

void BasketPoints::saveBacketpointstoRC(){
    //保存到hmi的setting
    QString source_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/backetpoints.xml";
    imol::ModuleObject temp_backetpoints("backetpoints");
    temp_backetpoints.append(this,"hualanpoints")->copyFrom(this, xchip::settingNode()->c("hualanpoints"));
    m().writeToXmlFile(source_path, &temp_backetpoints);

    //rsync到控制器
    QString path = "./internal/" + m("xcore.r.id")->getString() + "/settings/";
    SyncTool::instance()->pushFile(path,"user_settings");
}

