#pragma once

#include <QQuickItem>
#include <QSGMaterial>
#include <QSGTexture>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <jni.h>
#include <vector>
#include <functional>

class VideoTexture : public QSGDynamicTexture
{
    Q_OBJECT
public:
    VideoTexture();
    ~VideoTexture();

    bool isActive() const;

    int textureId() const;
    QSize textureSize() const;
    void setTextureSize(const QSize &size);
    bool hasAlphaChannel() const;
    bool hasMipmaps() const;

    QRectF normalizedTextureSubRect() const;

    void bind();
    bool updateTexture();

    void invalidateTexture();

public slots:
    void releaseTexture();

private:
    QRectF m_subRect;
    QSize m_textureSize;
    GLuint m_textureId;
    bool m_updated;
};

class VideoMaterial : public QSGMaterial
{
public:
    VideoMaterial(VideoTexture *texture);

    QSGMaterialShader *createShader() const;
    QSGMaterialType *type() const;
    int compare(const QSGMaterial *other) const;

    void setTexture(VideoTexture *texture);

private:
    friend class VideoMaterialShader;
    friend class VideoNode;

    VideoTexture *m_texture;
};

class VideoNode : public QSGGeometryNode
{
public:
    VideoNode(VideoTexture *texture);
    ~VideoNode();

    void setBoundingRect(const QRectF &rect, int orientation, bool horizontalMirror, bool verticalMirror);
    void preprocess();

private:
    VideoMaterial m_material;
    QSGGeometry m_geometry;
};

class VideoSurface : public QQuickItem
{
    Q_OBJECT
public:
    explicit VideoSurface(QQuickItem *parent = 0);

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *paintNodeData) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void parseNALUs(std::function<void(uint8_t const*, size_t)> callback);

    bool m_isGeomertyDirty = true;
    jobject m_surfaceTexture = nullptr;
    std::vector<uint8_t> m_videoData;
    JNIEnv* m_env = nullptr;
};

