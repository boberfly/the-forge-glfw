#pragma once
#ifndef RHI_ENUMS_H_
#define RHI_ENUMS_H_

typedef enum RHI_Result
{
	RHI_RESULT_SUCCESS = 0,
	RHI_RESULT_ERROR = -1,
} RHI_Result;

typedef enum RHI_RendererApi
{
	RHI_RENDERER_API_D3D12 = 0,
	RHI_RENDERER_API_VULKAN,
	RHI_RENDERER_API_METAL,
	RHI_RENDERER_API_XBOX_D3D12,
	RHI_RENDERER_API_D3D11,
	RHI_RENDERER_API_ORBIS
} RHI_RendererApi;

typedef enum RHI_MaxEnums
{
	RHI_MAX_INSTANCE_EXTENSIONS = 64,
	RHI_MAX_DEVICE_EXTENSIONS = 64,
	RHI_MAX_GPUS = 10,
	RHI_MAX_RENDER_TARGET_ATTACHMENTS = 8,
	RHI_MAX_SUBMIT_CMDS = 20,    // max number of command lists / command buffers
	RHI_MAX_SUBMIT_WAIT_SEMAPHORES = 8,
	RHI_MAX_SUBMIT_SIGNAL_SEMAPHORES = 8,
	RHI_MAX_PRESENT_WAIT_SEMAPHORES = 8,
	RHI_MAX_VERTEX_BINDINGS = 15,
	RHI_MAX_VERTEX_ATTRIBS = 15,
	RHI_MAX_SEMANTIC_NAME_LENGTH = 128,
	RHI_MAX_DEBUG_NAME_LENGTH = 128,
	RHI_MAX_MIP_LEVELS = 0xFFFFFFFF,
	RHI_MAX_SWAPCHAIN_IMAGES = 3,
	RHI_MAX_ROOT_CONSTANTS_PER_ROOTSIGNATURE = 4,
	RHI_MAX_GPU_VENDOR_STRING_LENGTH = 64    //max size for GPUVendorPreset strings
} RHI_MaxEnums;

typedef enum RHI_ShaderTarget
{
	RHI_SHADER_TARGET_5_1,
	RHI_SHADER_TARGET_6_0,
	RHI_SHADER_TARGET_6_1,
	RHI_SHADER_TARGET_6_2,
	RHI_SHADER_TARGET_6_3, //required for Raytracing
} RHI_ShaderTarget;

typedef enum RHI_GpuMode
{
	RHI_GPU_MODE_SINGLE = 0,
	RHI_GPU_MODE_LINKED,
	// #TODO GPU_MODE_UNLINKED,
} RHI_GpuMode;


typedef enum RHI_D3DFeatureLevel {
	RHI_D3D_FL_9_1,
	RHI_D3D_FL_1_0_CORE,
	RHI_D3D_FL_9_2,
	RHI_D3D_FL_9_3,
	RHI_D3D_FL_10_0,
	RHI_D3D_FL_10_1,
	RHI_D3D_FL_11_0,
	RHI_D3D_FL_11_1,
	RHI_D3D_FL_12_0,
	RHI_D3D_FL_12_1
} RHI_D3DFeatureLevel;

typedef enum RHI_QueueType
{
	RHI_QUEUE_TYPE_GRAPHICS = 0,
	RHI_QUEUE_TYPE_TRANSFER,
	RHI_QUEUE_TYPE_COMPUTE
} RHI_QueueType;

typedef enum RHI_QueueFlag
{
	RHI_QUEUE_FLAG_NONE = 0x0,
	RHI_QUEUE_FLAG_DISABLE_GPU_TIMEOUT = 0x1,
	RHI_QUEUE_FLAG_INIT_MICROPROFILE = 0x2,
} RHI_QueueFlag;

typedef enum RHI_QueuePriority
{
	RHI_QUEUE_PRIORITY_NORMAL,
	RHI_QUEUE_PRIORITY_HIGH,
	RHI_QUEUE_PRIORITY_GLOBAL_REALTIME,
} RHI_QueuePriority;

typedef enum RHI_BufferCreationFlags
{
	/// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
	RHI_BUFFER_CREATION_FLAG_NONE = 0x01,
	/// Buffer will allocate its own memory (COMMITTED resource)
	RHI_BUFFER_CREATION_FLAG_OWN_MEMORY_BIT = 0x02,
	/// Buffer will be persistently mapped
	RHI_BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT = 0x04,
	/// Use ESRAM to store this buffer
	RHI_BUFFER_CREATION_FLAG_ESRAM = 0x08,
	/// Flag to specify not to allocate descriptors for the resource
	RHI_BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION = 0x10,

	/// Metal-only
	/* ICB Flags */
	/// Ihnerit pipeline in ICB
	RHI_BUFFER_CREATION_FLAG_ICB_INHERIT_PIPELINE = 0x100,
	/// Ihnerit pipeline in ICB
	RHI_BUFFER_CREATION_FLAG_ICB_INHERIT_BUFFERS = 0x200,

} RHI_BufferCreationFlags;

typedef enum RHI_TextureCreationFlags
{
	/// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
	RHI_TEXTURE_CREATION_FLAG_NONE = 0,
	/// Texture will allocate its own memory (COMMITTED resource)
	RHI_TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT = 0x01,
	/// Texture will be allocated in memory which can be shared among multiple processes
	RHI_TEXTURE_CREATION_FLAG_EXPORT_BIT = 0x02,
	/// Texture will be allocated in memory which can be shared among multiple gpus
	RHI_TEXTURE_CREATION_FLAG_EXPORT_ADAPTER_BIT = 0x04,
	/// Texture will be imported from a handle created in another process
	RHI_TEXTURE_CREATION_FLAG_IMPORT_BIT = 0x08,
	/// Use ESRAM to store this texture
	RHI_TEXTURE_CREATION_FLAG_ESRAM = 0x10,
	/// Use on-tile memory to store this texture
	RHI_TEXTURE_CREATION_FLAG_ON_TILE = 0x20,
	/// Prevent compression meta data from generating (XBox)
	RHI_TEXTURE_CREATION_FLAG_NO_COMPRESSION = 0x40,
	/// Force 2D instead of automatically determining dimension based on width, height, depth
	RHI_TEXTURE_CREATION_FLAG_FORCE_2D = 0x80,
	/// Force 3D instead of automatically determining dimension based on width, height, depth
	RHI_TEXTURE_CREATION_FLAG_FORCE_3D = 0x100,
	/// Display target
	RHI_TEXTURE_CREATION_FLAG_ALLOW_DISPLAY_TARGET = 0x200,
	/// Create an sRGB texture.
	RHI_TEXTURE_CREATION_FLAG_SRGB = 0x400,
} RHI_TextureCreationFlags;

typedef enum RHI_SampleCount
{
	RHI_SAMPLE_COUNT_1 = 1,
	RHI_SAMPLE_COUNT_2 = 2,
	RHI_SAMPLE_COUNT_4 = 4,
	RHI_SAMPLE_COUNT_8 = 8,
	RHI_SAMPLE_COUNT_16 = 16,
} RHI_SampleCount;

typedef enum RHI_DescriptorType
{
	RHI_DESCRIPTOR_TYPE_UNDEFINED = 0,
	RHI_DESCRIPTOR_TYPE_SAMPLER = 0x01,
	// SRV Read only texture
	RHI_DESCRIPTOR_TYPE_TEXTURE = (RHI_DESCRIPTOR_TYPE_SAMPLER << 1),
	/// UAV Texture
	RHI_DESCRIPTOR_TYPE_RW_TEXTURE = (RHI_DESCRIPTOR_TYPE_TEXTURE << 1),
	// SRV Read only buffer
	RHI_DESCRIPTOR_TYPE_BUFFER = (RHI_DESCRIPTOR_TYPE_RW_TEXTURE << 1),
	RHI_DESCRIPTOR_TYPE_BUFFER_RAW = (RHI_DESCRIPTOR_TYPE_BUFFER | (RHI_DESCRIPTOR_TYPE_BUFFER << 1)),
	/// UAV Buffer
	RHI_DESCRIPTOR_TYPE_RW_BUFFER = (RHI_DESCRIPTOR_TYPE_BUFFER << 2),
	RHI_DESCRIPTOR_TYPE_RW_BUFFER_RAW = (RHI_DESCRIPTOR_TYPE_RW_BUFFER | (RHI_DESCRIPTOR_TYPE_RW_BUFFER << 1)),
	/// Uniform buffer
	RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER = (RHI_DESCRIPTOR_TYPE_RW_BUFFER << 2),
	/// Push constant / Root constant
	RHI_DESCRIPTOR_TYPE_ROOT_CONSTANT = (RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER << 1),
	/// IA
	RHI_DESCRIPTOR_TYPE_VERTEX_BUFFER = (RHI_DESCRIPTOR_TYPE_ROOT_CONSTANT << 1),
	RHI_DESCRIPTOR_TYPE_INDEX_BUFFER = (RHI_DESCRIPTOR_TYPE_VERTEX_BUFFER << 1),
	RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER = (RHI_DESCRIPTOR_TYPE_INDEX_BUFFER << 1),
	/// Cubemap SRV
	RHI_DESCRIPTOR_TYPE_TEXTURE_CUBE = (RHI_DESCRIPTOR_TYPE_TEXTURE | (RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER << 1)),
	/// RTV / DSV per mip slice
	RHI_DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES = (RHI_DESCRIPTOR_TYPE_INDIRECT_BUFFER << 2),
	/// RTV / DSV per array slice
	RHI_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES << 1),
	/// RTV / DSV per depth slice
	RHI_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
	RHI_DESCRIPTOR_TYPE_RAY_TRACING = (RHI_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),

	/// Subpass input (descriptor type only available in Vulkan)
	RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = (RHI_DESCRIPTOR_TYPE_RAY_TRACING << 1),
	RHI_DESCRIPTOR_TYPE_TEXEL_BUFFER = (RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT << 1),
	RHI_DESCRIPTOR_TYPE_RW_TEXEL_BUFFER = (RHI_DESCRIPTOR_TYPE_TEXEL_BUFFER << 1),

	/// Metal-only
	RHI_DESCRIPTOR_TYPE_ARGUMENT_BUFFER = (RHI_DESCRIPTOR_TYPE_RAY_TRACING << 1),
	RHI_DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER = (RHI_DESCRIPTOR_TYPE_ARGUMENT_BUFFER << 1),
	RHI_DESCRIPTOR_TYPE_RENDER_PIPELINE_STATE = (RHI_DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER << 1),

} RHI_DescriptorType;

typedef enum RHI_BlendConstant
{
	RHI_BC_ZERO = 0,
	RHI_BC_ONE,
	RHI_BC_SRC_COLOR,
	RHI_BC_ONE_MINUS_SRC_COLOR,
	RHI_BC_DST_COLOR,
	RHI_BC_ONE_MINUS_DST_COLOR,
	RHI_BC_SRC_ALPHA,
	RHI_BC_ONE_MINUS_SRC_ALPHA,
	RHI_BC_DST_ALPHA,
	RHI_BC_ONE_MINUS_DST_ALPHA,
	RHI_BC_SRC_ALPHA_SATURATE,
	RHI_BC_BLEND_FACTOR,
	RHI_BC_ONE_MINUS_BLEND_FACTOR,
} RHI_BlendConstant;

typedef enum RHI_BlendMode
{
	RHI_BM_ADD,
	RHI_BM_SUBTRACT,
	RHI_BM_REVERSE_SUBTRACT,
	RHI_BM_MIN,
	RHI_BM_MAX,
} RHI_BlendMode;

typedef enum RHI_CompareMode
{
	RHI_CMP_NEVER,
	RHI_CMP_LESS,
	RHI_CMP_EQUAL,
	RHI_CMP_LEQUAL,
	RHI_CMP_GREATER,
	RHI_CMP_NOTEQUAL,
	RHI_CMP_GEQUAL,
	RHI_CMP_ALWAYS,
} RHI_CompareMode;

typedef enum RHI_StencilOp
{
	RHI_STENCIL_OP_KEEP,
	RHI_STENCIL_OP_SET_ZERO,
	RHI_STENCIL_OP_REPLACE,
	RHI_STENCIL_OP_INVERT,
	RHI_STENCIL_OP_INCR,
	RHI_STENCIL_OP_DECR,
	RHI_STENCIL_OP_INCR_SAT,
	RHI_STENCIL_OP_DECR_SAT,
} RHI_StencilOp;

typedef enum RHI_ShaderStage
{
	RHI_SHADER_STAGE_NONE = 0,
	RHI_SHADER_STAGE_VERT = 0X00000001,
	RHI_SHADER_STAGE_TESC = 0X00000002,
	RHI_SHADER_STAGE_TESE = 0X00000004,
	RHI_SHADER_STAGE_GEOM = 0X00000008,
	RHI_SHADER_STAGE_FRAG = 0X00000010,
	RHI_SHADER_STAGE_COMP = 0X00000020,
	RHI_SHADER_STAGE_RAYTRACING  = 0X00000040,
	RHI_SHADER_STAGE_ALL_GRAPHICS = ((uint32_t)RHI_SHADER_STAGE_VERT | (uint32_t)RHI_SHADER_STAGE_TESC | (uint32_t)RHI_SHADER_STAGE_TESE | (uint32_t)RHI_SHADER_STAGE_GEOM | (uint32_t)RHI_SHADER_STAGE_FRAG),
	RHI_SHADER_STAGE_HULL = RHI_SHADER_STAGE_TESC,
	RHI_SHADER_STAGE_DOMN = RHI_SHADER_STAGE_TESE,
	RHI_SHADER_STAGE_COUNT = 7,
} RHI_ShaderStage;

typedef enum RHI_BlendStateTargets
{
	RHI_BLEND_STATE_TARGET_0 = 0x1,
	RHI_BLEND_STATE_TARGET_1 = 0x2,
	RHI_BLEND_STATE_TARGET_2 = 0x4,
	RHI_BLEND_STATE_TARGET_3 = 0x8,
	RHI_BLEND_STATE_TARGET_4 = 0x10,
	RHI_BLEND_STATE_TARGET_5 = 0x20,
	RHI_BLEND_STATE_TARGET_6 = 0x40,
	RHI_BLEND_STATE_TARGET_7 = 0x80,
	RHI_BLEND_STATE_TARGET_ALL = 0xFF,
} RHI_BlendStateTargets;

typedef enum RHI_CullMode
{
	RHI_CULL_MODE_NONE = 0,
	RHI_CULL_MODE_BACK,
	RHI_CULL_MODE_FRONT,
	RHI_CULL_MODE_BOTH,
} RHI_CullMode;

typedef enum RHI_FrontFace
{
	RHI_FRONT_FACE_CCW = 0,
	RHI_FRONT_FACE_CW
} RHI_FrontFace;

typedef enum RHI_FillMode
{
	RHI_FILL_MODE_SOLID,
	RHI_FILL_MODE_WIREFRAME,
} RHI_FillMode;

typedef enum RHI_PipelineType
{
	RHI_PIPELINE_TYPE_UNDEFINED = 0,
	RHI_PIPELINE_TYPE_COMPUTE,
	RHI_PIPELINE_TYPE_GRAPHICS,
	RHI_PIPELINE_TYPE_RAYTRACING,
} RHI_PipelineType;

typedef enum RHI_FilterType
{
	RHI_FILTER_NEAREST = 0,
	RHI_FILTER_LINEAR,
} RHI_FilterType;

typedef enum RHI_AddressMode
{
	RHI_ADDRESS_MODE_MIRROR,
	RHI_ADDRESS_MODE_REPEAT,
	RHI_ADDRESS_MODE_CLAMP_TO_EDGE,
	RHI_ADDRESS_MODE_CLAMP_TO_BORDER
} RHI_AddressMode;

typedef enum RHI_MipMapMode
{
	RHI_MIPMAP_MODE_NEAREST = 0,
	RHI_MIPMAP_MODE_LINEAR
} RHI_MipMapMode;

typedef enum RHI_DepthStencilClearFlags
{
	RHI_CLEAR_DEPTH = 0x01,
	RHI_CLEAR_STENCIL = 0x02
} RHI_DepthStencilClearFlags;

typedef enum RHI_RootSignatureFlags
{
	RHI_ROOT_SIGNATURE_FLAG_NONE = 0,
	RHI_ROOT_SIGNATURE_FLAG_LOCAL_BIT = 0x1,
} RHI_RootSignatureFlags;

typedef enum RHI_VertexAttribRate
{
	RHI_VERTEX_ATTRIB_RATE_VERTEX = 0,
	RHI_VERTEX_ATTRIB_RATE_INSTANCE = 1,
} RHI_VertexAttribRate;

typedef enum RHI_ShaderSemantic
{
	RHI_SEMANTIC_UNDEFINED = 0,
	RHI_SEMANTIC_POSITION,
	RHI_SEMANTIC_NORMAL,
	RHI_SEMANTIC_COLOR,
	RHI_SEMANTIC_TANGENT,
	RHI_SEMANTIC_BITANGENT,
	RHI_SEMANTIC_TEXCOORD0,
	RHI_SEMANTIC_TEXCOORD1,
	RHI_SEMANTIC_TEXCOORD2,
	RHI_SEMANTIC_TEXCOORD3,
	RHI_SEMANTIC_TEXCOORD4,
	RHI_SEMANTIC_TEXCOORD5,
	RHI_SEMANTIC_TEXCOORD6,
	RHI_SEMANTIC_TEXCOORD7,
	RHI_SEMANTIC_TEXCOORD8,
	RHI_SEMANTIC_TEXCOORD9,
} RHI_ShaderSemantic;

typedef enum RHI_IndexType
{
	RHI_INDEX_TYPE_UINT32 = 0,
	RHI_INDEX_TYPE_UINT16,
} RHI_IndexType;

typedef enum RHI_PrimitiveTopology
{
	RHI_PRIMITIVE_TOPO_POINT_LIST = 0,
	RHI_PRIMITIVE_TOPO_LINE_LIST,
	RHI_PRIMITIVE_TOPO_LINE_STRIP,
	RHI_PRIMITIVE_TOPO_TRI_LIST,
	RHI_PRIMITIVE_TOPO_TRI_STRIP,
	RHI_PRIMITIVE_TOPO_PATCH_LIST,
} RHI_PrimitiveTopology;

typedef enum RHI_LoadActionType
{
	RHI_LOAD_ACTION_DONTCARE,
	RHI_LOAD_ACTION_LOAD,
	RHI_LOAD_ACTION_CLEAR,
} RHI_LoadActionType;

typedef enum RHI_ResourceState
{
	RHI_RESOURCE_STATE_UNDEFINED = 0,
	RHI_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
	RHI_RESOURCE_STATE_INDEX_BUFFER = 0x2,
	RHI_RESOURCE_STATE_RENDER_TARGET = 0x4,
	RHI_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
	RHI_RESOURCE_STATE_DEPTH_WRITE = 0x10,
	RHI_RESOURCE_STATE_DEPTH_READ = 0x20,
	RHI_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
	RHI_RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
	RHI_RESOURCE_STATE_STREAM_OUT = 0x100,
	RHI_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
	RHI_RESOURCE_STATE_COPY_DEST = 0x400,
	RHI_RESOURCE_STATE_COPY_SOURCE = 0x800,
	RHI_RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	RHI_RESOURCE_STATE_PRESENT = 0x1000,
	RHI_RESOURCE_STATE_COMMON = 0x2000,
} RHI_ResourceState;

typedef enum RHI_ResourceMemoryUsage
{
	RHI_RESOURCE_MEMORY_USAGE_UNKNOWN = 0,
	RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY = 1,
	RHI_RESOURCE_MEMORY_USAGE_CPU_ONLY = 2,
	RHI_RESOURCE_MEMORY_USAGE_CPU_TO_GPU = 3,
	RHI_RESOURCE_MEMORY_USAGE_GPU_TO_CPU = 4,
} RHI_ResourceMemoryUsage;

typedef enum RHI_FenceStatus
{
	RHI_FENCE_STATUS_COMPLETE = 0,
	RHI_FENCE_STATUS_INCOMPLETE,
	RHI_FENCE_STATUS_NOTSUBMITTED,
} RHI_FenceStatus;

typedef enum RHI_IndirectArgumentType
{
	RHI_INDIRECT_DRAW,
	RHI_INDIRECT_DRAW_INDEX,
	RHI_INDIRECT_DISPATCH,
	RHI_INDIRECT_VERTEX_BUFFER,
	RHI_INDIRECT_INDEX_BUFFER,
	RHI_INDIRECT_CONSTANT,
	RHI_INDIRECT_DESCRIPTOR_TABLE,        // only for vulkan
	RHI_INDIRECT_PIPELINE,                // only for vulkan now, probally will add to dx when it comes to xbox
	RHI_INDIRECT_CONSTANT_BUFFER_VIEW,    // only for dx
	RHI_INDIRECT_SHADER_RESOURCE_VIEW,    // only for dx
	RHI_INDIRECT_UNORDERED_ACCESS_VIEW,   // only for dx
	RHI_INDIRECT_COMMAND_BUFFER,          // metal ICB
	RHI_INDIRECT_COMMAND_BUFFER_OPTIMIZE  // metal indirect buffer optimization
} RHI_IndirectArgumentType;

typedef enum RHI_QueryType
{
	RHI_QUERY_TYPE_TIMESTAMP = 0,
	RHI_QUERY_TYPE_PIPELINE_STATISTICS,
	RHI_QUERY_TYPE_OCCLUSION,
	RHI_QUERY_TYPE_COUNT,
} RHI_QueryType;

typedef enum RHI_DescriptorUpdateFrequency {
	RHI_DESCRIPTOR_UPDATE_FREQ_NONE = 0,
	RHI_DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
	RHI_DESCRIPTOR_UPDATE_FREQ_PER_BATCH,
	RHI_DESCRIPTOR_UPDATE_FREQ_PER_DRAW,
	RHI_DESCRIPTOR_UPDATE_FREQ_COUNT,
} RHI_DescriptorUpdateFrequency;

#endif // RHI_ENUMS_H_