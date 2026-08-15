#include "StyleLoader.h"
#include <mutex>
#include <QFile>

std::unique_ptr<CStyleLoader> CStyleLoader::m_instance = nullptr;

CStyleLoader::~CStyleLoader()
{

}

CStyleLoader *CStyleLoader::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&](){
        m_instance.reset(new CStyleLoader());
    });

    return m_instance.get();
}

void CStyleLoader::LoadStyle(const QString &strFilePath, QWidget *w)
{
    QFile file(strFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ;
    }

    QString strQss = QString::fromUtf8(file.readAll().data());
    //设置这个样式表
    w->setStyleSheet(strQss);
}

CStyleLoader::CStyleLoader() {}
