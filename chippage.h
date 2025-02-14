#ifndef CHIPPAGE_H
#define CHIPPAGE_H

#include <QWidget>
#include "page/basesubpage.h"
#include "ui_chippage.h"
#include "xnode.h"
#include <functional>

namespace Ui {
class ChipPage;
}

class QToolButton;

namespace xchip {
class ChipPage : public xcore::BasePage{
    Q_OBJECT

public:
    explicit ChipPage(QWidget *parent_wgt, QObject *parent = nullptr);
    ~ChipPage(){delete ui;}

    QWidget* wgt() override {return m_wgt;}

    void init() override;

protected:
    virtual void render() override;

private:
    void updateAvailability() override;
    void loadFromRC();

private slots:
    void switchPage();

private:
    struct SubPageCfg
    {
        typedef std::function<BaseSubPage*()> CreatePageFunc;

        QString m_title;
        QString m_icon;
        CreatePageFunc m_func;
        QToolButton* m_tbtn;
        bool m_is_resident;
        BaseSubPage* m_sub_page;
        SubPageCfg(const QString& title, const QString& icon, CreatePageFunc func,bool is_resident = false) :
            m_title(title), m_icon(icon), m_func(func), m_tbtn(nullptr),m_is_resident(is_resident),m_sub_page(nullptr){}
    };

private:
    Ui::ChipPage *ui;
    QWidget *m_wgt;
    QList<QWidget *> m_render_exception_wgts;
    QList<SubPageCfg> m_pageList;
    QToolButton* m_curr_tb;
    BaseSubPage* m_curr_page;
    imol::ModuleObject *m_result_mobj;
    ix::Node *m_sync_state;
    QStringList m_sync_name;
};
}

#endif // CHIPPAGE_H
