///////////////////////////////////////////////////////////////////////////
//
//  Autogenerated by bindgen.py
//
///////////////////////////////////////////////////////////////////////////


#pragma once
#ifndef RHI_RAY_ENUMS_H_
#define RHI_RAY_ENUMS_H_

typedef enum RHI_AccelerationStructureBuildFlags
{
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE = 0,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE = 0x1,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION = 0x2,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE = 0x4,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD = 0x8,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY = 0x10,
    RHI_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE = 0x20
} RHI_AccelerationStructureBuildFlags;

typedef enum RHI_AccelerationStructureGeometryFlags
{
    RHI_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NONE = 0,
    RHI_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE = 0x1,
    RHI_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2
} RHI_AccelerationStructureGeometryFlags;

typedef enum RHI_AccelerationStructureInstanceFlags
{
    RHI_ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE = 0,
    RHI_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
    RHI_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
    RHI_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
    RHI_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8
} RHI_AccelerationStructureInstanceFlags;



#endif // RHI_RAY_ENUMS_H_
