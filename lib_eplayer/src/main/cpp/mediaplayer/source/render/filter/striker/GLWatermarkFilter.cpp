//
// Created by Guns Roses on 2020/5/4.
//
#include "GLWatermarkFilter.h"

// 顶点坐标
static const float vertices_default1[] = {
        -1.0f, -1.0f,  // left,  bottom,代表物体的左下角对应OpenGL的坐标值，这个值也是OpenGL坐标系的左下角
        1.0f, -1.0f,  // right, bottom
        -1.0f, 1.0f,  // left,  top
        1.0f, 1.0f,  // right, top

//        -1.0f, -0.5f,  // left,  bottom,代表物体的左下角对应OpenGL的坐标值，这个值也是OpenGL坐标系的左下角
//        1.0f, -0.5f,  // right, bottom
//        -1.0f, 0.5f,  // left,  top
//        1.0f, 0.5f,  // right, top
};

// 水印纹理坐标
const static GLfloat WATERMARK_COORD[] = {
        0.0f, 1.0f, // 左下
        1.0f, 1.0f, // 右下
        0.0f, 0.0f, // 左上，原点
        1.0f, 0.0f, // 右上
};

GLWatermarkFilter::GLWatermarkFilter() {
}

void GLWatermarkFilter::initProgram() {
    initProgram(WatermarkVertexShader.c_str(), WatermarkFragmentShader.c_str());
}

void GLWatermarkFilter::initProgram(const char *vertexShader, const char *fragmentShader) {
    GLFilter::initProgram(vertexShader, fragmentShader);
    // 使用链接好的程序
    glUseProgram(programHandle);
    if (isInitialized()) {
        mWatermarkTexCoord = glGetAttribLocation(programHandle, "watermarkTextureCoord");
        // 水印的着色器纹理句柄
        inputTextureHandle[1] = glGetUniformLocation(programHandle, "watermarkTexture");
        // 水印纹理坐标矩阵
        mMatrixHandle = glGetUniformLocation(programHandle, "mWatermarkMatrix");

        // 水印纹理对象
        glGenTextures(1, &mWatermarkTexture);
        glBindTexture(GL_TEXTURE_2D, mWatermarkTexture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // 上载水印纹理对象的数据
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWatermarkWidth, mWatermarkHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, mWatermarkPixel);
    }
}

void GLWatermarkFilter::setWatermark(uint8_t *watermarkPixel, size_t length, GLint width, GLint height, GLfloat scale,
                                     GLint location) {

    mWatermarkWidth = width;
    mWatermarkHeight = height;
    if (!mWatermarkPixel) {
        // 首先分配内存空间
        mWatermarkPixel = new uint8_t[length];
    }
    memcpy(mWatermarkPixel, watermarkPixel, length);
    // 计算水印的缩放矩阵
    // location的意义：0左上，1左下，2右上，3右下
    if (location==0){
        v_mat4 = glm::translate(v_mat4, glm::vec3(-0.0f, -0.0f, 0.0f));
    } else if(location==1){
        v_mat4 = glm::translate(v_mat4, glm::vec3(-0.0f, -scale+1, 0.0f));
    } else if (location==2){
        v_mat4 = glm::translate(v_mat4, glm::vec3(-scale+1, -0.0f, 0.0f));
    } else{
        v_mat4 = glm::translate(v_mat4, glm::vec3(-scale+1, -scale+1, 0.0f));
    }
    //v_mat4 = glm::translate(v_mat4, glm::vec3(-0.0f, -5.5f, 0.0f));
    v_mat4 = glm::scale(v_mat4, glm::vec3(scale, scale, 0.0f));
}

void GLWatermarkFilter::bindTexture(GLuint texture) {
    GLFilter::bindTexture(texture);
    // 激活纹理单元1
    glActiveTexture(GL_TEXTURE1);
    // 将纹理对象绑定纹理单元1
    glBindTexture(GL_TEXTURE_2D, mWatermarkTexture);
    // 将纹理单元1跟着色器句柄关联
    glUniform1i(inputTextureHandle[1], 1);
}

void GLWatermarkFilter::bindAttributes(const float *vertices, const float *textureVertices) {
    GLFilter::bindAttributes(vertices, textureVertices);
    // 水印的纹理坐标
    glEnableVertexAttribArray(mWatermarkTexCoord);
    glVertexAttribPointer(mWatermarkTexCoord, 2, GL_FLOAT, GL_FALSE, 0, WATERMARK_COORD);
}

void GLWatermarkFilter::unbindAttributes() {
    GLFilter::unbindAttributes();
    glDisableVertexAttribArray(mWatermarkTexCoord);
}

void GLWatermarkFilter::onDrawBegin() {
    GLFilter::onDrawBegin();
    // 设置矩阵的值
    glUniformMatrix4fv(mMatrixHandle, 1, GL_FALSE, glm::value_ptr(v_mat4));
}
