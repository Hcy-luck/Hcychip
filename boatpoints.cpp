#include "boatpoints.h"
#include "ui_boatpoints.h"
#include "common/entry.h"
#include "../chippage.h"
#include "../xchip_helper.h"
#include "util/synctool.h""
#include "xservice.h"
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QtMath>
#include <QListView>

using namespace xchip;

IMOL_VER_REG("xchip.boatpoints", 4)
BoatPoints::BoatPoints(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "boatpoints",tr("BoatPoints"), parent),
    ui(new Ui::BoatPoints),
    m_pointpos_mobj(zhoupointsNode()),
    m_boatset_mobj(workstaNode()->c("zhouset")),
    m_pointdesc_mobj(defaultNode()->c("zhoupoints")->c("zhou1_z_qu")),
    //m_record_mobj(defaultNode()->c("record_point")),
    m_record_mobj(new ix::Node("")),
    m_mobj(new ix::Node("")),
    m_curr_tb(nullptr),
    m_curr_cfg(nullptr)
{
    ui->setupUi(wgt());

//    ui->wgtNav->setGeometry(0,0,wgt()->width(),wgt()->height()/10);
//    ui->wgtNav->show();

    m_curr_tbtn = 0;
    m_record_mobj = xchip::defaultNode()->c("record_point");
    ui->lblpoint1->setEnabled(false);
    ui->lblpoint2->setEnabled(false);
    ui->btnRecord1->setProperty("func","operation");
    ui->btnRecord2->setProperty("func","operation");
    ui->btnCali->setProperty("func","operation");
    ui->btnConfirm->setProperty("func","operation");
    ui->btnGetPos->setProperty("func","operation");
    ui->btnCaliValue->setProperty("func","operation");
    ui->btnMoveTo->setProperty("func","operation");

    m_mobj = xchip::defaultNode()->c("cali_point");
    for(int i =0;i<4;i++){
        if(m_mobj->c(i)->c("reference")->cmobjCount() < m_boatset_mobj->c(0)->c("slots")->getInt()){
            for(int j = m_mobj->c(i)->c("reference")->cmobjCount();j<m_boatset_mobj->c(0)->c("slots")->getInt();j++){
                m_mobj->c(i)->c("reference")->append(this)->copyFrom(this,defaultNode()->c("point_pos"));
            }
        }
        if(m_mobj->c(i+4)->c("reference")->cmobjCount()< m_boatset_mobj->c(1)->c("slots")->getInt()){
            for(int j = m_mobj->c(i+4)->c("reference")->cmobjCount();j<m_boatset_mobj->c(1)->c("slots")->getInt();j++){
                m_mobj->c(i+4)->c("reference")->append(this)->copyFrom(this,defaultNode()->c("point_pos"));
            }
        }
    }
    for(int i =0;i<4;i++){
        if(m_mobj->c(i)->c("comp")->cmobjCount() < m_boatset_mobj->c(0)->c("slots")->getInt()){
            for(int j = m_mobj->c(i)->c("comp")->cmobjCount();j<m_boatset_mobj->c(0)->c("slots")->getInt();j++){
                m_mobj->c(i)->c("comp")->append(this)->copyFrom(this,defaultNode()->c("point_pos"));
            }
        }
        if(m_mobj->c(i+4)->c("comp")->cmobjCount()< m_boatset_mobj->c(1)->c("slots")->getInt()){
            for(int j = m_mobj->c(i+4)->c("comp")->cmobjCount();j<m_boatset_mobj->c(1)->c("slots")->getInt();j++){
                m_mobj->c(i+4)->c("comp")->append(this)->copyFrom(this,defaultNode()->c("point_pos"));
            }
        }
    }

    if(xchip::settingNode()->c("two_point_cal_enable")->getBool()){
        ui->grbCalSlot->setHidden(false);
        ui->groupBox_4->setHidden(true);
        ui->rbtnenable->setChecked(true);
    }else{
        ui->grbCalSlot->setHidden(true);
        ui->groupBox_4->setHidden(false);
        ui->rbtnenable_2->setChecked(false);
    }

    ui->spn_re_x->setEnabled(false);
    ui->spn_re_y->setEnabled(false);
    ui->spn_re_z->setEnabled(false);
    ui->spn_re_a->setEnabled(false);
    ui->spn_re_b->setEnabled(false);
    ui->spn_re_c->setEnabled(false);

    m_boatpoint_list << SubBoatCfg(tr("Boat1_P_Pick")) << SubBoatCfg(tr("Boat1_P_Place")) << SubBoatCfg(tr("Boat1_N_Pick")) << SubBoatCfg(tr("Boat1_N_Place"))
                        << SubBoatCfg(tr("Boat2_P_Pick")) << SubBoatCfg(tr("Boat2_P_Place")) << SubBoatCfg(tr("Boat2_N_Pick")) << SubBoatCfg(tr("Boat2_N_Place"));
    initTable();

    connect(ui->rbtnenable,&QRadioButton::clicked,this,[=]{
        if(!ui->rbtnenable->isChecked()){
            ui->grbCalSlot->setHidden(true);
            ui->groupBox_4->setHidden(false);
            ui->rbtnenable_2->setChecked(false);
            xchip::settingNode()->c("two_point_cal_enable")->set(this,false);
        }
    });
    connect(ui->rbtnenable_2,&QRadioButton::clicked,this,[=]{
        if(ui->rbtnenable_2->isChecked()){
            ui->grbCalSlot->setHidden(false);
            ui->groupBox_4->setHidden(true);
            ui->rbtnenable->setChecked(true);
            xchip::settingNode()->c("two_point_cal_enable")->set(this,true);
        }
    });

    connect(ui->btnRecord1,&QPushButton::clicked,this,[=]{
        ix::Node* point_node = m_record_mobj->c("record_point1")->c("pos");
        xservice()->getAt(this, "sys.posture", point_node);

        m_record_mobj->c("record_point1")->c("slots_num")->set(this,ui->spinBox->value());
        m_record_mobj->c("record_point1")->c("pos")->copyFrom(this,point_node);

        QString l = cartTostring(point_node);
        ui->lblpoint1->setText(l);
    });

    connect(ui->btnRecord2,&QPushButton::clicked,this,[=]{
        ix::Node* point_node = m_record_mobj->c("record_point2")->c("pos");
        xservice()->getAt(this, "sys.posture", point_node);

        m_record_mobj->c("record_point2")->c("slots_num")->set(this,ui->spinBox_2->value());
        m_record_mobj->c("record_point2")->c("pos")->copyFrom(this,point_node);
        QString l = cartTostring(point_node);
        ui->lblpoint2->setText(l);
    });

    connect(ui->btnCali,&QPushButton::clicked,this,[=]{
        //计算算出该舟所有槽点位的基准值，并清空该舟的补偿值
        int m_curr_boat = m_curr_tbtn/4;
        if(ui->spinBox->value() < 1 || ui->spinBox->value() > m_boatset_mobj->c(m_curr_boat)->c("slots")->getInt()){
            helper().throwLog(this,"error", tr("Please enter the correct slot number!"));
            return;
        }
        if(ui->spinBox_2->value() < 1 || ui->spinBox_2->value() > m_boatset_mobj->c(m_curr_boat)->c("slots")->getInt()){
            helper().throwLog(this,"error", tr("Please enter the correct slot number!"));
            return;
        }
        if(ui->spinBox->value() == ui->spinBox_2->value()){
            helper().throwLog(this,"error", tr("Can not enter two identical slot numbers!"));
            return;
        }
        if(ui->lblpoint1->text() == "" || ui->lblpoint2->text() == ""){
            helper().throwLog(this,"error", tr("Record point cannot be empty!"));
            return;
        }
        m_mobj->c(m_curr_tbtn)->c("reference")->clear(this);
        m_mobj->c(m_curr_tbtn)->c("comp")->clear(this);
        m_mobj->c(m_curr_tbtn)->c("is_cali")->set(this,true);

        //求槽之间的点位差值
        double x_record1 = m_record_mobj->c("record_point1")->r("pos.trans.#0")->getDouble();
        double x_record2 = m_record_mobj->c("record_point2")->r("pos.trans.#0")->getDouble();
        double y_record1 = m_record_mobj->c("record_point1")->r("pos.trans.#1")->getDouble();
        double y_record2 = m_record_mobj->c("record_point2")->r("pos.trans.#1")->getDouble();
        double z_record1 = m_record_mobj->c("record_point1")->r("pos.trans.#2")->getDouble();
        double z_record2 = m_record_mobj->c("record_point2")->r("pos.trans.#2")->getDouble();
//        double a_record1 = m_record_mobj->c("record_point1")->r("pos.rot.#0")->getDouble();
//        double a_record2 = m_record_mobj->c("record_point2")->r("pos.rot.#0")->getDouble();
//        double b_record1 = m_record_mobj->c("record_point1")->r("pos.rot.#1")->getDouble();
//        double b_record2 = m_record_mobj->c("record_point2")->r("pos.rot.#1")->getDouble();
//        double c_record1 = m_record_mobj->c("record_point1")->r("pos.rot.#2")->getDouble();
//        double c_record2 = m_record_mobj->c("record_point2")->r("pos.rot.#2")->getDouble();
        int slots_diff = ui->spinBox_2->value() - ui->spinBox->value();
        double x_diff = (x_record2 - x_record1) / slots_diff;
        double y_diff = (y_record2 - y_record1) / slots_diff;
        double z_diff = (z_record2 - z_record1) / slots_diff;
//        double a_diff = (a_record2 - a_record1) / slots_diff;
//        double b_diff = (b_record2 - b_record1) / slots_diff;
//        double c_diff = (c_record2 - c_record1) / slots_diff;
        //计算每个槽点位的基准
        for(int i = 0 ; i<m_boatset_mobj->c(m_curr_boat)->c("slots")->getInt();i++){
            ix::Node *slot_point_reference = new ix::Node("");
            slot_point_reference->copyFrom(this,defaultNode()->c("point_pos"));
            ix::Node *slot_point_comp = new ix::Node("");
            slot_point_comp->copyFrom(this,defaultNode()->c("point_pos"));
            int curr_slots = ui->spinBox->value() - i - 1;
            slot_point_reference->c("trans")->c("#0")->set(this,x_record1 - (x_diff * curr_slots));
            slot_point_reference->r("trans.#1")->set(this,y_record1 - (y_diff * curr_slots));
            slot_point_reference->r("trans.#2")->set(this,z_record1 - (z_diff * curr_slots));
            QList<double> slot_ori;
            slot_ori = caliSlerp(ui->spinBox->value(),ui->spinBox_2->value(),m_record_mobj->c("record_point1")->r("pos.rot"),
                      m_record_mobj->c("record_point2")->r("pos.rot"),i+1);

            slot_point_reference->r("rot.#0")->set(this,slot_ori.at(0));
            slot_point_reference->r("rot.#1")->set(this,slot_ori.at(1));
            slot_point_reference->r("rot.#2")->set(this,slot_ori.at(2));
            slot_point_comp->r("trans.#0")->set(this,0);
            slot_point_comp->r("trans.#1")->set(this,0);
            slot_point_comp->r("trans.#2")->set(this,0);
            slot_point_comp->r("rot.#0")->set(this,0);
            slot_point_comp->r("rot.#1")->set(this,0);
            slot_point_comp->r("rot.#2")->set(this,0);
            m_mobj->c(m_curr_tbtn)->c("reference")->append(this)->copyFrom(this,slot_point_reference);
            m_mobj->c(m_curr_tbtn)->c("comp")->append(this)->copyFrom(this,slot_point_comp);
        }

        refrashSlotsPoints(false);

        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(0)->set(this,ui->spn_re_x->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(1)->set(this,ui->spn_re_y->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(2)->set(this,ui->spn_re_z->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(0)->set(this,ui->spn_re_a->value()/transR);
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(1)->set(this,ui->spn_re_b->value()/transR);
        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(2)->set(this,ui->spn_re_c->value()/transR);
        helper().throwLog(this,"info", tr("Slot postion calculation successful!"));
    });

    connect(ui->btnGetPos,&QPushButton::clicked,this,[=]{
        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        ix::Node *point_node = new ix::Node("");
        xservice()->getAt(this, "sys.posture", point_node);
        ui->spn_re_x->setValue(point_node->c("trans")->c(0)->getDouble()*transK);
        ui->spn_re_y->setValue(point_node->c("trans")->c(1)->getDouble()*transK);
        ui->spn_re_z->setValue(point_node->c("trans")->c(2)->getDouble()*transK);
        ui->spn_re_a->setValue(point_node->c("rot")->c(0)->getDouble()*transR);
        ui->spn_re_b->setValue(point_node->c("rot")->c(1)->getDouble()*transR);
        ui->spn_re_c->setValue(point_node->c("rot")->c(2)->getDouble()*transR);

        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->copyFrom(this,point_node);

//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(0)->set(this,ui->spn_re_x->value()/transK);
//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(1)->set(this,ui->spn_re_y->value()/transK);
//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(2)->set(this,ui->spn_re_z->value()/transK);
//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(0)->set(this,ui->spn_re_a->value()/transR);
//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(1)->set(this,ui->spn_re_b->value()/transR);
//        m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(2)->set(this,ui->spn_re_c->value()/transR);
    });
    connect(ui->btnCaliValue,&QPushButton::clicked,this,[=]{
        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(0)->set(this,ui->spn_co_x->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(1)->set(this,ui->spn_co_y->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(2)->set(this,ui->spn_co_z->value()/transK);
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(0)->set(this,ui->spn_co_a->value()/transR);
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(1)->set(this,ui->spn_co_b->value()/transR);
        m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(2)->set(this,ui->spn_co_c->value()/transR);

        ui->spn_value_x->setValue(ui->spn_re_x->value() + ui->spn_co_x->value());
        ui->spn_value_y->setValue(ui->spn_re_y->value() + ui->spn_co_y->value());
        ui->spn_value_z->setValue(ui->spn_re_z->value() + ui->spn_co_z->value());
        ui->spn_value_a->setValue(ui->spn_re_a->value() + ui->spn_co_a->value());
        ui->spn_value_b->setValue(ui->spn_re_b->value() + ui->spn_co_b->value());
        ui->spn_value_c->setValue(ui->spn_re_c->value() + ui->spn_co_c->value());
    });
    connect(ui->btnConfirm,&QPushButton::clicked,this,[=]{
        if(!helper().execMsgOk("info",tr("Are you sure to save the boat point?"))){
            refrashPointsFinalValue();
            return;
        }
        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        ix::Node* point_node = m_pointdesc_mobj;
        point_node->c("pos")->r("trans.#0")->set(this,ui->spn_value_x->value()/transK);
        point_node->c("pos")->r("trans.#1")->set(this,ui->spn_value_y->value()/transK);
        point_node->c("pos")->r("trans.#2")->set(this,ui->spn_value_z->value()/transK);
        point_node->c("pos")->r("rot.#0")->set(this,ui->spn_value_a->value()/transR);
        point_node->c("pos")->r("rot.#1")->set(this,ui->spn_value_b->value()/transR);
        point_node->c("pos")->r("rot.#2")->set(this,ui->spn_value_c->value()/transR);
        point_node->c("name")->set(this,m_curr_trbtn_name);
        point_node->c("name_en")->set(this,m_curr_trbtn_en_name);
        point_node->c("type")->set(this,0);
        point_node->c("tool")->set(this,"s_tool");
        if(m_curr_tbtn/4 == 0){
            point_node->c("wobj")->set(this,"s_boat1");
        }else{
            point_node->c("wobj")->set(this,"s_boat2");
        }
        for(int i=0;i<m_pointpos_mobj->cmobjCount();i++){
            if(m_pointpos_mobj->c(i)->c("name")->getString() == m_curr_trbtn_name){
                m_pointpos_mobj->c(i)->copyFrom(this,point_node);
                helper().throwLog(this,"info",tr("Successfully saved the boat location"));
                saveBoatpointstoRC();
                return;
            }
        }
        m_pointpos_mobj->append(this)->copyFrom(this,point_node);
        helper().throwLog(this,"info",tr("Successfully saved the boat location"));
        saveBoatpointstoRC();
    });

    connect(ui->btnMoveTo, &QPushButton::clicked, this, [=]{
        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        ix::Node *move_point = new ix::Node("");
        move_point->copyFrom(this,defaultNode()->c("point_pos"));
        move_point->r("trans.#0")->set(this,ui->spn_value_x->value()/transK);
        move_point->r("trans.#1")->set(this,ui->spn_value_y->value()/transK);
        move_point->r("trans.#2")->set(this,ui->spn_value_z->value()/transK);
        move_point->r("rot.#0")->set(this,ui->spn_value_a->value()/transR);
        move_point->r("rot.#1")->set(this,ui->spn_value_b->value()/transR);
        move_point->r("rot.#2")->set(this,ui->spn_value_c->value()/transR);
        bool is_joint = false;
        bool is_abs = false;

        if(m("imol.cfg.common.language")->getString() == "cn"){
            moveToPointWithDialog(move_point, is_joint, is_abs, wgt()->parentWidget(),m_curr_trbtn_name);
        }else if(m("imol.cfg.common.language")->getString() == "en"){
            moveToPointWithDialog(move_point, is_joint, is_abs, wgt()->parentWidget(),m_curr_trbtn_en_name);
        }


//        foreach(auto point_node,m_pointpos_mobj->cmobjs()){
//            if(point_node->c("name")->getString() == m_curr_trbtn_name){
//                bool is_joint = false;
//                bool is_abs = false;
//                moveToPointWithDialog(point_node->c("pos"), is_joint, is_abs, wgt()->parentWidget());
//            }
//        }
    });

    ui->rbtnenable->setChecked(true);
}

BoatPoints::~BoatPoints()
{
    delete ui;
}

void BoatPoints::initPage()
{
    refrashSlotsPoints(true);
}

void BoatPoints::initTable(){
    for(int i=0;i<m_boatset_mobj->cmobjCount();i++){
        for(int j =0;j<4;j++){
            QPushButton* tbtn = new QPushButton(ui->wgtNav);
            QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(0);
            sizePolicy.setHeightForWidth(tbtn->sizePolicy().hasHeightForWidth());
            tbtn->setSizePolicy(sizePolicy);
            tbtn->setText(m_boatpoint_list.at(i *4 + j).m_title);
            tbtn->setMinimumSize(QSize(0, 35));
            tbtn->setCheckable(true);
            tbtn->setChecked(false);
            tbtn->setProperty("func","chb_operation");
            ui->wgtNav->layout()->addWidget(tbtn);


            tbtn->setEnabled(m_boatset_mobj->c(i)->c("enable")->getBool());
            connect(tbtn, &QPushButton::clicked, this, &BoatPoints::switchBoat);
            m_boatpoint_list[i*4+j].m_tbtn = tbtn;
            m_boatpoint_list[i*4+j].m_tbtn_num = i*4+j;
        }
    }
    m_boatpoint_list[0].m_tbtn->click();
    refrashPointsFinalValue();
}

void BoatPoints::switchBoat(){
    //radio按钮
    while(ui->wgtNav_2->layout()->count()){
        QWidget *pWidget = ui->wgtNav_2->layout()->itemAt(0)->widget();
        pWidget->setParent(NULL);
        ui->wgtNav_2->layout()->removeWidget(pWidget);
        delete pWidget;
    }
    auto tb = qobject_cast<QPushButton *>(sender());
    if(!tb) return;
    //修复点击已选中的button时，页面没有radiobutton的问题
//    if(tb == m_curr_tb){
//        tb->setChecked(true);
//        return;
//    }
    for(int i=0; i<m_boatpoint_list.count(); i++){
        if(tb != m_boatpoint_list.at(i).m_tbtn){
            if(m_boatset_mobj->c(i/4)->c("enable")->getBool()){
                m_boatpoint_list.at(i).m_tbtn->setChecked(false);
            }

            m_boatpoint_list.at(i).m_tbtn->setChecked(false);
        }else{
            m_boatpoint_list.at(i).m_tbtn->setChecked(true);
            int slot_num = m_boatset_mobj->c(m_boatpoint_list.at(i).m_tbtn_num/4)->c("slots")->getInt();
            if(slot_num !=0){
                ui->spinBox->setRange(1,slot_num);
                ui->spinBox_2->setRange(1,slot_num);
            }
            for(int j =0;j<slot_num;j++){
                QRadioButton* trbtn = new QRadioButton(ui->wgtNav_2);
                QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
                sizePolicy.setHorizontalStretch(0);
                sizePolicy.setVerticalStretch(0);
                sizePolicy.setHeightForWidth(trbtn->sizePolicy().hasHeightForWidth());
                trbtn->setSizePolicy(sizePolicy);

                QString slot_name,slot_en_name = "";
                if(m_boatpoint_list.at(i).m_tbtn_num == 0 || m_boatpoint_list.at(i).m_tbtn_num == 4){
                    slot_name = "s_zhou" + QString::number(i/4+1) + "_z_qu_c" + QString::number(j+1);
                    slot_en_name = "s_boat" + QString::number(i/4+1) + "_p_pick_s" + QString::number(j+1);
                }else if(m_boatpoint_list.at(i).m_tbtn_num == 1 || m_boatpoint_list.at(i).m_tbtn_num == 5){
                    slot_name = "s_zhou" + QString::number(i/4+1) + "_z_fang_c" + QString::number(j+1);
                    slot_en_name = "s_boat" + QString::number(i/4+1) + "_p_place_s" + QString::number(j+1);
                }else if(m_boatpoint_list.at(i).m_tbtn_num == 2 || m_boatpoint_list.at(i).m_tbtn_num == 6){
                    slot_name = "s_zhou" + QString::number(i/4+1) + "_f_qu_c" + QString::number(j+1);
                    slot_en_name = "s_boat" + QString::number(i/4+1) + "_n_pick_s" + QString::number(j+1);
                }else if(m_boatpoint_list.at(i).m_tbtn_num == 3 || m_boatpoint_list.at(i).m_tbtn_num == 7){
                    slot_name = "s_zhou" + QString::number(i/4+1) + "_f_fang_c" + QString::number(j+1);
                    slot_en_name = "s_boat" + QString::number(i/4+1) + "_n_place_s" + QString::number(j+1);
                }
                m_boatpoint_list.at(i).m_tbtn->setChecked(true);

                m_curr_tbtn = i;

                if(m("imol.cfg.common.language")->getString() == "cn"){
                     trbtn->setText(slot_name);
                }else if(m("imol.cfg.common.language")->getString() == "en"){
                     trbtn->setText(slot_en_name);
                }

                trbtn->setMinimumSize(QSize(0, 45));
                ui->wgtNav_2->layout()->addWidget(trbtn);
                if(slot_num == 1){
                    QButtonGroup *group1 = new QButtonGroup;
                    group1->addButton(trbtn);
                }
                connect(trbtn, &QRadioButton::clicked, this, [=]{
                    //读setting里的点位
                    QString slot_text = tr("Slot") + QString::number(j + 1) + tr("Pos");
                    ui->label_12->setText(slot_text + tr("Reference"));
                    ui->label_26->setText(slot_text + tr("Compensate"));
                    ui->label_33->setText(slot_text + tr("Final Value"));
                    m_curr_trbtn = j;
                    m_curr_trbtn_name = slot_name;
                    m_curr_trbtn_en_name = slot_en_name;
                    refrashSlotsPoints(true);
                    //不同槽点位进行工具工件切换
                    if(i/4 == 0){
                        xchip::switchToToolset(this,"s_boat1","s_tool");
                    }
                    else if(i/4 ==1){
                        xchip::switchToToolset(this,"s_boat2","s_tool");
                    }
                });
                if(j == 0){
                    trbtn->click();
                }
            }

        }
    }
    m_curr_tb = tb;
    //
}

QString BoatPoints::cartTostring(ix::Node *posmobj)
{
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    QString cart =  tr("(x: %1,y: %2,z: %3 ,A: %4 ,B: %5 ,C: %6 )").arg(QString::number(posmobj->c("trans")->c(0)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("trans")->c(1)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("trans")->c(2)->getDouble()*transK,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(0)->getDouble()*transR,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(1)->getDouble()*transR,'f',3))
                                                                     .arg(QString::number(posmobj->c("rot")->c(2)->getDouble()*transR,'f',3)) ;
    return cart;
}

void BoatPoints::refrashSlotsPoints(bool is_del_point){
    //刷新最终值
    refrashPointsFinalValue();

//    if(!m_mobj->c(m_curr_tbtn)->c("is_cali")->getBool()){
//        //未进行计算的舟，基准和补偿应该清0
//        ui->spn_re_x->setValue(0);
//        ui->spn_re_y->setValue(0);
//        ui->spn_re_z->setValue(0);
//        ui->spn_re_a->setValue(0);
//        ui->spn_re_b->setValue(0);
//        ui->spn_re_c->setValue(0);

//        ui->spn_co_x->setValue(0);
//        ui->spn_co_y->setValue(0);
//        ui->spn_co_z->setValue(0);
//        ui->spn_co_a->setValue(0);
//        ui->spn_co_b->setValue(0);
//        ui->spn_co_c->setValue(0);
//        return;
//    }
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    ui->spn_re_x->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(0)->getDouble() * transK);
    ui->spn_re_y->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(1)->getDouble() * transK);
    ui->spn_re_z->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("trans")->c(2)->getDouble() * transK);
    ui->spn_re_a->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(0)->getDouble() * transR);
    ui->spn_re_b->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(1)->getDouble() * transR);
    ui->spn_re_c->setValue(m_mobj->c(m_curr_tbtn)->c("reference")->c(m_curr_trbtn)->c("rot")->c(2)->getDouble() * transR);

    ui->spn_co_x->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(0)->getDouble() * transK);
    ui->spn_co_y->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(1)->getDouble() * transK);
    ui->spn_co_z->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("trans")->c(2)->getDouble() * transK);
    ui->spn_co_a->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(0)->getDouble() * transR);
    ui->spn_co_b->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(1)->getDouble() * transR);
    ui->spn_co_c->setValue(m_mobj->c(m_curr_tbtn)->c("comp")->c(m_curr_trbtn)->c("rot")->c(2)->getDouble() * transR);

    if(is_del_point){
        ui->lblpoint1->setText("");
        ui->lblpoint2->setText("");
    }
    ui->spinBox->setValue(m_record_mobj->c("record_point1")->c("slots_num")->getInt(1));
    ui->spinBox_2->setValue(m_record_mobj->c("record_point2")->c("slots_num")->getInt(1));

}



void BoatPoints::refrashPointsFinalValue(){
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    for(int i=0;i<m_pointpos_mobj->cmobjCount();i++){
//        qDebug()<<"node name"<<m_pointpos_mobj->c(i)->c("name")->getString();
//        qDebug()<<"point name"<<m_curr_trbtn_name;
        if(m_pointpos_mobj->c(i)->c("name")->getString() == m_curr_trbtn_name){
            ui->spn_value_x->setValue(m_pointpos_mobj->c(i)->c("pos")->c("trans")->c(0)->getDouble() * transK);
            ui->spn_value_y->setValue(m_pointpos_mobj->c(i)->c("pos")->c("trans")->c(1)->getDouble() * transK);
            ui->spn_value_z->setValue(m_pointpos_mobj->c(i)->c("pos")->c("trans")->c(2)->getDouble() * transK);
            ui->spn_value_a->setValue(m_pointpos_mobj->c(i)->c("pos")->c("rot")->c(0)->getDouble() * transR);
            ui->spn_value_b->setValue(m_pointpos_mobj->c(i)->c("pos")->c("rot")->c(1)->getDouble() * transR);
            ui->spn_value_c->setValue(m_pointpos_mobj->c(i)->c("pos")->c("rot")->c(2)->getDouble() * transR);
            return;
        }
    }
    ui->spn_value_x->setValue(0);
    ui->spn_value_y->setValue(0);
    ui->spn_value_z->setValue(0);
    ui->spn_value_a->setValue(0);
    ui->spn_value_b->setValue(0);
    ui->spn_value_c->setValue(0);
}

void BoatPoints::saveBoatpointstoRC(){
    //保存到hmi的setting
    QString source_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/boatpoints.xml";
    imol::ModuleObject temp_boatpoints("boatpoints");
    temp_boatpoints.append(this,"zhoupoints")->copyFrom(this, xchip::settingNode()->c("zhoupoints"));
    m().writeToXmlFile(source_path, &temp_boatpoints);

    //rsync到控制器
    QString path = "./internal/" + m("xcore.r.id")->getString() + "/settings/";
    SyncTool::instance()->pushFile(path,"user_settings");
}

QList<double> BoatPoints::caliSlerp(int start_slot_num,int end_slot_num,ix::Node *start_Node,ix::Node *end_Node,int result_num){
    double transR = 180.0/3.1415926;
    double t = (result_num - start_slot_num)/(end_slot_num - start_slot_num);
//    qDebug()<<"t:"<<t;
//    qDebug()<<"start ori:"<<start_Node->c(0)->getDouble() *transR << start_Node->c(1)->getDouble() *transR << start_Node->c(2)->getDouble()*transR ;
//    qDebug()<<"end ori:"<<end_Node->c(0)->getDouble() *transR << end_Node->c(1)->getDouble() *transR << end_Node->c(2)->getDouble()*transR ;
    ix::Node quaternionObjstart("quaternionObj1");
    ix::Node quaternionObjend("quaternionObj2");
    ix::calc::rot2Quaternion(start_Node,&quaternionObjstart);
    ix::calc::rot2Quaternion(end_Node,&quaternionObjend);

    double cosw = 0;
    for(int i=0;i<4;i++){
//        qDebug()<<"start node:"<<i<<":"<<quaternionObjstart.c(i)->getDouble();
//        qDebug()<<"end node:"<<i<<":"<<quaternionObjend.c(i)->getDouble();
        cosw += quaternionObjstart.c(i)->getDouble() * quaternionObjend.c(i)->getDouble();
    }
    if(cosw < 0.0){
        //endOrient = -end;
        for(int i =0;i<4;i++){
            quaternionObjend.c(i)->set(this,-quaternionObjend.c(i)->getDouble());
        }
        cosw = -cosw;
    }

    double k0,k1;

    if(cosw > 0.999999999){
        k0 = 1.0 - t;
        k1 = t;
    }else{
        double sinw = sqrt(1.0 - cosw * cosw);
        double w = acos(cosw);
        double invSinw = 1.0/sinw;
        k0 = sin((1.0 - t)*w)*invSinw;
        k1 = sin(t * w) * invSinw;
    }

    ix::Node tmp("tmp");
    tmp.set(this,"#0",quaternionObjstart.c(0)->getDouble() * k0 + quaternionObjend.c(0)->getDouble() * k1);
    tmp.set(this,"#1",quaternionObjstart.c(1)->getDouble() * k0 + quaternionObjend.c(1)->getDouble() * k1);
    tmp.set(this,"#2",quaternionObjstart.c(2)->getDouble() * k0 + quaternionObjend.c(2)->getDouble() * k1);
    tmp.set(this,"#3",quaternionObjstart.c(3)->getDouble() * k0 + quaternionObjend.c(3)->getDouble() * k1);
    ix::calc::quaternion2Rot(&tmp);
    QList<double> ori_list;
    ori_list << tmp.c(0)->getDouble() << tmp.c(1)->getDouble() << tmp.c(2)->getDouble();
    return ori_list;
    //qDebug() << "ori:" << tmp.c(0)->getDouble() * transR << "," << tmp.c(1)->getDouble() * transR<< "," << tmp.c(2)->getDouble()* transR;
    //result_Node = &tmp;
}

