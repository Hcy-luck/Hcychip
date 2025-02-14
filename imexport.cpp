#include "imexport.h"
#include "ui_imexport.h"
#include "common/entry.h"
#include "framework/custom/xfiledialog.h"
#include "../xchip_helper.h"
#include "xservice.h"
#include <QLabel>

using namespace xchip;

IMOL_VER_REG("xchip.imexport", 3)
ImExport::ImExport(QWidget *parent_wgt,QObject *parent) :
    BaseSubPage(parent_wgt, "imexport",tr("ImExport"), parent),
    ui(new Ui::ImExport),
    m_result_mobj(new imol::ModuleObject("result_list"))
{
    ui->setupUi(wgt());

    ui->edtExport->setEnabled(false);
    ui->btnOpenIm->setProperty("func", "operation");
    ui->btnOpenEx->setProperty("func", "operation");

    m_import_map.insert("worksta",ui->chbset);
    m_import_map.insert("workpoints",ui->chbworkpos);
    m_import_map.insert("zhoupoints",ui->chbzhoupos);
    m_import_map.insert("hualanpoints",ui->chbbacket);
    m_import_map.insert("frame",ui->chbFrame);

    m_export_map.insert("worksta",ui->chbsetex);
    m_export_map.insert("workpoints",ui->chbworkposex);
    m_export_map.insert("zhoupoints",ui->chbzhouposex);
    m_export_map.insert("hualanpoints",ui->chbbacketex);
    m_export_map.insert("frame",ui->chbFrameex);

    connect(ui->btnOpenIm, &QPushButton::clicked, this, &ImExport::importOpen);
    connect(ui->btnOpenEx, &QPushButton::clicked, this, &ImExport::exportOpen);
    connect(ui->btnExport, &QPushButton::clicked, this, &ImExport::exportData);
    connect(ui->btnImport, &QPushButton::clicked, this, &ImExport::importData);
}

ImExport::~ImExport()
{
    delete ui;
}

void ImExport::initPage()
{
    foreach(auto chb, m_export_map.values()){
        chb->setChecked(true);
    }
}

void ImExport::importOpen()
{
    QString dir_path = m("xmate.help.file_path")->cmobj("import")->getString().isEmpty() ? QDir::current().path() : m("xmate.help.file_path")->cmobj("import")->getString();
    QString path = xcore::xFileDialog::getOpenFileName(nullptr, tr("Open XML file"), dir_path, tr("XML (*.xml)"));

    if (path.isEmpty()) return;
    m("xmate.help.file_path")->cmobj("import")->set(this, path.left(path.lastIndexOf("/") + 1));

    if(!path.contains(".xml")) path += ".xml";
    QString relative_path = QDir::current().relativeFilePath(path);
    ui->edtImport->setText(relative_path);

    //analyse import
    m_result_mobj->clear(this);
    m().readFromXmlFile(this, m_result_mobj, relative_path);

    foreach (auto chb, m_import_map.values()) {
        chb->setChecked(false);
    }
    //check
    if(m_result_mobj->c("file_id")->getString() != "chip_layout" || !m_result_mobj->hasCmobj("version")){
        helper().throwLog(this, "warning", tr("Invalid chip layout file!"));
        foreach (auto chb, m_import_map.values()) {
            chb->setEnabled(false);
        }
        return;
    }
    foreach (auto n, m_result_mobj->cmobjNames()) {
        if(m_import_map.value(n, nullptr)) m_import_map.value(n)->setChecked(true);
    }
    foreach (auto chb, m_import_map.values()) {
        if(!chb->isChecked()) chb->setEnabled(false);
        else chb->setEnabled(true);
    }
}

void ImExport::exportOpen()
{
    QString dir_path = m("xmate.help.file_path")->cmobj("export")->getString().isEmpty() ? QDir::current().path() : m("xmate.help.file_path")->cmobj("export")->getString();
    QString path = xcore::xFileDialog::getSaveFileName(nullptr, tr("Save XML file"), dir_path, tr("XML (*.xml)"), "chiplayout.xml");
    if(path.isEmpty()) return;
    m("xmate.help.file_path")->cmobj("export")->set(this, path.left(path.lastIndexOf("/") + 1));
    QString relative_path = QDir::current().relativeFilePath(path);
    ui->edtExport->setText(relative_path);
}

void ImExport::exportData()
{
    if(ui->edtExport->text().isEmpty()){
        helper().throwLog(this, "warning", tr("Export path is empty. Can not export file!"));
        return;
    }
    QString path = ui->edtExport->text();

    imol::ModuleObject temp_list_mobj("temp_list");
    temp_list_mobj.set(this, "file_id", "chip_layout");
    temp_list_mobj.set(this, "version", xchip::settingNode()->c("version")->getInt());

    int count = 0;
    QMap<QString, QCheckBox*>::const_iterator iter = m_export_map.constBegin();
    while (iter != m_export_map.constEnd()) {
        if(iter.value()->isChecked()){
            temp_list_mobj.append(this, iter.key())->copyFrom(this, xchip::settingNode()->c(iter.key()));
            count++;
        }
        ++iter;
    }
    if(count == 0){
        helper().throwLog(this, "warning", tr("please select item!"));
        return;
    }

    m().writeToXmlFile(path, &temp_list_mobj);

#ifdef PLATFORM_LINUX_ARM64
    system("sync");
    system("sync");
#endif
    helper().throwLog(this, "info", tr("Export success to %1!").arg(QFileInfo(path).baseName()));
}

void ImExport::importData()
{
    int count = 0;
    QMap<QString, QCheckBox*>::const_iterator iter = m_import_map.constBegin();
    while (iter != m_import_map.constEnd()) {
        if(iter.value()->isChecked()){
            xchip::settingNode()->c(iter.key())->copyFrom(this, m_result_mobj->c(iter.key()));
            count++;
        }
        ++iter;
    }

    if (count > 0){
        helper().throwLog(this, "info", tr("Import success!"));
        //sync
        //xservice()->set(this, "chip.data", xchip::settingNode());
    }else{
        helper().throwLog(this, "warning", tr("Nothing to import!"));
    }
}

