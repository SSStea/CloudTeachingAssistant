#include "OpenGLRender.h"
#include <QShowEvent>
#include <QMovie>
//添加着色器

static GLfloat vertices[] = {
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
};

//索引
static GLuint indices[] ={
    0,1,3,
    1,2,3
};

COpenGLRender::COpenGLRender(QWidget *parent, Qt::WindowFlags f)
    :QOpenGLWidget(parent, f)
{
    m_pos = QPoint(0, 0);
    m_zoomSize = QSize(0, 0);
    m_rect.setRect(0, 0, 0, 0);
    this->setMouseTracking(true);
    this->resize(parent->size());
    this->setMinimumSize(400, 250);
    m_pLabel = new QLabel(this);
    m_pLabel->resize(parent->size());
}

COpenGLRender::~COpenGLRender()
{
    //释放OpenGl上下文
    if(!isValid())
    {
        return ;
    }
    //获取上下文
    this->makeCurrent();
    //释放纹理
    freeTexYUV420P();
    //释放上下文
    this->doneCurrent();
    //释放VBO，EBO，VAO
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void COpenGLRender::Repaint(AVFramePtr frame)
{
    //重绘视频数据
    if(!frame || frame->width == 0 || frame->height == 0)
    {
        return ;
    }
    //开始绘制时结束loading.gif
    if(m_pLabel)
    {
        m_pLabel->hide();
        delete m_pLabel;
        m_pLabel = nullptr;
    }

    //更新yuv纹理
    repaintTexYUV420P(frame);
    //调用paintGl绘制
    this->update();//update会自动调用paintGl
}

void COpenGLRender::GetPosRation(MouseMove_Body &body)
{

}

void COpenGLRender::showEvent(QShowEvent *event)
{
    QMovie* movie = new QMovie(":/UI/brown/center/loading.gif");
    if(m_pLabel)
    {
        m_pLabel->setMovie(movie);
        movie->start();
        m_pLabel->show();
    }

    //COpenGLRender::showEvent(event);
}

void COpenGLRender::initializeGL()
{
    //初始化
    initializeOpenGLFunctions();

    //加载脚本 顶点着色器，片段着色器
    m_pProgram = new QOpenGLShaderProgram();
    m_pProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/UI/brown/vertex.vsh");
    m_pProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/UI/brown/fragment.fsh");
    //链接
    m_pProgram->link();

    //绑定yuv变量值
    m_pProgram->bind();
    m_pProgram->setUniformValue("tex_y", 0);
    m_pProgram->setUniformValue("tex_u", 1);
    m_pProgram->setUniformValue("tex_v", 2);

    //赋值坐标和纹理
    GLuint posArrt = GLuint(m_pProgram->attributeLocation("aPos"));
    GLuint texCord = GLuint(m_pProgram->attributeLocation("aTexCord"));

    //创建VAO
    glGenVertexArrays(1, &m_VAO);
    //绑定VAO
    glBindVertexArray(m_VAO);

    //创建VBO
    glGenBuffers(1, &m_VBO);
    //绑定VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    //创建一个新的数据存储
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//准备一个数组
    //将顶点索引传入EBO缓存
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //数值顶点坐标
    glVertexAttribPointer(posArrt, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    //启用顶点数组
    glEnableVertexAttribArray(posArrt);
    //设置纹理坐标
    glVertexAttribPointer(texCord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          reinterpret_cast<const GLvoid*>(3 * sizeof(GLfloat)));
    //启用纹理
    glEnableVertexAttribArray(texCord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //清空窗口颜色
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void COpenGLRender::resizeGL(int w, int h)
{
    if(m_size.width() < 0 || m_size.height() < 0)
    {
        return ;
    }

    //计算显示图片窗口大小，实现长宽等比缩放
    if((double(w) / h) < (double(m_size.width()) / m_size.height()))
    {
        //更新大小
        m_zoomSize.setWidth(w);
        m_zoomSize.setHeight(((double(w) / m_size.width()) * m_size.height()));
    }
    else
    {
        m_zoomSize.setHeight(h);
        m_zoomSize.setWidth(((double(h) / m_size.height()) * m_size.width()));
    }

    //更新位置pos
    m_pos.setX(double(w - m_zoomSize.width()) / 2);
    m_pos.setY(double(h - m_zoomSize.height()) / 2);
    m_rect.setRect(m_pos.x(), m_pos.y(), m_zoomSize.width(), m_zoomSize.height());

    //更新宽高
    this->update(QRect(0, 0, w, h));
    if(m_pLabel)
    {
        m_pLabel->resize(w, h);
    }
}

void COpenGLRender::paintGL()
{
    //重绘之前清空上一次颜色
    glClear(GL_COLOR_BUFFER_BIT);

    //更新视图
    glViewport(m_pos.x(), m_pos.y(), m_zoomSize.width(), m_zoomSize.height());
    //绑定着色器，开始渲染
    m_pProgram->bind();

    //绑定纹理
    if(m_pTexY && m_pTexU && m_pTexV)
    {
        m_pTexY->bind(0);
        m_pTexU->bind(1);
        m_pTexV->bind(2);
    }

    //绑定VAO
    glBindVertexArray(m_VAO);

    //绘制
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    //释放纹理
    if(m_pTexY && m_pTexU && m_pTexV)
    {
        m_pTexY->release();
        m_pTexU->release();
        m_pTexV->release();
    }
    //释放着色器程序
    m_pProgram->release();
}

void COpenGLRender::repaintTexYUV420P(AVFramePtr frame)
{
    //更新YUV
    if(frame->width != m_size.width() || frame->height != m_size.height())
    {
        //释放yuv，重新初始化
        freeTexYUV420P();
    }
    //重新初始化
    initTexYUV420P(frame);

    //传值
    m_options.setImageHeight(frame->height);
    m_options.setRowLength(frame->linesize[0]);//步长
    //设置图片数据
    m_pTexY->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, static_cast<const void*>(frame->data[0]),
                     &m_options);
    m_options.setRowLength(frame->linesize[1]);
    m_pTexU->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, static_cast<const void*>(frame->data[1]),
                     &m_options);
    m_options.setRowLength(frame->linesize[2]);
    m_pTexV->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, static_cast<const void*>(frame->data[2]),
                     &m_options);
}

void COpenGLRender::initTexYUV420P(AVFramePtr frame)
{
    //初始化yuv420p
    //初始化yuv纹理
    if(!m_pTexY)
    {
        m_pTexY = new QOpenGLTexture(QOpenGLTexture::Target2D);
        //大小
        m_pTexY->setSize(frame->width, frame->height);
        //纹理属性
        m_pTexY->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
        m_pTexY->setFormat(QOpenGLTexture::R8_UNorm);
        m_pTexY->allocateStorage();
        //更新宽高
        m_size.setWidth(frame->width);
        m_size.setHeight(frame->height);
        //重置宽高
        this->resizeGL(this->width(),this->height());
    }
    if(!m_pTexU)
    {
        m_pTexU = new QOpenGLTexture(QOpenGLTexture::Target2D);
        //大小
        m_pTexU->setSize(frame->width / 2, frame->height / 2);
        //纹理属性
        m_pTexU->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
        m_pTexU->setFormat(QOpenGLTexture::R8_UNorm);
        m_pTexU->allocateStorage();
    }
    if(!m_pTexV)
    {
        m_pTexV = new QOpenGLTexture(QOpenGLTexture::Target2D);
        //大小
        m_pTexV->setSize(frame->width / 2, frame->height / 2);
        //纹理属性
        m_pTexV->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
        m_pTexV->setFormat(QOpenGLTexture::R8_UNorm);
        m_pTexV->allocateStorage();
    }
}

void COpenGLRender::freeTexYUV420P()
{
    //释放资源
    if(m_pTexY)
    {
        m_pTexY->destroy();
        delete m_pTexY;
        m_pTexY = nullptr;
    }
    if(m_pTexU)
    {
        m_pTexU->destroy();
        delete m_pTexU;
        m_pTexU = nullptr;
    }
    if(m_pTexV)
    {
        m_pTexV->destroy();
        delete m_pTexV;
        m_pTexV = nullptr;
    }
}
