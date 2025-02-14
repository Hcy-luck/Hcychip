#include "chippage.h"
#include "ui_chippage.h"
#include "xservice.h"
#include "common/entry.h"
#include "xchip_helper.h"
#include "util/synctool.h"
#include "page/basketpoints.h"
#include "page/boatpoints.h"
#include "page/imexport.h"
#include "page/synchronizerl.h"
#include "page/workstationsetup.h"
#include "page/workstattionlocation.h"
#include <QToolButton>

IMOL_VER_REG("xchip.chip_page", 2)

using namespace xchip;

ChipPage::ChipPage(QWidget *parent_wgt, QObject *parent) :
    xcore::BasePage("xchip", tr("xchip"), parent),
    ui(new Ui::ChipPage),
    m_wgt(new QWidget(parent_wgt)),
    m_curr_tb(nullptr), m_curr_page(nullptr),
    m_result_mobj(new imol::ModuleObject("result_list")),
    m_sync_state(xchip::syncstateNode())
{
    ui->setupUi(m_wgt);

    m_wgt->setAttribute(Qt::WA_StyledBackground, true);
    m_pageList  << SubPageCfg(tr("StationSetting"), ":/img/image/worksta.png",[this]{return new WorkStationSetup(ui->wgtContent);},true)
                << SubPageCfg(tr("StationPoints"), ":/img/image/Position.png",[this]{return new WorkStattionLocation(ui->wgtContent);},true)
                << SubPageCfg(tr("BoatPoints"), ":/img/image/boat.png",[this]{return new BoatPoints(ui->wgtContent);})
                << SubPageCfg(tr("BasketPoints"), ":/img/image/basket.png",[this]{return new BasketPoints(ui->wgtContent);})
                << SubPageCfg(tr("SynchronizeRL"), ":/img/image/project.png",[this]{return new SynchronizeRL(ui->wgtContent);},true)
                << SubPageCfg(tr("ImExport"), ":/img/image/Export.png",[this]{return new ImExport(ui->wgtContent);});

    m_sync_name << "zhoupoints" << "workpoints" << "hualanpoints" << "frame";

    for(int i=0;i<m_pageList.count();i++){
        QToolButton* tbtn = new QToolButton(ui->wgtNav);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tbtn->sizePolicy().hasHeightForWidth());
        tbtn->setSizePolicy(sizePolicy);
        tbtn->setText(m_pageList.at(i).m_title);
        tbtn->setMinimumSize(QSize(0, 45));
        tbtn->setIconSize(QSize(30, 30));
        tbtn->setCheckable(true);
        tbtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui->wgtNav->layout()->addWidget(tbtn);
        QColor icon_color(Qt::black);
        tbtn->setIcon(ix::xtheme::pixmap(QPixmap(m_pageList.at(i).m_icon), icon_color));
//        QPalette patlette;
//        patlette.setColor(QPalette::ButtonText,Qt::black);
//        tbtn->setPalette(patlette);
        connect(tbtn, &QToolButton::clicked, this, &ChipPage::switchPage);
        m_pageList[i].m_tbtn = tbtn;
    }
    ui->wgtNav->layout()->addItem(new QSpacerItem(20, 352, QSizePolicy::Minimum, QSizePolicy::Expanding));
    if(m_pageList.count() > 0){
        m_pageList.at(0).m_tbtn->setChecked(true);
        emit m_pageList.at(0).m_tbtn->clicked(true);
    }

    connect(ui->btnsynch,&QPushButton::clicked,this,[=]{
        m_pageList[4].m_tbtn->click();
    });
}

void ChipPage::init(){
    autoUpdate();
    connect(rc(), &xcore::ControllerManager::rcConnected, this, [this]{
        loadFromRC();
        //每次连接控制器之后，*已经不能正确反应当前工艺包节点状态是否已经同步给工程；所以每次重新连接工程同步按钮复位
        ui->btnsynch->setText(tr("SynchToRL"));
        if(m_curr_page) m_curr_page->initPage();
        connect(m_sync_state,&ix::Node::activated,this,[=]{
            for(int i=0;i<m_sync_name.count();i++){
                if(m_sync_state->c(m_sync_name[i])->getBool()){
                    ui->btnsynch->setText(tr("SynchToRL*"));
                    return;
                }
            }
            ui->btnsynch->setText(tr("SynchToRL"));
        });

        for(int i=0;i<m_sync_name.count();i++){
            if(m_sync_name[i] != "hualanpoints"){
                connect(xchip::settingNode()->c(m_sync_name[i]),&ix::Node::activated,this,[=]{
                    m_sync_state->c(m_sync_name[i])->set(this,true);
                });
            }else{      //解决点进花篮点位界面，工程设置按钮就*的问题
                for(int j=0;j<4;j++){
                    connect(xchip::settingNode()->c(m_sync_name[i])->c(j)->c("pos"),&ix::Node::activated,this,[=]{
                        m_sync_state->c(m_sync_name[i])->set(this,true);
                    });
                }

            }

        }
        //
        xchip::settingNode()->c("is_cali_connect")->set(this,true);
    });
}

void ChipPage::render(){
    //setRecursiveWidgetStyleSheet(ui->wgtTitle, theme()->genStyleSheet(":/xchip/render/sub_page.json"));
    setRecursiveWidgetStyleSheet(m_wgt, theme()->genStyleSheet(":/xchip/render/sub_page.json"), m_render_exception_wgts);
}

void ChipPage::updateAvailability(){
    if(m_curr_page) m_curr_page->setAvailable(isAvailable());
}

void ChipPage::loadFromRC(){
    //考虑用rsync获取工艺包备份数据，copy给settingNode()
    QString workset_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta.xml";
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, workset_path);
    xchip::settingNode()->c("worksta")->copyFrom(this,m_result_mobj->c("worksta"));

    QString workpoints_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/workpoints.xml";
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, workpoints_path);
    xchip::settingNode()->c("workpoints")->copyFrom(this,m_result_mobj->c("workpoints"));

    QString frame_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/worksta_frame.xml";
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, frame_path);
    xchip::settingNode()->c("frame")->copyFrom(this,m_result_mobj->c("frame"));

    QString backetpoints_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/backetpoints.xml";
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, backetpoints_path);
    xchip::settingNode()->c("hualanpoints")->copyFrom(this,m_result_mobj->c("hualanpoints"));

    QString boatpoints_path = "./internal/" + m("xcore.r.id")->getString() + "/settings/boatpoints.xml";
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, boatpoints_path);
    xchip::settingNode()->c("zhoupoints")->copyFrom(this,m_result_mobj->c("zhoupoints"));
}

void ChipPage::switchPage(){
    auto tb = qobject_cast<QToolButton *>(sender());
    if(!tb) return;
    if(tb == m_curr_tb){
        tb->setChecked(true);
        return;
    }
    for(int i=0; i<m_pageList.count(); i++){
        if(tb != m_pageList.at(i).m_tbtn){
            m_pageList.at(i).m_tbtn->setChecked(false);
            m_pageList.at(i).m_tbtn->setIcon(ix::xtheme::pixmap(QPixmap(m_pageList.at(i).m_icon), Qt::black));
//            QPalette patlette;
//            patlette.setColor(QPalette::ButtonText,Qt::black);
//            m_pageList.at(i).m_tbtn->setPalette(patlette);
            BaseSubPage* curr_page = m_pageList.at(i).m_sub_page;
            if(m_pageList.at(i).m_is_resident){
                if(curr_page){
                    curr_page->wgt()->setVisible(false);
                }
            }else{
                if(curr_page){
                    ui->wgtContent->layout()->removeWidget(curr_page->wgt());
                    delete curr_page;
                    m_pageList[i].m_sub_page = nullptr;
                }
            }
        }else{
            BaseSubPage* curr_page = nullptr;
            if(m_pageList.at(i).m_is_resident){
                if(m_pageList.at(i).m_sub_page){
                    m_pageList.at(i).m_sub_page->wgt()->setVisible(true);
                    curr_page = m_pageList.at(i).m_sub_page;
                }else{
                    curr_page = m_pageList.at(i).m_func();
                    ui->wgtContent->layout()->addWidget(curr_page->wgt());
                }
            }else{
                curr_page = m_pageList.at(i).m_func();
                ui->wgtContent->layout()->addWidget(curr_page->wgt());
            }
            m_pageList[i].m_sub_page = curr_page;
            m_pageList.at(i).m_tbtn->setIcon(ix::xtheme::pixmap(QPixmap(m_pageList.at(i).m_icon), Qt::red));
            //ui->lblTitle->setText(curr_page->label());
//            QPalette patlette;
//            patlette.setColor(QPalette::ButtonText,Qt::red);
//            m_pageList.at(i).m_tbtn->setPalette(patlette);
            curr_page->initPage();//非常驻内存，每次切换页面都会调用initpage
            m_curr_page = curr_page;
            m_curr_page->wgt()->setEnabled(isAvailable());
        }
    }
    m_curr_tb = tb;
}

