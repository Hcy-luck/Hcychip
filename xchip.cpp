#include "xchip.h"
#include <QDebug>

#include "bwaf/bfactory.h"
#include "bwaf/bmodule.h"
#include "bwaf/centerwidget.h"

#include "core/translationmanager.h"
#include "common/entry.h"
#include "xhelper.h"
#include "chippage.h"

#define XChip_VERSION_MAIN 1
#define XChip_VERSION_RELEASE 0
#define XChip_VERSION_NUM 3

#define XChip_API 1

namespace xchip {

int api()
{
    return XChip_API;
}

QString version()//?
{
    return mName(QString::number(XChip_VERSION_MAIN),
                 QString::number(XChip_VERSION_RELEASE),
                 QString::number(ver(MODULE_NAME, true) - XChip_VERSION_NUM));
}

}

class XChip : public bwaf::BModule
{
public:
    explicit XChip(QObject *parent) : bwaf::BModule(parent)
    {
        SLOG << "XChip v" << xchip::version();
        translator().installQMTranslator(ix::languagePath("XChip_cn"));
        helper().registQrcAlias(MODULE_NAME);
        bindMobjJson(MODULE_NAME, ix::templatePath(MODULE_NAME));
        mobj()->set(this, "version", xchip::version());
    }
    QString name() const { return MODULE_NAME; }

private:
    void setup()
    {
        bwaf::CenterWidget *center_wgt = instantiatedCenterWidget(this);
        xchip::ChipPage *chip_page = new xchip::ChipPage(center_wgt,this);
        this->addSubmodule(chip_page);
        center_wgt->addPage(this, "XChip.page", chip_page->wgt());
    }

    void init()//在菜单栏中注册
    {
        if(!m("imol.lic.debug")->getBool() && setting()->mobj()->r("function.chip")->getString("deactivate") != "activate") return;
        ix::xnav::registerPage(this, QObject::tr("Chip Pakage"), "robot.pack")->set(this, "nav", "XChip.page")
                ->set(this, "key", "xchippackage");
    }

};

IMOL_PLUGIN("XChip", XChip)//center_wgt->addPage(this, "XChip.page", solar_page->wgt())必须和这里写法一样
