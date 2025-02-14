#include "synchronizerl.h"
#include "ui_synchronizerl.h"
#include "common/entry.h"
#include "../chippage.h"
#include "../xchip_helper.h"
#include "xservice.h"
#include <qmath.h>
#include <QLabel>

using namespace xchip;

IMOL_VER_REG("xchip.synchronizerl", 1)
SynchronizeRL::SynchronizeRL(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "synchronizerl",tr("SynchronizeRL"), parent),
    ui(new Ui::SynchronizeRL)
{
    ui->setupUi(wgt());
    ui->edtProject->setEnabled(false);

    connect(ui->btnExport, &QPushButton::clicked, this, &SynchronizeRL::exportToProject);
    connect(ui->btnClear, &QPushButton::clicked, this, [this]{ui->plainTextEdit->clear();});

    connect(ui->btnExportWorkStaPoint, &QPushButton::clicked, this, [this]{
        if(!checkProject()) return;
        exportWorkStaPoint();
        saveProject();
    });

    connect(ui->btnExportBoatPoint, &QPushButton::clicked, this, [this]{
        if(!checkProject()) return;
        exportBoatPoint();
        saveProject();
    });

    connect(ui->btnExportBacketPoint, &QPushButton::clicked, this, [this]{
        if(!checkProject()) return;
        exportBacketPoint();
        saveProject();
    });

    connect(ui->btnExportToolWobj, &QPushButton::clicked, this, [this]{
        if(!checkProject()) return;
        exportFrame();
        saveProject();
    });

    connect(m("xmate.project.cur_project"), &imol::ModuleObject::changed, this, [this]{
        refreshProject();
    });
}

SynchronizeRL::~SynchronizeRL()
{
    delete ui;
}

void SynchronizeRL::initPage()
{
    refreshProject();
}

ix::Node* getChildNode(const QString& name, ix::Node* list_node)
{
    foreach (auto n, list_node->cmobjs()) {
        if(n->c("name")->getString() == name){
            return n;
        }
    }
    return nullptr;
}

void point2frame(QObject *context, ix::Node *point_node, ix::Node *frame_node)
{
    ix::Node tmp("tmp");
    frame_node->set(context, "ori.euler.a", point_node->r("rot.#0")->getDouble() / M_PI * 180);
    frame_node->set(context, "ori.euler.b", point_node->r("rot.#1")->getDouble() / M_PI * 180);
    frame_node->set(context, "ori.euler.c", point_node->r("rot.#2")->getDouble() / M_PI * 180);
    ix::calc::rot2Quaternion(point_node->c("rot"), &tmp);
    frame_node->set(context, "ori.quaternion.q1", tmp.c(0)->getDouble());
    frame_node->set(context, "ori.quaternion.q2", tmp.c(1)->getDouble());
    frame_node->set(context, "ori.quaternion.q3", tmp.c(2)->getDouble());
    frame_node->set(context, "ori.quaternion.q4", tmp.c(3)->getDouble());
    frame_node->set(context, "pos.x", point_node->r("trans.#0")->getDouble() * 1000);
    frame_node->set(context, "pos.y", point_node->r("trans.#1")->getDouble() * 1000);
    frame_node->set(context, "pos.z", point_node->r("trans.#2")->getDouble() * 1000);
}

void xLoad2Load(QObject *context, ix::Node *xload_node, ix::Node *load_node)
{
    load_node->set(context, "mass", xload_node->r("mass")->getDouble());
    load_node->set(context, "moment.#0", xload_node->r("inertia.#0")->getDouble() * 1000000);
    load_node->set(context, "moment.#1", xload_node->r("inertia.#1")->getDouble() * 1000000);
    load_node->set(context, "moment.#2", xload_node->r("inertia.#2")->getDouble() * 1000000);
    load_node->set(context, "pose.pos.x", xload_node->r("cog.#0")->getDouble() * 1000);
    load_node->set(context, "pose.pos.y", xload_node->r("cog.#1")->getDouble() * 1000);
    load_node->set(context, "pose.pos.z", xload_node->r("cog.#2")->getDouble() * 1000);
}

void xTool2Tool(QObject *context, ix::Node *xtool_node, ix::Node *tool_node)
{
    tool_node->set(context, "name", xtool_node->name());
    tool_node->set(context, "description", xtool_node->c("dec")->getString());
    tool_node->set(context, "robhold", true);
    tool_node->set(context, "pers", false);
    tool_node->set(context, "calibrated", true);
    tool_node->set(context, "identified", true);
    point2frame(context, xtool_node, tool_node->c("frame"));
    xLoad2Load(context, xtool_node->c("load"), tool_node->c("load"));
}

void xWobj2Wobj(QObject *context, ix::Node *xwobj_node, ix::Node *wobj_node){
    wobj_node->set(context,"name",xwobj_node->name());
    wobj_node->set(context,"description", xwobj_node->c("dec")->getString());
    wobj_node->set(context, "identified", true);
    wobj_node->set(context,"robhold", true);
    wobj_node->set(context, "pers", false);
    wobj_node->set(context,"mechanic","robot");
    wobj_node->set(context,"movable",true);
    point2frame(context,xwobj_node,wobj_node->c("frame"));
}

void SynchronizeRL::exportToProject()
{
    if(!checkProject()) return;

    exportWorkStaPoint();
    exportBacketPoint();
    exportBoatPoint();
    exportFrame();
    saveProject();
}

void SynchronizeRL::exportWorkStaPoint(){
    logPlain(tr("Begin to export work station points"));
    ix::Node* rl_point_list = m("xmate.project.point._list");
    ix::Node* point_node = xchip::workpointsNode();

    foreach (auto p_node,point_node->cmobjs()){
        QString name = p_node->c("name")->getString();
        QString desc = p_node->c("dec")->getString();
        int type = p_node->c("type")->getInt();
        if(type != 0 && type !=1){
            logError(tr("Point %1 type is invalid").arg(name));
            continue;
        }

        ix::Node* rl_node = getChildNode(name, rl_point_list);
        if(!rl_node){
            logInfo(tr("Insert a new point %1").arg(name));
            rl_node = rl_point_list->append(this);
            rl_node->copyFrom(this, m("xmate.project.point.default"));
        }

        rl_node->set(this, "name", name);
        rl_node->set(this, "description", desc);
        if(type ==0){
            rl_node->set(this, "property", "cart");
        }else{
            rl_node->set(this, "property", "joint");
        }

        auto pose_n = rl_node->r("cartesian.pose");
        pose_n->set(this,"elb",p_node->c("elb")->get());
        point2frame(this, p_node->c("pos"), pose_n->c("frame"));
        ix::Node tmp("tmp");
        tmp.copyFrom(this, p_node);
        xservice()->execCommandAt(this, "sys.calc.conf", &tmp);
        rl_node->set(this, "cartesian.conf.cf1", tmp.c(0)->getInt());
        rl_node->set(this, "cartesian.conf.cf2", tmp.c(1)->getInt());
        rl_node->set(this, "cartesian.conf.cf3", tmp.c(2)->getInt());
        rl_node->set(this, "cartesian.conf.cf4", tmp.c(3)->getInt());
        rl_node->set(this, "cartesian.conf.cf5", tmp.c(4)->getInt());
        rl_node->set(this, "cartesian.conf.cf6", tmp.c(5)->getInt());
        rl_node->set(this, "cartesian.conf.cf7", tmp.c(6)->getInt());
        rl_node->set(this, "cartesian.conf.cfx", tmp.c(7)->getInt());
        for (int i = 0; i < 7; i++) {
            rl_node->r("joint.inner_pos")->c(i)->set(this, p_node->c("pos")->c("near")->c(i)->getDouble() / M_PI * 180);
        }
        if(p_node->c("tool")->getString() == "s_tool"){
            xTool2Tool(this, xchip::frameNode()->r("tool.s_tool"), rl_node->r("coordinate.tool"));
        }

        if(p_node->c("wobj")->getString() == "s_boat1"){
            xWobj2Wobj(this,xchip::frameNode()->r("wobj.s_boat1"),rl_node->r("coordinate.wobj"));
        }

        if(p_node->c("wobj")->getString() == "s_boat2"){
            xWobj2Wobj(this,xchip::frameNode()->r("wobj.s_boat2"),rl_node->r("coordinate.wobj"));
        }

        if(p_node->c("tool")->getString() == "tool0"){
            rl_node->r("coordinate.tool")->copyFrom(this,m("xmate.project.tool.system_list")->c(0));
        }

        if(p_node->c("wobj")->getString() == "wobj0"){
            rl_node->r("coordinate.wobj")->copyFrom(this,m("xmate.project.wobj.system_list")->c(0));
        }

        rl_node->trigger();
        logInfo(tr("Refresh point %1").arg(name));
    }
    logPlain(tr("Export work station point finished"));
    xchip::syncstateNode()->c("workpoints")->set(this,false);
}

void SynchronizeRL::exportBoatPoint(){
    logPlain(tr("Begin to export boat points"));
    ix::Node* rl_point_list = m("xmate.project.point._list");
    ix::Node* point_node = xchip::zhoupointsNode();
    foreach (auto p_node,point_node->cmobjs()){
        QString name = "";
        if(m("imol.cfg.common.language")->getString() == "cn"){
            name = p_node->c("name")->getString();
        }else if(m("imol.cfg.common.language")->getString() == "en"){
            name = p_node->c("name_en")->getString();
        }

        QString desc = p_node->c("dec")->getString();
        int type = p_node->c("type")->getInt();

        if(type != 0 && type !=1){
            logError(tr("Point %1 type is invalid").arg(name));
            continue;
        }
        ix::Node* rl_node = getChildNode(name, rl_point_list);
        if(!rl_node){
            logInfo(tr("Insert a new point %1").arg(name));
            rl_node = rl_point_list->append(this);
            rl_node->copyFrom(this, m("xmate.project.point.default"));
        }

        rl_node->set(this, "name", name);
        rl_node->set(this, "description", desc);
        if(type ==0){
            rl_node->set(this, "property", "cart");
        }else{
            rl_node->set(this, "property", "joint");
        }

        auto pose_n = rl_node->r("cartesian.pose");
        pose_n->set(this,"elb",p_node->c("elb")->get());
        point2frame(this, p_node->c("pos"), pose_n->c("frame"));
        ix::Node tmp("tmp");
        tmp.copyFrom(this, p_node);
        xservice()->execCommandAt(this, "sys.calc.conf", &tmp);
        rl_node->set(this, "cartesian.conf.cf1", tmp.c(0)->getInt());
        rl_node->set(this, "cartesian.conf.cf2", tmp.c(1)->getInt());
        rl_node->set(this, "cartesian.conf.cf3", tmp.c(2)->getInt());
        rl_node->set(this, "cartesian.conf.cf4", tmp.c(3)->getInt());
        rl_node->set(this, "cartesian.conf.cf5", tmp.c(4)->getInt());
        rl_node->set(this, "cartesian.conf.cf6", tmp.c(5)->getInt());
        rl_node->set(this, "cartesian.conf.cf7", tmp.c(6)->getInt());
        rl_node->set(this, "cartesian.conf.cfx", tmp.c(7)->getInt());
        for (int i = 0; i < 7; i++) {
            rl_node->r("joint.inner_pos")->c(i)->set(this, p_node->c("pos")->c("near")->c(i)->getDouble() / M_PI * 180);
        }
        if(p_node->c("tool")->getString() == "s_tool"){
            xTool2Tool(this, xchip::frameNode()->r("tool.s_tool"), rl_node->r("coordinate.tool"));
        }

        if(p_node->c("wobj")->getString() == "s_boat1"){
            xWobj2Wobj(this,xchip::frameNode()->r("wobj.s_boat1"),rl_node->r("coordinate.wobj"));
        }

        if(p_node->c("wobj")->getString() == "s_boat2"){
            xWobj2Wobj(this,xchip::frameNode()->r("wobj.s_boat2"),rl_node->r("coordinate.wobj"));
        }

        rl_node->trigger();
        logInfo(tr("Refresh point %1").arg(name));
    }
    logPlain(tr("Export boat point finished"));
    xchip::syncstateNode()->c("zhoupoints")->set(this,false);
}

void SynchronizeRL::exportBacketPoint(){
    logPlain(tr("Begin to export basket points"));
    ix::Node* rl_point_list = m("xmate.project.point._list");
    ix::Node* point_node = xchip::hualanpointsNode();

    int point_num = 0;
    foreach (auto p_node,point_node->cmobjs()){
        if(!xchip::workstaNode()->c("hualanset")->c(point_num)->c("enable")->getBool()) {
            point_num++;
            continue;
        }
        QString name = "";
        if(m("imol.cfg.common.language")->getString() == "cn"){
            name = p_node->c("name")->getString();
        }else if(m("imol.cfg.common.language")->getString() == "en"){
            name = p_node->c("name_en")->getString();
        }
        QString desc = p_node->c("dec")->getString();
        auto type = p_node->c("type")->getInt();

        if(name == ""){
            point_num++;
            continue;
        }

        if(type != 0 && type != 1){
            logError(tr("Point %1 type is invalid").arg(name));
            point_num++;
            continue;
        }

        ix::Node* rl_node = getChildNode(name, rl_point_list);
        if(!rl_node){
            logInfo(tr("Insert a new point %1").arg(name));
            rl_node = rl_point_list->append(this);
            rl_node->copyFrom(this, m("xmate.project.point.default"));
        }
        rl_node->set(this, "name", name);
        rl_node->set(this, "description", desc);
        if(type == 0){
            rl_node->set(this, "property", "cart");
        }else if(type == 1){
            rl_node->set(this, "property", "joint");
        }


        ix::Node* node = xchip::hualanpointsNode()->c(point_num)->c("pos");
        auto pose_n = rl_node->r("cartesian.pose");
        pose_n->set(this, "elb", node->c("elb")->get());
        point2frame(this, node, pose_n->c("frame"));

        ix::Node tmp("tmp");
        tmp.copyFrom(this, node);
        xservice()->execCommandAt(this, "sys.calc.conf", &tmp);
        rl_node->set(this, "cartesian.conf.cf1", tmp.c(0)->getInt());
        rl_node->set(this, "cartesian.conf.cf2", tmp.c(1)->getInt());
        rl_node->set(this, "cartesian.conf.cf3", tmp.c(2)->getInt());
        rl_node->set(this, "cartesian.conf.cf4", tmp.c(3)->getInt());
        rl_node->set(this, "cartesian.conf.cf5", tmp.c(4)->getInt());
        rl_node->set(this, "cartesian.conf.cf6", tmp.c(5)->getInt());
        rl_node->set(this, "cartesian.conf.cf7", tmp.c(6)->getInt());
        rl_node->set(this, "cartesian.conf.cfx", tmp.c(7)->getInt());
        for (int i = 0; i < 7; i++) {
            rl_node->r("joint.inner_pos")->c(i)->set(this, node->c("pos")->c("near")->c(i)->getDouble() / M_PI * 180);
        }

        xTool2Tool(this, xchip::frameNode()->r("tool.s_tool"), rl_node->r("coordinate.tool"));

        rl_node->trigger();
        point_num++;
        logInfo(tr("Refresh point %1").arg(name));
    }
    logPlain(tr("Export basket point finished"));
    xchip::syncstateNode()->c("hualanpoints")->set(this,false);
}

void SynchronizeRL::exportFrame(){
    logPlain(tr("Begin to export tool"));
    ix::Node* tool_list_n = m("xmate.project.tool._list");
    ix::Node* tool_n = xchip::frameNode()->c("tool");
    foreach (auto tool, tool_n->cmobjs()) {
        ix::Node *new_tool_n = getChildNode(tool->name(), tool_list_n);
        if(!new_tool_n){
            new_tool_n = new ix::Node(ix::Node::generateId(tool_list_n));
            new_tool_n->copyFrom(this, tool_list_n->b("default"));
            tool_list_n->insert(this, new_tool_n);
            logInfo(tr("Insert a new tool %1").arg(tool->name()));
        }
        xTool2Tool(this, tool, new_tool_n);
        new_tool_n->trigger();
        logInfo(tr("Refresh tool %1").arg(tool->name()));
    }
    logPlain(tr("Export tool finished"));

    logPlain(tr("Begin to export wobj"));
    ix::Node* wobj_list_n = m("xmate.project.wobj._list");

    ix::Node* wobj_n = xchip::frameNode()->c("wobj");
    foreach (auto wobj, wobj_n->cmobjs()) {
        //(wobj->name() != "panel") continue; //just export panel wobj
        ix::Node *new_wobj_n = getChildNode(wobj->name(), wobj_list_n);
        if(!new_wobj_n){
            new_wobj_n = new ix::Node(ix::Node::generateId(wobj_list_n));
            new_wobj_n->copyFrom(this, wobj_list_n->b("default"));
            wobj_list_n->insert(this, new_wobj_n);
            logInfo(tr("Insert a new wobj %1").arg(wobj->name()));
        }
        new_wobj_n->set(this, "name", wobj->name());
        new_wobj_n->set(this, "description", wobj->c("desc")->getString());
        new_wobj_n->set(this, "robhold", false);
        new_wobj_n->set(this, "pers", false);
        new_wobj_n->set(this, "calibrated", true);
        new_wobj_n->set(this, "identified", true);
        point2frame(this, wobj, new_wobj_n->c("frame"));
        new_wobj_n->trigger();
        logInfo(tr("Refresh wobj %1").arg(wobj->name()));
    }
    logPlain(tr("Export wobj finished"));
    xchip::syncstateNode()->c("frame")->set(this,false);
}

void SynchronizeRL::logPlain(const QString &info)
{
    QApplication::processEvents();
    //ui->plainTextEdit->appendPlainText(info);
    ui->plainTextEdit->appendHtml(QString("<div style='color: black;'>%1</div>").arg(info));
}

void SynchronizeRL::logInfo(const QString &info)
{
    QApplication::processEvents();
    ui->plainTextEdit->appendHtml(QString("<div style='color: green;'>%1</div>").arg(info));
}

void SynchronizeRL::logError(const QString &info)
{
    ui->plainTextEdit->appendHtml(QString("<div style='color: red;'>%1</div>").arg(info));
}

bool SynchronizeRL::checkProject()
{
    QString cur_prj = m("xmate.project.cur_project")->getString().split("/").last();
    if(cur_prj.isEmpty()){
        helper().throwLog(this, "error", tr("Current project is empty, please reload a project."));
        return false;
    }
    QString last_prj = xchip::projectNode()->c("cur_project")->getString();
    if(!last_prj.isEmpty() && cur_prj != last_prj){
        if(!helper().execMsgOk("q", tr("The project has changed from %1 to %2, are you sure to export?").arg(last_prj).arg(cur_prj))) return false;
    }
    return true;
}

void SynchronizeRL::saveProject(bool auto_reload)
{
    xchip::projectNode()->set(this, "cur_project", ui->edtProject->text());
    xservice()->set(this, "solar.data", "project", xchip::projectNode());
    m("xmate.project.var.built")->set(this, false);
    m("xmate.project.point.built")->set(this, false);
    m("xmate.project.tool.built")->set(this, false);
    m("xmate.project.wobj.built")->set(this, false);
    m("xmate.project.io.built")->set(this, false);
    m("xmate.project.rsync.push.auto_reload")->set(this, auto_reload);
    m("xmate.project.rsync.build")->trigger();
}

void SynchronizeRL::refreshProject()
{
    QString cur_prj = m("xmate.project.cur_project")->getString().split("/").last();
    if(cur_prj.isEmpty()) cur_prj = tr("No Project");
    ui->edtProject->setText(cur_prj);
}


