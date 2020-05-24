#pragma once
#ifndef RHI_SHADERREFLECTION_H_
#define RHI_SHADERREFLECTION_H_

static const uint32_t RHI_MAX_SHADER_STAGE_COUNT = 5;

typedef enum RHI_TextureDimension
{
	RHI_TEXTURE_DIM_1D,
	RHI_TEXTURE_DIM_2D,
	RHI_TEXTURE_DIM_2DMS,
	RHI_TEXTURE_DIM_3D,
	RHI_TEXTURE_DIM_CUBE,
	RHI_TEXTURE_DIM_1D_ARRAY,
	RHI_TEXTURE_DIM_2D_ARRAY,
	RHI_TEXTURE_DIM_2DMS_ARRAY,
	RHI_TEXTURE_DIM_CUBE_ARRAY,
	RHI_TEXTURE_DIM_COUNT,
	RHI_TEXTURE_DIM_UNDEFINED,
} RHI_TextureDimension;

typedef struct RHI_VertexInput
{
	// The size of the attribute
	uint32_t size;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;
} RHI_VertexInput;

typedef struct RHI_ShaderResource
{
	// resource Type
	RHI_DescriptorType type;

	// The resource set for binding frequency
	uint32_t set;

	// The resource binding location
	uint32_t reg;

	// The size of the resource. This will be the DescriptorInfo array size for textures
	uint32_t size;

	// what stages use this resource
	RHI_ShaderStage used_stages;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;

	// 1D / 2D / Array / MSAA / ...
	RHI_TextureDimension dim;

	union {
		struct {
			uint32_t mtlTextureType;           // Needed to bind different types of textures as default resources on Metal.
			uint32_t mtlArgumentBufferType;    // Needed to bind multiple resources under a same descriptor on Metal.
		};
		uint32_t dx11Constant_size; 					 // dx11
	};

} RHI_ShaderResource;

typedef struct RHI_ShaderVariable
{
	// parents resource index
	uint32_t parent_index;

	// The offset of the Variable.
	uint32_t offset;

	// The size of the Variable.
	uint32_t size;

	// Variable name
	const char* name;

	// name size
	uint32_t name_size;
} RHI_ShaderVariable;

typedef struct RHI_ShaderReflection
{
	RHI_ShaderStage mShaderStage;

	// single large allocation for names to reduce number of allocations
	char*    pNamePool;
	uint32_t mNamePoolSize;

	RHI_VertexInput* pVertexInputs;
	uint32_t     mVertexInputsCount;

	RHI_ShaderResource* pShaderResources;
	uint32_t        mShaderResourceCount;

	RHI_ShaderVariable* pVariables;
	uint32_t        mVariableCount;

	// Thread group size for compute shader
	uint32_t mNumThreadsPerGroup[3];

	//number of tessellation control point
	uint32_t mNumControlPoint;

	// vulkan only
	char* pEntryPoint;

} RHI_ShaderReflection;

typedef struct RHI_PipelineReflection
{
	RHI_ShaderStage mShaderStages;
	// the individual stages reflection data.
	RHI_ShaderReflection mStageReflections[RHI_MAX_SHADER_STAGE_COUNT];
	uint32_t mStageReflectionCount;

	uint32_t mVertexStageIndex;
	uint32_t mHullStageIndex;
	uint32_t mDomainStageIndex;
	uint32_t mGeometryStageIndex;
	uint32_t mPixelStageIndex;

	RHI_ShaderResource* pShaderResources;
	uint32_t mShaderResourceCount;

	RHI_ShaderVariable* pVariables;
	uint32_t mVariableCount;
}
RHI_PipelineReflection;

#endif
