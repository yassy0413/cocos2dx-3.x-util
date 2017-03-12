#include "CCMathUtil.h"

NS_CC_MATH_BEGIN

Vec3 getRotationFromQuaternion(const Quaternion& q) noexcept {
    Vec3 r;
    
    float x = q.x, y = q.y, z = q.z, w = q.w;
    r.x = atan2f(2.f * (w * x + y * z), 1.f - 2.f * (x * x + y * y));
    r.y = asinf(2.f * (w * y - z * x));
    r.z = atan2f(2.f * (w * z + x * y), 1.f - 2.f * (y * y + z * z));
    
    r.x = CC_RADIANS_TO_DEGREES(r.x);
    r.y = CC_RADIANS_TO_DEGREES(r.y);
    r.z = -CC_RADIANS_TO_DEGREES(r.z);
    
    return r;
}

void getInnerSphere(Vec3& outCenter, float& outRadius, const AABB& aabb) noexcept {
    const Vec3 edgeSize(aabb._max - aabb._min);
    outRadius = std::min(std::min(edgeSize.x, edgeSize.y), edgeSize.z) * 0.5f;
    outCenter = (aabb._max + aabb._min) * 0.5f;
}

void getOuterSphere(Vec3& outCenter, float& outRadius, const AABB& aabb) noexcept {
    outRadius = aabb._max.distance(aabb._min) * 0.5f;
    outCenter = (aabb._max + aabb._min) * 0.5f;
}

namespace intersect {
    
    bool segmentPlane(Vec3& out, const Vec3& v0, const Vec3& v1, const Plane& plane) noexcept {
        const Vec3 v( v1 - v0 );
        const float d = plane.getNormal().dot( v );
        
        if( d == 0.0f )
        {// 平面と線分は平行
            return false;
        }
        
        const float t = ( plane.getDist() - plane.getNormal().dot( v0 ) ) / d;
        out = v0 + v * t;
        return true;
    }
    
    bool segmentSphere(Vec3& out, const Vec3& v0, const Vec3& v1, const Vec3& sphereCenter, float sphereRadius) noexcept {
        const Vec3& p = v0;
        const Vec3 d = (v1 - v0).getNormalized();
        
        const Vec3 m = p - sphereCenter;
        const float b = m.dot( d );
        const float c = m.lengthSquared() - sphereRadius * sphereRadius;
        
        if( b > 0.0f && c > 0.0f ){
            return false;
        }
        
        const float discr = b*b - c;
        if( discr < 0.0f ){
            return false;
        }
        
        float t = -b - sqrtf(discr);
        if( t < 0.0f ){
            t = 0.0f;
        }
        
        out = p + d * t;
        return true;
    }
    
    /// 線分の両端点がバウンディングボックス(軸並行)に内包されているか判定
    inline bool segmentPointsToAABB(const Vec3& v0, const Vec3& v1, const AABB& aabb) noexcept {
        return
        ((aabb._min.x <= v0.x) || (v0.x <= aabb._max.x)) &&
        ((aabb._min.y <= v0.y) || (v0.y <= aabb._max.y)) &&
        ((aabb._min.z <= v0.z) || (v0.z <= aabb._max.z)) &&
        ((aabb._min.x <= v1.x) || (v1.x <= aabb._max.x)) &&
        ((aabb._min.y <= v1.y) || (v1.y <= aabb._max.y)) &&
        ((aabb._min.z <= v1.z) || (v1.z <= aabb._max.z)) ;
    }
    
    bool segmentAABB(const Vec3& v0, const Vec3& v1, const AABB& aabb) noexcept {
        const Vec3& boxmin = aabb._min;
        const Vec3& boxmax = aabb._max;
        
        const Vec3& rayQ = v0;
        const Vec3  rayV = (v1 - v0).getNormalized();
        
        // Ｘ軸と垂直な平面について判定
        if( rayV.x != 0.0f ){
            const float t = ((rayV.x > 0.0f)? (boxmin.x - rayQ.x) : (boxmax.x - rayQ.x)) / rayV.x;
            
            const Vec3 point = rayQ + rayV * t;
            
            if(((boxmin.y <= point.y) || (point.y <= boxmax.y)) &&
               ((boxmin.z <= point.z) || (point.z <= boxmax.z)) )
            {
                if( segmentPointsToAABB(v0, v1, aabb) ){
                    return true;
                }
            }
        }
        
        // Ｙ軸と垂直な平面について判定
        if( rayV.y != 0.0f ){
            const float t = ((rayV.y > 0.0f)? (boxmin.y - rayQ.y) : (boxmax.y - rayQ.y)) / rayV.y;
            
            const Vec3 point = rayQ + rayV * t;
            
            if(((boxmin.x <= point.x ) || ( point.x <= boxmax.x)) &&
               ((boxmin.z <= point.z ) || ( point.z <= boxmax.z)) )
            {
                if( segmentPointsToAABB(v0, v1, aabb) ){
                    return true;
                }
            }
        }
        
        // Ｚ軸と垂直な平面について判定
        if( rayV.z != 0.0f ){
            const float t = ((rayV.z > 0.0f)? (boxmin.z - rayQ.z) : (boxmax.z - rayQ.z)) / rayV.z;
            
            const Vec3 point = rayQ + rayV * t;
            
            if(((boxmin.x <= point.x ) || (point.x <= boxmax.x)) &&
               ((boxmin.y <= point.y ) || (point.y <= boxmax.y)) )
            {
                if( segmentPointsToAABB(v0, v1, aabb) ){
                    return true;
                }
            }
        }
        
        return false;
    }
}

NS_CC_MATH_END
