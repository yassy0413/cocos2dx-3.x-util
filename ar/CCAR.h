/****************************************************************************
 Copyright (c) Yassy
 https://github.com/yassy0413/cocos2dx-3.x-util
 ****************************************************************************/
#ifndef __CCAR_H__
#define __CCAR_H__

#include <cocos2d.h>
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

/**
 * Augmented Reality
 * @required iOS ARKit.framework
 */
class AR
{
public:
    CC_DISALLOW_COPY_AND_ASSIGN(AR);
    
    /** Return the shared instance **/
    static AR *getInstance();
    
    /** Relase the shared instance **/
    static void destroyInstance();
    
    /// 機能が有効か確認
    bool isSupported() const;
    
    /// 開始
    void start();
    
    /// 終了
    void stop();
    
    ///
    void pause();
    
    ///
    void resume();
    
    /// 平面 検出通知
    std::function<void(const char* key, const Mat4& transform)> onAddPlane;
    /// 平面 更新通知
    std::function<void(const char* key, const Mat4& transform)> onUpdatePlane;
    /// 平面 消失通知
    std::function<void(const char* key)> onRemovePlane;
    
    /// デバイスカメラの回転を取得
    const Quaternion& getQuat() const { return _quat; }
    Quaternion& getQuat(){ return _quat; }
    
    /// デバイスカメラの起動時からの相対位置を取得
    const Vec3& getPosition() const { return _position; }
    Vec3& getPosition(){ return _position; }
    
    /// 撮影中の映像をテクスチャとして取得する
    Texture2D* getTexture();
    
    /// 撮影映像をテクスチャとして持つスプライトを生成する
    Sprite* createSprite(const std::function<void(Sprite* sender)>& onCreated = nullptr);
    
    
    virtual void update(float delta);
    void applyImage(const void* data, int32_t width, int32_t height);
    
private:
    AR();
    virtual ~AR();
    
    void clearSprites();
    void applySprite(Sprite* sprite);
    
    void* _internal;
    experimental::RenderTarget* _renderTarget;
    std::vector<std::pair<Sprite*, std::function<void(Sprite* sender)>>> _sprites;
    Quaternion _quat;
    Vec3 _position;
};

NS_CC_EXT_END

#endif
