#ifndef __CCMATHUTIL_H__
#define __CCMATHUTIL_H__

#include "cocos2d.h"

NS_CC_MATH_BEGIN

/**
 * Yawの算出
 */
inline float makeYaw(Vec3 v) noexcept {
    v.y = 0.0f;
    v.normalize();
    return CC_RADIANS_TO_DEGREES( atan2f(v.x, v.z) );
}

inline float makeYaw(const Vec3& from, const Vec3& to) noexcept {
    return makeYaw(to - from);
}

inline float makeYaw(const Node* from, const Node* to) noexcept {
    return makeYaw(to->getPosition3D() - from->getPosition3D());
}

/**
 * Pitchの算出
 */
inline float makePitch(Vec3 v) noexcept {
    v.normalize();
    return CC_RADIANS_TO_DEGREES( atan2f( Vec2(v.x, v.z).length(), v.y ) - M_PI_2 );
}

/**
 * QuaternionからVec3(degree)を取得
 */
Vec3 getRotationFromQuaternion(const Quaternion& q) noexcept;

/**
 * AABBの内接円を取得
 */
void getInnerSphere(Vec3& outCenter, float& outRadius, const AABB& aabb) noexcept;

/**
 * AABBの外接円を取得
 */
void getOuterSphere(Vec3& outCenter, float& outRadius, const AABB& aabb) noexcept;

/**
 * 衝突判定
 */
namespace intersect {
    
    /**
     * 線分と平面の衝突位置を算出
     * @param out 接触位置
     * @param v0 始点
     * @param v1 終点
     */
    bool segmentPlane(Vec3& out, const Vec3& v0, const Vec3& v1, const Plane& plane) noexcept;
    
    /**
     * 線分と球の衝突位置を算出
     * @param out 接触位置
     * @param v0 始点
     * @param v1 終点
     */
    bool segmentSphere(Vec3& out, const Vec3& v0, const Vec3& v1, const Vec3& sphereCenter, float sphereRadius) noexcept;
    
    /**
     * 線分のAABBの衝突を算出
     * @param v0 始点
     * @param v1 終点
     */
    bool segmentAABB(const Vec3& v0, const Vec3& v1, const AABB& aabb) noexcept;
}

NS_CC_MATH_END

#endif
