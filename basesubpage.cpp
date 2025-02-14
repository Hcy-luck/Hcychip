#include "basesubpage.h"
#include <QWidget>

BaseSubPage::BaseSubPage(QWidget *parent_wgt, const QString &name, const QString &label, QObject *parent) :
    xcore::BasePage(name, label, parent), m_wgt(new QWidget(parent_wgt)){}

BaseSubPage::~BaseSubPage()
{
    delete m_wgt;
}

QWidget* BaseSubPage::wgt()
{
    return m_wgt;
}

void BaseSubPage::initPage()
{

}

void BaseSubPage::deinitPage()
{

}

void BaseSubPage::changeVisible(const bool visible)//对常驻内存的页面，切换页面时触发
{

}

void BaseSubPage::setAvailable(bool is_available)
{
    if(m_wgt) m_wgt->setEnabled(is_available);
}
