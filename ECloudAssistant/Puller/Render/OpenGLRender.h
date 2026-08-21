#ifndef OPENGLRENDER_H
#define OPENGLRENDER_H
#include <QLabel>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include "AVCommon.h"
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLPixelTransferOptions>
#include "define.h"

class COpenGLRender : public QOpenGLWidget, protected  QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit COpenGLRender(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    COpenGLRender(const COpenGLRender&) = delete;
    COpenGLRender& operator=(const COpenGLRender&) = delete;
    virtual ~COpenGLRender();
public:
    virtual void Repaint(AVFramePtr frame);
    void GetPosRation(MouseMove_Body& body);
protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
private:
    void repaintTexYUV420P(AVFramePtr frame);
    void initTexYUV420P(AVFramePtr frame);
    void freeTexYUV420P();
private:
    QLabel* m_pLabel = nullptr;

    QOpenGLTexture* m_pTexY = nullptr;
    QOpenGLTexture* m_pTexU = nullptr;
    QOpenGLTexture* m_pTexV = nullptr;
    QOpenGLShaderProgram* m_pProgram = nullptr;
    QOpenGLPixelTransferOptions m_options;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;
    QSize   m_size;
    QSizeF  m_zoomSize;
    QRect   m_rect;
    QPointF m_pos;
};

#endif // OPENGLRENDER_H
