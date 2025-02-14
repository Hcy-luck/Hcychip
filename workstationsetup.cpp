#include "workstationsetup.h"
#include "ui_workstationsetup.h"
#include "common/entry.h"
#include "../chippage.h"
#include "../xchip_helper.h"
#include "xservice.h"
#include "util/synctool.h"
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>

using namespace xchip;

IMOL_VER_REG("xchip.workstationsetup", 3)
WorkStationSetup::WorkStationSetup(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "workstationsetup",tr("WorkStationSetup"), parent),
    ui(new Ui::WorkStationSetup),
    m_defboat_mobj(defaultNode()->c("worksta")->c("zhouset")),
    m_defbacket_mobj(defaultNode()->c("worksta")->c("hualanset")),
    m_boat_mobj(workstaNode()->c("zhouset")),
    m_backet_mobj(workstaNode()->c("hualanset")),
    m_frame_mobj(frameNode()),
    m_boat_points_mobj(zhoupointsNode()),
    lock_state(false),
    m_result_mobj(new imol::ModuleObject("result_list"))
{
    ui->setupUi(wgt());

//    connect(rc(), &xcore::ControllerManager::rcConnected, this, [=]{
//        QString workset_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta.xml";
//                m_result_mobj->clear(this);
//                m().readFromXmlFile(this, m_result_mobj, workset_path);
//                xchip::settingNode()->c("worksta")->copyFrom(this,m_result_mobj->c("worksta"));

//                QString frame_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta_frame.xml";
//                m_result_mobj->clear(this);
//                m().readFromXmlFile(this, m_result_mobj, frame_path);
//                xchip::settingNode()->c("frame")->copyFrom(this,m_result_mobj->c("frame"));
//        refreshTable();
//    });

    connect(ui->rbtnboatbacket,&QRadioButton::clicked,this,[=]{
        ui->grbboatset->setHidden(false);
        ui->grpBacketSet->setHidden(false);
        ui->groupBox->setHidden(true);
        ui->groupBox_2->setHidden(true);
        ui->groupBox_3->setHidden(true);
        ui->groupBox_4->setHidden(true);
    });
    connect(ui->rbtnToolWobj,&QRadioButton::clicked,this,[=]{
        ui->grbboatset->setHidden(true);
        ui->grpBacketSet->setHidden(true);
        ui->groupBox->setHidden(false);

        ui->groupBox_3->setHidden(false);
        if(ui->rbtnposcali->isChecked()){
            ui->groupBox_4->setHidden(true);
            ui->groupBox_2->setHidden(false);
        }else if(ui->rbtnloadcali->isChecked()){
            ui->groupBox_2->setHidden(true);
            ui->groupBox_4->setHidden(false);
        }
        //
    });
    ui->rbtnboatbacket->setChecked(true);
    ui->grbboatset->setHidden(false);
    ui->grpBacketSet->setHidden(false);
    ui->groupBox->setHidden(true);
    ui->groupBox_2->setHidden(true);
    ui->groupBox_3->setHidden(true);
    ui->groupBox_4->setHidden(true);
    ui->lineEdit->setEnabled(false);
    ui->dspnx->setEnabled(false);
    ui->dspny->setEnabled(false);
    ui->dspnz->setEnabled(false);
    ui->dspna->setEnabled(false);
    ui->dspnb->setEnabled(false);
    ui->dspnc->setEnabled(false);
    QButtonGroup  *m_group_1 = new QButtonGroup(this);
    m_group_1->addButton(ui->rbtnboatbacket,1);
    m_group_1->addButton(ui->rbtnToolWobj,2);

    pbt_all_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali5 <<ui->btnCali6 << ui->btnCali;
    pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali ;
    state_list << "p1" <<"p2"<<"p3"<<"p4"<<"p";
    state().regist(this,state_list,true,imol::StateManager::LOOP_CONNECTION);
    refreshWidget();
    for(int i =0;i<6;i++){
        pbt_all_list[i]->setProperty("func","operation");
        connect(pbt_all_list[i],&QPushButton::clicked,this,[=]{
            //标定
            if(!checkCalibrateTool()) return;
            rc()->request("motion.move.get_joint_pos");
        });
    }
    ui->btnCali->setProperty("func","operation");


    connect(state().watcher(this),&imol::StateWatcher::changed,this,[this](imol::StateObject *from, imol::StateObject *to){
        refreshWidget();
    });

    connect(ui->btnCali,&QPushButton::clicked,this,[=]{
        imol::ModuleObject *frame_mobj = rc()->requestMobj("robot.calibrate.frame");

        if(ui->rbtnTool->isChecked()){
            frame_mobj->set(this,"coordination_type","tool");
            frame_mobj->set(this,"rob_hold",true);
        }else{
            frame_mobj->set(this,"coordination_type","wobj");
            frame_mobj->set(this,"rob_hold",false);
        }
        frame_mobj->set(this,"point_num",state_list.size() - 1);
        frame_mobj->set(this,"related_frame",0);

        int pos_vec_size = frame_mobj->cmobj("pos_vec")->size();
        for(int i = state_list.size() - 1;i<pos_vec_size;++i){
            frame_mobj->remove(this,mName("pos_vec",mIndex(state_list.size() -1)));
        }
        frame_mobj->set(this,"pos_vec",QVariant());
        foreach(imol::ModuleObject *pos_mobj,frame_mobj->cmobj("pos_vec")->cmobjs()){
            pos_mobj->set(this,"inner_pos",QVariant());
            pos_mobj->set(this,"extern_pos",QVariant());
            pos_mobj->c("extern_pos")->clear(this);
            int axis_num = m("xmate.robot.axis")->getInt();
            for(int i = axis_num;i<pos_mobj->c("inner_pos")->cmobjCount();i++){
                pos_mobj->c("inner_pos")->remove(this,pos_mobj->c("inner_pos")->c(i));
            }
        }
        rc()->request("robot.calibrate.frame");
        frame_mobj->copyFrom(this,xchip::defaultNode()->c("cali_frame"));
        state().toNext(this);
    });

    connect(ui->btnConfirm,&QPushButton::clicked,this,[=]{
        double transK = 1000.0;
        double transR = 180.0/3.1415926;
        if(ui->rbtnTool->isChecked()){
            if(!helper().execMsgOk("info",tr("Are you sure to save the s_tool?"))){
                getToolWobj("tool","s_tool");
                return;
            }
            m_frame_mobj->c("tool")->c("s_tool")->c("trans")->c("#0")->set(this,ui->dspnx->value()/transK);
            m_frame_mobj->c("tool")->c("s_tool")->c("trans")->c("#1")->set(this,ui->dspny->value()/transK);
            m_frame_mobj->c("tool")->c("s_tool")->c("trans")->c("#2")->set(this,ui->dspnz->value()/transK);
            m_frame_mobj->c("tool")->c("s_tool")->c("rot")->c("#0")->set(this,ui->dspna->value()/transR);
            m_frame_mobj->c("tool")->c("s_tool")->c("rot")->c("#1")->set(this,ui->dspnb->value()/transR);
            m_frame_mobj->c("tool")->c("s_tool")->c("rot")->c("#2")->set(this,ui->dspnc->value()/transR);
        }
        else if(ui->rbtnBoat1->isChecked()){
            if(!helper().execMsgOk("info",tr("Are you sure to save the s_boat1?"))){
                getToolWobj("wobj","s_boat1");
                return;
            }
            m_frame_mobj->c("wobj")->c("s_boat1")->c("trans")->c("#0")->set(this,ui->dspnx->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat1")->c("trans")->c("#1")->set(this,ui->dspny->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat1")->c("trans")->c("#2")->set(this,ui->dspnz->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat1")->c("rot")->c("#0")->set(this,ui->dspna->value()/transR);
            m_frame_mobj->c("wobj")->c("s_boat1")->c("rot")->c("#1")->set(this,ui->dspnb->value()/transR);
            m_frame_mobj->c("wobj")->c("s_boat1")->c("rot")->c("#2")->set(this,ui->dspnc->value()/transR);
        }
        else if(ui->rbtnBoat2->isChecked()){
            if(!helper().execMsgOk("info",tr("Are you sure to save the s_boat2?"))){
                getToolWobj("wobj","s_boat2");
                return;
            }
            m_frame_mobj->c("wobj")->c("s_boat2")->c("trans")->c("#0")->set(this,ui->dspnx->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat2")->c("trans")->c("#1")->set(this,ui->dspny->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat2")->c("trans")->c("#2")->set(this,ui->dspnz->value()/transK);
            m_frame_mobj->c("wobj")->c("s_boat2")->c("rot")->c("#0")->set(this,ui->dspna->value()/transR);
            m_frame_mobj->c("wobj")->c("s_boat2")->c("rot")->c("#1")->set(this,ui->dspnb->value()/transR);
            m_frame_mobj->c("wobj")->c("s_boat2")->c("rot")->c("#2")->set(this,ui->dspnc->value()/transR);
        }
        saveWorksettoRC();
        helper().throwLog(this,"info",tr("Successfully saved"));
    });

    connect(ui->btnConfirm_load,&QPushButton::clicked,this,[=]{
        if(!helper().execMsgOk("info",tr("Are you sure to save load information of the tool?"))){
            ui->dspnQuality->setValue(m_frame_mobj->r("tool.s_tool.load.mass")->getDouble());
            ui->dspnx_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#0")->getDouble() * 1000);
            ui->dspny_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#1")->getDouble() * 1000);
            ui->dspnz_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#2")->getDouble() * 1000);
            return;
        }
        m_frame_mobj->r("tool.s_tool.load.mass")->set(this,ui->dspnQuality->value());
        m_frame_mobj->r("tool.s_tool.load.cog.#0")->set(this,ui->dspnx_2->value()/1000);
        m_frame_mobj->r("tool.s_tool.load.cog.#1")->set(this,ui->dspny_2->value()/1000);
        m_frame_mobj->r("tool.s_tool.load.cog.#2")->set(this,ui->dspnz_2->value()/1000);
        saveWorksettoRC();
        helper().throwLog(this,"info",tr("Successfully saved"));
    });

//    connect(rc()->responseMobj("motion.move.get_joint_pos"),&imol::ModuleObject::changed,this,[=]{
//        rc()->responseMobj("robot.calibrate.frame")->rmobj(mName("pos_vec",mIndex(1)))->copyFrom(this,rc()->responseMobj("motion.move.get_joint_pos"));
//    });

    connect(ui->btnPageLock,&QPushButton::clicked,this,[=]{
        if(!lock_state){
            ui->rbtnboatbacket->setEnabled(false);
            ui->rbtnToolWobj->setEnabled(false);
            ui->grbboatset->setEnabled(false);
            ui->grpBacketSet->setEnabled(false);
            ui->groupBox->setEnabled(false);
            ui->groupBox_2->setEnabled(false);
            ui->groupBox_3->setEnabled(false);
            ui->groupBox_4->setEnabled(false);
            ui->btnPageLock->setText(tr("Page Unlock"));
            lock_state = true;
        }
        else{
            ui->rbtnboatbacket->setEnabled(true);
            ui->rbtnToolWobj->setEnabled(true);
            ui->grbboatset->setEnabled(true);
            ui->grpBacketSet->setEnabled(true);
            ui->groupBox->setEnabled(true);
            ui->groupBox_2->setEnabled(true);
            ui->groupBox_3->setEnabled(true);
            ui->groupBox_4->setEnabled(true);
            ui->btnPageLock->setText(tr("Page Lock"));
            lock_state = false;
        }
    });

//    connect(rc()->responseMobj("motion.move.get_joint_pos"),&imol::ModuleObject::changed,this,[=]{

//    });

    initTable();

    //rbtn限制

    connect(ui->rbtnTool,&QRadioButton::clicked,this,&WorkStationSetup::selectToolWobj);
    connect(ui->rbtnBoat1,&QRadioButton::clicked,this,&WorkStationSetup::selectToolWobj);
    connect(ui->rbtnBoat2,&QRadioButton::clicked,this,&WorkStationSetup::selectToolWobj);
    connect(ui->rbtnposcali,&QRadioButton::clicked,this,&WorkStationSetup::selectToolLoad);
    connect(ui->rbtnloadcali,&QRadioButton::clicked,this,&WorkStationSetup::selectToolLoad);
    connect(ui->rbtnthree,&QRadioButton::clicked,this,&WorkStationSetup::selectPointsNum);
    connect(ui->rbtnfour,&QRadioButton::clicked,this,&WorkStationSetup::selectPointsNum);
    connect(ui->rbtnsix,&QRadioButton::clicked,this,&WorkStationSetup::selectPointsNum);
    connect(ui->rbtnmanual,&QRadioButton::clicked,this,&WorkStationSetup::selectPointsNum);
    ui->rbtnTool->setChecked(true);
    ui->rbtnposcali->setChecked(true);
    ui->rbtnfour->setChecked(true);
    ui->groupBox_4->setHidden(true);
    ui->lineEdit->setText("s_tool");
    ui->rbtnthree->setHidden(true);
    ui->btnCali5->setHidden(true);
    ui->btnCali6->setHidden(true);

    //舟设置
    connect(ui->btnConfirmBoat,&QPushButton::clicked,this,&WorkStationSetup::SetBoat);
    //花篮设置
    connect(ui->btnConfirmBacket,&QPushButton::clicked,this,&WorkStationSetup::SetBacket);

}

WorkStationSetup::~WorkStationSetup()
{
    delete ui;
}

void WorkStationSetup::initNode(){

}

void WorkStationSetup::initTable(){
   refreshTable();
}

void WorkStationSetup::initPage()
{
    if(!xchip::settingNode()->c("is_cali_connect")->getBool()){
        connect(rc()->responseMobj("motion.move.get_joint_pos"),&imol::ModuleObject::changed,this,[=]{
            if (m("xnav._current")->getString() != "XChip.page") return;
            QString cur_state = state().current(this)->name();
            int index = state_list.indexOf(cur_state);
            rc()->requestMobj("robot.calibrate.frame")->rmobj(mName("pos_vec",mIndex(index)))->copyFrom(this,rc()->responseMobj("motion.move.get_joint_pos"));
            state().toNext(this);
        });


        connect(rc()->responseMobj("robot.calibrate.frame"),&imol::ModuleObject::changed,this,[=]{
            if (m("xnav._current")->getString() != "XChip.page") return;
            QString calibrate_result = tr("Calibration success!") + "\n";
            QVariantList errors = rc()->responseMobj("robot.calibrate.frame")->cmobj("calibrate_error")->getList();
            calibrate_result.append(tr("Calibrate error:") + tr("minimum:%1mm, maximum:%3mm, average:%2mm")
                                    .arg(errors.value(0,0).toDouble() * 1000, 0, 'f', 4)
                                    .arg(errors.value(1,0).toDouble() * 1000, 0, 'f', 4)
                                    .arg(errors.value(2,0).toDouble() * 1000, 0, 'f', 4));
            if(!helper().execMsgOk("info",calibrate_result)) return;
            imol::ModuleObject *result_mobj = rc()->responseMobj("robot.calibrate.frame");
            ui->dspnx->setValue(result_mobj->cmobj("frame")->cmobj("pos")->cmobj("x")->getDouble());
            ui->dspny->setValue(result_mobj->cmobj("frame")->cmobj("pos")->cmobj("y")->getDouble());
            ui->dspnz->setValue(result_mobj->cmobj("frame")->cmobj("pos")->cmobj("z")->getDouble());
            ui->dspna->setValue(result_mobj->cmobj("frame")->cmobj("ori")->cmobj("euler")->cmobj("a")->getDouble());
            ui->dspnb->setValue(result_mobj->cmobj("frame")->cmobj("ori")->cmobj("euler")->cmobj("b")->getDouble());
            ui->dspnc->setValue(result_mobj->cmobj("frame")->cmobj("ori")->cmobj("euler")->cmobj("c")->getDouble());
        });
    }

    //重新进入界面，重置标定
    state_list.clear();
    pbt_list.clear();
    if(ui->rbtnthree->isChecked()){
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p";
    }else if(ui->rbtnfour->isChecked()){
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p4"<<"p";
    }else if(ui->rbtnsix->isChecked()){
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali5 << ui->btnCali6 << ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p4"<<"p5"<<"p6"<<"p";
    }

    state().regist(this,state_list,true,imol::StateManager::LOOP_CONNECTION);
    refreshWidget();

    refreshTable();
}

void WorkStationSetup::refreshTable(){
    imol::ModuleObject *boat1 = m_boat_mobj->c(0);
    imol::ModuleObject *boat2 = m_boat_mobj->c(1);
    ui->cbenableboat1->setChecked(boat1->c("enable")->getBool());
    ui->cbenableboat2->setChecked(boat2->c("enable")->getBool());
    ui->spbboat1->setValue(boat1->c("slots")->getInt());
    ui->spbboat2->setValue(boat2->c("slots")->getInt());

    imol::ModuleObject *backet1 = m_backet_mobj->c(0);
    imol::ModuleObject *backet2 = m_backet_mobj->c(1);
    imol::ModuleObject *backet3 = m_backet_mobj->c(2);
    imol::ModuleObject *backet4 = m_backet_mobj->c(3);
    ui->cbbasket1->setChecked(backet1->c("enable")->getBool());
    ui->cbbasket2->setChecked(backet2->c("enable")->getBool());
    ui->cbbasket3->setChecked(backet3->c("enable")->getBool());
    ui->cbbasket4->setChecked(backet4->c("enable")->getBool());

    if(ui->rbtnTool->isChecked()){
        getToolWobj("tool","s_tool");
    }else if(ui->rbtnBoat1->isChecked()){
        getToolWobj("wobj","s_boat1");
    }else if(ui->rbtnBoat2->isChecked()){
        getToolWobj("wobj","s_boat2");
    }

    if(ui->rbtnloadcali->isChecked()){
        ui->dspnQuality->setValue(m_frame_mobj->r("tool.s_tool.load.mass")->getDouble());
        ui->dspnx_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#0")->getDouble() * 1000);
        ui->dspny_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#1")->getDouble() * 1000);
        ui->dspnz_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#2")->getDouble() * 1000);
    }

}

void WorkStationSetup::SetBoat(){
    if(!helper().execMsgOk("info",tr("Are you sure to save the boat set?"))){
        refreshTable();
        return;
    }
    int boat1 = m_boat_mobj->c(0)->c("slots")->getInt();
    int boat2 = m_boat_mobj->c(1)->c("slots")->getInt();
    bool boat_enable1 = m_boat_mobj->c(0)->c("enable")->getBool();
    bool boat_enable2 = m_boat_mobj->c(1)->c("enable")->getBool();
    if(boat_enable1 == ui->cbenableboat1->isChecked() && boat_enable2 == ui->cbenableboat2->isChecked()
            && boat1 == ui->spbboat1->value() && boat2 == ui->spbboat2->value()) {
        helper().throwLog(this,"info",tr("Successfully saved the boat set"));
        return;
    }
    m_boat_mobj->c(0)->c("name")->set(this,tr("boat1"));
    m_boat_mobj->c(0)->c("enable")->set(this,ui->cbenableboat1->isChecked());
    m_boat_mobj->c(0)->c("slots")->set(this,ui->spbboat1->value());
    m_boat_mobj->c(1)->c("name")->set(this,tr("boat2"));
    m_boat_mobj->c(1)->c("enable")->set(this,ui->cbenableboat2->isChecked());
    m_boat_mobj->c(1)->c("slots")->set(this,ui->spbboat2->value());

    saveWorksettoRC();
    helper().throwLog(this,"info",tr("Successfully saved the boat set"));
    //如果没有舟点位并且点位基准和补偿都为空；return
    bool is_has_slotpoints = false;
    if(m_boat_points_mobj->cmobjCount() == 0){
        for(int i = 0;i<8;i++){
            if(xchip::defaultNode()->c("cali_point")->c(i)->c("reference")->cmobjCount() !=0 || xchip::defaultNode()->c("cali_point")->c(i)->c("comp")->cmobjCount() !=0){
                is_has_slotpoints = true;
                break;
            }
        }
    } else{
        is_has_slotpoints = true;
    }
    if(!is_has_slotpoints) return;
    //弹窗提示
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Confirm Dialog"));
    msgBox.setText(tr("The boat settings have changed. Do you want to reserve the previous boat points?"));
    QPushButton *yesButton = msgBox.addButton(tr("Reserve"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(tr("Not Reserve"), QMessageBox::NoRole);
    msgBox.setDefaultButton(yesButton);// 设置默认按钮
    msgBox.exec();
    if (msgBox.clickedButton() == yesButton){
        //保存，删除超出槽数的槽点位，其他保存
        int boat_slots1 = m_boat_mobj->c(0)->c("slots")->getInt();
        int boat_slots2 = m_boat_mobj->c(1)->c("slots")->getInt();

        for(int j =0;j<4;j++){
            for(int i = xchip::defaultNode()->c("cali_point")->c(j)->c("reference")->cmobjCount() - 1;i > boat_slots1 -1; i--){
                xchip::defaultNode()->c("cali_point")->c(j)->c("reference")->remove(this,xchip::defaultNode()->c("cali_point")->c(j)->c("reference")->c(i));
                xchip::defaultNode()->c("cali_point")->c(j)->c("comp")->remove(this,xchip::defaultNode()->c("cali_point")->c(j)->c("comp")->c(i));
            }
            for(int i = xchip::defaultNode()->c("cali_point")->c(j + 4)->c("reference")->cmobjCount() - 1;i > boat_slots2-1; i--){
                xchip::defaultNode()->c("cali_point")->c(j+4)->c("reference")->remove(this,xchip::defaultNode()->c("cali_point")->c(j+4)->c("reference")->c(i));
                xchip::defaultNode()->c("cali_point")->c(j+4)->c("comp")->remove(this,xchip::defaultNode()->c("cali_point")->c(j+4)->c("comp")->c(i));
            }
        }

        for(int i = m_boat_points_mobj->cmobjCount() - 1; i >= 0 ;i--){
            QString point_name = m_boat_points_mobj->c(i)->c("name")->getString();
            if(point_name.contains("1_")){
                //舟1点位
                bool is_delete = true;
                for(int j = 1;j<=boat_slots1;j++){
                    if(point_name.contains("c" + QString::number(j))) {
                        is_delete = false;
                        break;
                    }
                }
                if(is_delete) m_boat_points_mobj->remove(this,m_boat_points_mobj->c(i));
            }
            else if(point_name.contains("2_")){
                //舟2点位
                bool is_delete = true;
                for(int j = 1;j<=boat_slots2;j++){
                    if(point_name.contains("c" + QString::number(j))) {
                        is_delete = false;
                        break;
                    }
                }
                if(is_delete) m_boat_points_mobj->remove(this,m_boat_points_mobj->c(i));
            }
        }
    }else if(msgBox.clickedButton() == noButton){
        //不保存，删除所有舟点位
        m_boat_points_mobj->clear(this);
        for(int i =0;i<8;i++){
            xchip::defaultNode()->c("cali_point")->c(i)->c("is_cali")->set(this,false);
            xchip::defaultNode()->c("cali_point")->c(i)->c("reference")->clear(this);
            xchip::defaultNode()->c("cali_point")->c(i)->c("comp")->clear(this);
        }
    }

//    if(helper().execMsgOk("info",tr("The boat settings have changed. Do you want to save the previous boat points?"))){
//        //保存，删除超出槽数的槽点位，其他保存
//        int boat_slots1 = m_boat_mobj->c(0)->c("slots")->getInt();
//        int boat_slots2 = m_boat_mobj->c(1)->c("slots")->getInt();
//        for(int i = m_boat_points_mobj->cmobjCount() - 1; i >= 0 ;i--){
//            QString point_name = m_boat_points_mobj->c(i)->c("name")->getString();
//            if(point_name.contains("1_")){
//                //舟1点位
//                bool is_delete = true;
//                for(int j = 1;j<=boat_slots1;j++){
//                    if(point_name.contains("c" + QString::number(j))) {
//                        is_delete = false;
//                        break;
//                    }
//                }
//                if(is_delete) m_boat_points_mobj->remove(this,m_boat_points_mobj->c(i));
//            }
//            else if(point_name.contains("2_")){
//                //舟2点位
//                bool is_delete = true;
//                for(int j = 1;j<=boat_slots2;j++){
//                    if(point_name.contains("c" + QString::number(j))) {
//                        is_delete = false;
//                        break;
//                    }
//                }
//                if(is_delete) m_boat_points_mobj->remove(this,m_boat_points_mobj->c(i));
//            }
//        }
//    }else{
//        //不保存，删除所有舟点位
//        m_boat_points_mobj->clear(this);
//    }
}

void WorkStationSetup::SetBacket(){
    if(!helper().execMsgOk("info",tr("Are you sure to save the basket set?"))){
        refreshTable();
        return;
    }
    m_backet_mobj->c(0)->c("name")->set(this,tr("basket1"));
    m_backet_mobj->c(0)->c("enable")->set(this,ui->cbbasket1->isChecked());
    m_backet_mobj->c(1)->c("name")->set(this,tr("basket2"));
    m_backet_mobj->c(1)->c("enable")->set(this,ui->cbbasket2->isChecked());
    m_backet_mobj->c(2)->c("name")->set(this,tr("basket3"));
    m_backet_mobj->c(2)->c("enable")->set(this,ui->cbbasket3->isChecked());
    m_backet_mobj->c(3)->c("name")->set(this,tr("basket4"));
    m_backet_mobj->c(3)->c("enable")->set(this,ui->cbbasket4->isChecked());

    saveWorksettoRC();
    helper().throwLog(this,"info",tr("Successfully saved the basket set"));
}

void WorkStationSetup::CaliToolAndWobj(const QString& point_name, const QString& tool_wobj_name){

}

void WorkStationSetup::selectToolWobj(){
    if(ui->rbtnTool->isChecked()){
        ui->lineEdit->setText(tr("s_tool"));

        ui->rbtnloadcali->setHidden(false);
        ui->rbtnposcali->setHidden(false);
        ui->rbtnposcali->click();

        ui->rbtnthree->setHidden(true);
        ui->rbtnfour->setHidden(false);
        ui->rbtnsix->setHidden(false);
        ui->rbtnmanual->setHidden(false);
        ui->rbtnfour->click();
        getToolWobj("tool","s_tool");
    }
    if(ui->rbtnBoat1->isChecked()){
        ui->lineEdit->setText(tr("s_boat1"));

        ui->rbtnloadcali->setHidden(true);
        ui->rbtnposcali->setHidden(false);
        ui->rbtnposcali->click();

        ui->rbtnthree->setHidden(false);
        ui->rbtnfour->setHidden(true);
        ui->rbtnsix->setHidden(true);
        ui->rbtnmanual->setHidden(false);
        ui->rbtnthree->click();
        getToolWobj("wobj","s_boat1");
    }
    if(ui->rbtnBoat2->isChecked()){
        ui->lineEdit->setText(tr("s_boat2"));

        ui->rbtnloadcali->setHidden(true);
        ui->rbtnposcali->setHidden(false);
        ui->rbtnposcali->click();

        ui->rbtnthree->setHidden(false);
        ui->rbtnfour->setHidden(true);
        ui->rbtnsix->setHidden(true);
        ui->rbtnmanual->setHidden(false);
        ui->rbtnthree->click();
        getToolWobj("wobj","s_boat2");
    }
}

void WorkStationSetup::selectToolLoad(){
    if(ui->rbtnposcali->isChecked()){
        ui->groupBox_2->setHidden(false);
        ui->groupBox_4->setHidden(true);
    }
    if(ui->rbtnloadcali->isChecked()){
        ui->groupBox_2->setHidden(true);
        ui->groupBox_4->setHidden(false);
        ui->dspnQuality->setValue(m_frame_mobj->r("tool.s_tool.load.mass")->getDouble());
        ui->dspnx_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#0")->getDouble() * 1000);
        ui->dspny_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#1")->getDouble() * 1000);
        ui->dspnz_2->setValue(m_frame_mobj->r("tool.s_tool.load.cog.#2")->getDouble() * 1000);
    }
}

void WorkStationSetup::selectPointsNum(){
    if(ui->rbtnthree->isChecked()){
        ui->btnCali1->setHidden(false);
        ui->btnCali2->setHidden(false);
        ui->btnCali3->setHidden(false);
        ui->btnCali4->setHidden(true);
        ui->btnCali5->setHidden(true);
        ui->btnCali6->setHidden(true);
        ui->btnCali->setHidden(false);
        ui->dspnx->setEnabled(false);
        ui->dspny->setEnabled(false);
        ui->dspnz->setEnabled(false);
        ui->dspna->setEnabled(false);
        ui->dspnb->setEnabled(false);
        ui->dspnc->setEnabled(false);
        state_list.clear();
        pbt_list.clear();
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p";
        state().regist(this,state_list,true,imol::StateManager::LOOP_CONNECTION);
        refreshWidget();
    }
    if(ui->rbtnfour->isChecked()){
        ui->btnCali1->setHidden(false);
        ui->btnCali2->setHidden(false);
        ui->btnCali3->setHidden(false);
        ui->btnCali4->setHidden(false);
        ui->btnCali5->setHidden(true);
        ui->btnCali6->setHidden(true);
        ui->btnCali->setHidden(false);
        ui->dspnx->setEnabled(false);
        ui->dspny->setEnabled(false);
        ui->dspnz->setEnabled(false);
        ui->dspna->setEnabled(false);
        ui->dspnb->setEnabled(false);
        ui->dspnc->setEnabled(false);
        state_list.clear();
        pbt_list.clear();
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p4"<<"p";
        state().regist(this,state_list,true,imol::StateManager::LOOP_CONNECTION);
        refreshWidget();
    }
    if(ui->rbtnsix->isChecked()){
        ui->btnCali1->setHidden(false);
        ui->btnCali2->setHidden(false);
        ui->btnCali3->setHidden(false);
        ui->btnCali4->setHidden(false);
        ui->btnCali5->setHidden(false);
        ui->btnCali6->setHidden(false);
        ui->btnCali->setHidden(false);
        ui->dspnx->setEnabled(false);
        ui->dspny->setEnabled(false);
        ui->dspnz->setEnabled(false);
        ui->dspna->setEnabled(false);
        ui->dspnb->setEnabled(false);
        ui->dspnc->setEnabled(false);
        state_list.clear();
        pbt_list.clear();
        pbt_list << ui->btnCali1 << ui->btnCali2 <<ui->btnCali3<< ui->btnCali4 << ui->btnCali5 << ui->btnCali6 << ui->btnCali ;
        state_list << "p1" <<"p2"<<"p3"<<"p4"<<"p5"<<"p6"<<"p";
        state().regist(this,state_list,true,imol::StateManager::LOOP_CONNECTION);
        refreshWidget();
    }
    if(ui->rbtnmanual->isChecked()){
        ui->btnCali1->setHidden(true);
        ui->btnCali2->setHidden(true);
        ui->btnCali3->setHidden(true);
        ui->btnCali4->setHidden(true);
        ui->btnCali5->setHidden(true);
        ui->btnCali6->setHidden(true);
        ui->btnCali->setHidden(true);
        ui->dspnx->setEnabled(true);
        ui->dspny->setEnabled(true);
        ui->dspnz->setEnabled(true);
        ui->dspna->setEnabled(true);
        ui->dspnb->setEnabled(true);
        ui->dspnc->setEnabled(true);
    }
}

void WorkStationSetup::getToolWobj(const QString& type,const QString& name){
    double transK = 1000.0;
    double transR = 180.0/3.1415926;
    ui->dspnx->setValue(m_frame_mobj->c(type)->c(name)->c("trans")->c("#0")->getDouble() * transK);
    ui->dspny->setValue(m_frame_mobj->c(type)->c(name)->c("trans")->c("#1")->getDouble() * transK);
    ui->dspnz->setValue(m_frame_mobj->c(type)->c(name)->c("trans")->c("#2")->getDouble() * transK);
    ui->dspna->setValue(m_frame_mobj->c(type)->c(name)->c("rot")->c("#0")->getDouble() * transR);
    ui->dspnb->setValue(m_frame_mobj->c(type)->c(name)->c("rot")->c("#1")->getDouble() * transR);
    ui->dspnc->setValue(m_frame_mobj->c(type)->c(name)->c("rot")->c("#2")->getDouble() * transR);
}

void WorkStationSetup::refreshWidget(){
    int cpnum = state_list.size();
    QString cur_state = state().current(this)->name();
    int index = state_list.indexOf(cur_state);
    if(index<0 || index>cpnum) return;
    for(int i =0;i<cpnum;i++){
        pbt_list[i]->setEnabled(cur_state == state_list[i]);
    } 
}

void WorkStationSetup::saveWorksettoRC(){
    //保存到hmi的setting
    QString source_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta.xml";
    imol::ModuleObject temp_workset("workset");
    temp_workset.append(this,"worksta")->copyFrom(this, xchip::settingNode()->c("worksta"));
    //m().writeToJson(source_path,temp_workset.exportToJson());
    m().writeToXmlFile(source_path, &temp_workset);

    QString frame_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta_frame.xml";
    imol::ModuleObject temp_frame("frame");
    temp_frame.append(this,"frame")->copyFrom(this, xchip::settingNode()->c("frame"));
    m().writeToXmlFile(frame_path, &temp_frame);

    //rsync到控制器
    QString path = "./internal/" + m("xcore.r.id")->getString() + "/settings/";
    SyncTool::instance()->pushFile(path,"user_settings");
}

bool WorkStationSetup::checkCalibrateTool(){
    const QString curToolName = m("xmate.toolpage.tool")->getString();
    const QString curWobjName = m("xmate.toolpage.wobj")->getString();

    if(ui->rbtnTool->isChecked()){
        if(curToolName == "tool0"){
            if(curWobjName == "wobj0"){
                return true;
            }
            foreach(auto* wobjMobj,m("xmate.project.wobj._list")->cmobjs()){
                if(wobjMobj->c("name")->getString() == curWobjName){
                    if(wobjMobj->c("robhold")->getBool()){
                        helper().throwLog(this,"error",tr("Please set current wobj to wobj0 or external wobj!"));
                        return false;
                    }
                    return true;
                }
            }
        }
        helper().throwLog(this,"error",tr("Please use tool0 to calibrate the robot hold tool!"));
        return false;
    }

    if(ui->rbtnBoat1->isChecked() || ui->rbtnBoat2->isChecked()){
        if(curToolName == "tool0"){
            if(curWobjName == "wobj0"){
                return true;
            }
            foreach(auto* wobjMobj,m("xmate.project.wobj._list")->cmobjs()){
                if(wobjMobj->c("name")->getString() == curWobjName){
                    if(wobjMobj->c("robhold")->getBool()){
                        helper().throwLog(this,"error",tr("Please set current wobj to wobj0 or external wobj!"));
                        return false;
                    }
                    return true;
                }
            }
        }
        foreach(auto* toolMobj,m("xmate.project.tool._list")->cmobjs()){
            if(toolMobj->c("name")->getString() == curToolName){
                if(toolMobj->c("robhold")->getBool()){
                    return true;
                }
                helper().throwLog(this,"error",tr("Please use robot hold tool to calibrate the external wobj!"));
                return false;
            }
        }
    }
    return true;
}



