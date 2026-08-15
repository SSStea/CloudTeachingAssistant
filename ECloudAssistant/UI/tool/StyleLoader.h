#ifndef STYLELOADER_H
#define STYLELOADER_H
#include <memory>
#include <QString>
#include <QWidget>

class CStyleLoader
{
public:
    ~CStyleLoader();
    static CStyleLoader* GetInstance();
    void LoadStyle(const QString& strFilePath, QWidget* w);

private:
    CStyleLoader();
    static std::unique_ptr<CStyleLoader> m_instance;
};

#endif // STYLELOADER_H
