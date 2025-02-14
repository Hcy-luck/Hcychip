#ifndef SYNCHRONIZERL_H
#define SYNCHRONIZERL_H

#include "basesubpage.h"
#include "xnode.h"

namespace Ui {
class SynchronizeRL;
}

class SynchronizeRL : public BaseSubPage
{
    Q_OBJECT

public:
    explicit SynchronizeRL(QWidget *parent_wgt, QObject *parent = nullptr);
    ~SynchronizeRL();
    void initPage() override;

private slots:
    void exportToProject();

private:
    void logPlain(const QString& info);
    void logInfo(const QString& info);
    void logError(const QString& info);
    void exportWorkStaPoint();
    void exportBoatPoint();
    void exportBacketPoint();
    void exportFrame();

    bool checkProject();
    void saveProject(bool auto_reload = true);
    void refreshProject();

private:
    Ui::SynchronizeRL *ui;
    ix::Node *m_pointdesc_mobj;
    ix::Node *m_pointpos_mobj;
};

#endif // SYNCHRONIZERL_H
