#ifndef BASESUBPAGE_H
#define BASESUBPAGE_H

#include "framework/basepage.h"

class QWidget;

class BaseSubPage : public xcore::BasePage
{
public:
    explicit BaseSubPage(QWidget *parent_wgt, const QString &name, const QString &label, QObject *parent = nullptr);
    ~BaseSubPage();

    QWidget* wgt() override;
    virtual void initPage();
    virtual void deinitPage();

    virtual void changeVisible(const bool visible = true);
    virtual void setAvailable(bool is_available);

private:
    QWidget* m_wgt;
};


#endif // BASESUBPAGE_H
