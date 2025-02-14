#ifndef IMEXPORT_H
#define IMEXPORT_H

#include "basesubpage.h"
#include "xnode.h"
#include <QMap>

namespace Ui {
class ImExport;
}

class QCheckBox;

class ImExport : public BaseSubPage
{
    Q_OBJECT

public:
    explicit ImExport(QWidget *parent_wgt, QObject *parent = nullptr);
    ~ImExport();
    void initPage() override;

private slots:
    void importOpen();
    void exportOpen();
    void exportData();
    void importData();

private:
    Ui::ImExport *ui;
//    ix::Node *m_pointdesc_mobj;
//    ix::Node *m_pointpos_mobj;
    QMap<QString, QCheckBox*> m_import_map;
    QMap<QString, QCheckBox*> m_export_map;
    imol::ModuleObject *m_result_mobj;
};

#endif // IMEXPORT_H
