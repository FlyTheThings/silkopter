#pragma once

#include "IIntegral_Type_Template.h"
#include "All_Integral_Traits.h"

namespace ts
{

typedef IIntegral_Type_Template<UInt8_Type_Traits>   IUInt8_Type;
typedef IIntegral_Type_Template<Int8_Type_Traits>    IInt8_Type;
typedef IIntegral_Type_Template<UInt16_Type_Traits>  IUInt16_Type;
typedef IIntegral_Type_Template<Int16_Type_Traits>   IInt16_Type;
typedef IIntegral_Type_Template<UInt32_Type_Traits>  IUInt32_Type;
typedef IIntegral_Type_Template<Int32_Type_Traits>   IInt32_Type;
typedef IIntegral_Type_Template<Int64_Type_Traits>   IInt64_Type;

typedef IIntegral_Type_Template<Vec2u8_Type_Traits>  IVec2u8_Type;
typedef IIntegral_Type_Template<Vec2s8_Type_Traits>  IVec2s8_Type;
typedef IIntegral_Type_Template<Vec2u16_Type_Traits> IVec2u16_Type;
typedef IIntegral_Type_Template<Vec2s16_Type_Traits> IVec2s16_Type;
typedef IIntegral_Type_Template<Vec2u32_Type_Traits> IVec2u32_Type;
typedef IIntegral_Type_Template<Vec2s32_Type_Traits> IVec2s32_Type;

typedef IIntegral_Type_Template<Vec3u8_Type_Traits>  IVec3u8_Type;
typedef IIntegral_Type_Template<Vec3s8_Type_Traits>  IVec3s8_Type;
typedef IIntegral_Type_Template<Vec3u16_Type_Traits> IVec3u16_Type;
typedef IIntegral_Type_Template<Vec3s16_Type_Traits> IVec3s16_Type;
typedef IIntegral_Type_Template<Vec3u32_Type_Traits> IVec3u32_Type;
typedef IIntegral_Type_Template<Vec3s32_Type_Traits> IVec3s32_Type;

typedef IIntegral_Type_Template<Vec4u8_Type_Traits>  IVec4u8_Type;
typedef IIntegral_Type_Template<Vec4s8_Type_Traits>  IVec4s8_Type;
typedef IIntegral_Type_Template<Vec4u16_Type_Traits> IVec4u16_Type;
typedef IIntegral_Type_Template<Vec4s16_Type_Traits> IVec4s16_Type;
typedef IIntegral_Type_Template<Vec4u32_Type_Traits> IVec4u32_Type;
typedef IIntegral_Type_Template<Vec4s32_Type_Traits> IVec4s32_Type;

}