#pragma once
#ifndef GFX_RHI_STRUCTS_H_
#define GFX_RHI_STRUCTS_H_

#include "enums.h"
#include "imageformat.h"
#include <OS/Interfaces/IOperatingSystem.h>

typedef struct RHI_Renderer *RHI_RendererHandle;
typedef struct RHI_Fence *RHI_FenceHandle;
typedef struct RHI_Semaphore *RHI_SemaphoreHandle;
typedef struct RHI_Queue *RHI_QueueHandle;
typedef struct RHI_CmdPool *RHI_CmdPoolHandle;
typedef struct RHI_Cmd *RHI_CmdHandle;
typedef struct RHI_RenderTarget *RHI_RenderTargetHandle;
typedef struct RHI_Sampler *RHI_SamplerHandle;
typedef struct RHI_Shader *RHI_ShaderHandle;
typedef struct RHI_RootSignature *RHI_RootSignatureHandle;
typedef struct RHI_Pipeline *RHI_PipelineHandle;
typedef struct RHI_Raytracing *RHI_RaytracingHandle;
typedef struct RHI_RaytracingHitGroup *RHI_RaytracingHitGroupHandle;
typedef struct RHI_DescriptorInfo *RHI_DescriptorInfoHandle;
typedef struct RHI_DescriptorSet *RHI_DescriptorSetHandle;
typedef struct RHI_Buffer *RHI_BufferHandle;

typedef struct RHI_Texture *RHI_TextureHandle;
typedef struct RHI_AcclerationStructure *RHI_AcclerationStructureHandle;
typedef struct RHI_QueryPool *RHI_QueryPoolHandle;
typedef struct RHI_CommandSignature *RHI_CommandSignatureHandle;
typedef struct RHI_SwapChain *RHI_SwapChainHandle;

typedef struct RHI_ClearValue {
	union {
		struct {
			float r;
			float g;
			float b;
			float a;
		};
		struct {
			float depth;
			uint32_t stencil;
		};
	};
} RHI_ClearValue;

typedef struct RHI_RendererDesc {
	RHI_ShaderTarget shaderTarget;
	RHI_GpuMode gpuMode;

	union {
		RHI_D3DFeatureLevel d3dFeatureLevel;
	};
} RHI_RendererDesc;

typedef struct RHI_QueueDesc {
	RHI_QueueType type;
	RHI_QueueFlag flags;
	RHI_QueuePriority priority;
	uint32_t nodeIndex;
} RHI_QueueDesc;

typedef struct RHI_QueueSubmitDesc
{
	uint32_t cmdCount;
	RHI_CmdHandle *pCmds;
	RHI_FenceHandle signalFence;
	uint32_t waitSemaphoreCount;
	RHI_SemaphoreHandle *pWaitSemaphores;
	uint32_t signalSemaphoreCount;
	RHI_SemaphoreHandle *pSignalSemaphores;
	bool submitDone;
} RHI_QueueSubmitDesc;

typedef struct RHI_QueuePresentDesc
{
	RHI_SwapChainHandle swapChain;
	uint32_t waitSemaphoreCount;
	RHI_SemaphoreHandle *pWaitSemaphores;
	uint8_t index;
	bool submitDone;
} RHI_QueuePresentDesc;

typedef struct RHI_RenderTargetDesc {
	/// Texture creation flags (decides memory allocation strategy, sharing access,...)
	RHI_TextureCreationFlags flags;
	/// Width
	uint32_t width;
	/// Height
	uint32_t height;
	/// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
	uint32_t depth;
	/// Texture array size (Should be 1 if texture is not a texture array or cubemap)
	uint32_t arraySize;
	/// Number of mip levels
	uint32_t mipLevels;
	/// MSAA
	RHI_SampleCount sampleCount;
	/// image format
	RHI_ImageFormat format;
	/// Optimized clear value (recommended to use this same value when clearing the rendertarget)
	RHI_ClearValue clearValue;
	/// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
	uint32_t sampleQuality;
	/// Descriptor creation
	RHI_DescriptorType descriptors;
	const void* pNativeHandle;
	/// Debug name used in gpu profile
	char const *debugName;
	/// GPU indices to share this texture
	uint32_t* sharedNodeIndices;
	/// Number of GPUs to share this texture
	uint32_t sharedNodeIndexCount;
	/// GPU which will own this texture
	uint32_t nodeIndex;

} RHI_RenderTargetDesc;

typedef struct RHI_ShaderMacro
{
	const char* definition;
	const char* value;
} RHI_ShaderMacro;

typedef struct RHI_ShaderStageDesc {
	char const *name;
	char const *code;
	char const *entryPoint;
	RHI_ShaderMacro *macros;
	uint32_t macroCount;
} RHI_ShaderStageDesc;

typedef struct RHI_ShaderDesc {
	RHI_ShaderStage stages;
	RHI_ShaderStageDesc vert;
	RHI_ShaderStageDesc frag;
	RHI_ShaderStageDesc geom;
	RHI_ShaderStageDesc hull;
	RHI_ShaderStageDesc domain;
	RHI_ShaderStageDesc comp;
} RHI_ShaderDesc;

typedef struct RHI_BinaryShaderStageDesc {
	/// Byte code array
	const char *byteCode;
	uint32_t byteCodeSize;
	const char *entryPoint;

	// Shader source is needed for reflection on Metal only
#if defined(METAL)
	char *source;
	uint32_t sourceSize;
#endif
} RHI_BinaryShaderStageDesc;

typedef struct RHI_BinaryShaderDesc {
	RHI_ShaderStage stages;
	RHI_BinaryShaderStageDesc vert;
	RHI_BinaryShaderStageDesc frag;
	RHI_BinaryShaderStageDesc geom;
	RHI_BinaryShaderStageDesc hull;
	RHI_BinaryShaderStageDesc domain;
	RHI_BinaryShaderStageDesc comp;
} RHI_BinaryShaderDesc;

typedef struct RHI_SamplerDesc {
	RHI_FilterType minFilter;
	RHI_FilterType magFilter;
	RHI_MipMapMode mipMapMode;
	RHI_AddressMode addressU;
	RHI_AddressMode addressV;
	RHI_AddressMode addressW;
	float mipLodBias;
	float maxAnisotropy;
	RHI_CompareMode compareFunc;
} RHI_SamplerDesc;

typedef struct RHI_RootSignatureDesc {
	RHI_ShaderHandle *pShaders;
	uint32_t shaderCount;
	uint32_t maxBindlessTextures;
	const char **ppStaticSamplerNames;
	RHI_SamplerHandle *pStaticSamplers;
	uint32_t staticSamplerCount;
	RHI_RootSignatureFlags flags;
} RHI_RootSignatureDesc;

typedef struct RHI_VertexAttrib {
	RHI_ShaderSemantic semantic;
	uint32_t semanticNameLength;
	char semanticName[RHI_MAX_SEMANTIC_NAME_LENGTH];
	RHI_ImageFormat format;
	uint32_t binding;
	uint32_t location;
	uint32_t offset;
	RHI_VertexAttribRate rate;
} RHI_VertexAttrib;

typedef struct RHI_VertexLayout {
	uint32_t attribCount;
	RHI_VertexAttrib attribs[RHI_MAX_VERTEX_ATTRIBS];
} RHI_VertexLayout;

typedef struct RHI_RaytracingPipelineDesc {
	RHI_RaytracingHandle raytracing;
	RHI_RootSignatureHandle globalRootSignature;
	RHI_ShaderHandle rayGenShader;
	RHI_RootSignatureHandle rayGenRootSignature;
	RHI_ShaderHandle *pMissShaders;
	RHI_RootSignatureHandle *pMissRootSignatures;
	RHI_RaytracingHitGroupHandle hitGroups;
	RHI_RootSignatureHandle emptyRootSignature;
	unsigned missShaderCount;
	unsigned hitGroupCount;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned payloadSize;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned attributeSize;
	unsigned maxTraceRecursionDepth;
	unsigned maxRaysCount;
} RHI_RaytracingPipelineDesc;

typedef struct RHI_BlendStateDesc {
	RHI_BlendConstant srcFactors[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendConstant dstFactors[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendConstant srcAlphaFactors[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendConstant dstAlphaFactors[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendMode blendModes[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendMode blendAlphaModes[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	uint32_t masks[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_BlendStateTargets renderTargetMask;
	bool alphaToCoverage;
	bool independentBlend;
} RHI_BlendStateDesc;

typedef struct RHI_DepthStateDesc {
	bool depthTest;
	bool depthWrite;

	RHI_CompareMode depthFunc;

	bool stencilTest;
	uint8_t stencilReadMask;
	uint8_t stencilWriteMask;

	RHI_CompareMode stencilFrontFunc;

	RHI_StencilOp stencilFrontFail;
	RHI_StencilOp depthFrontFail;
	RHI_StencilOp stencilFrontPass;

	RHI_CompareMode stencilBackFunc;
	RHI_StencilOp stencilBackFail;
	RHI_StencilOp depthBackFail;
	RHI_StencilOp stencilBackPass;
} RHI_DepthStateDesc;

typedef struct RHI_RasterizerStateDesc {
	RHI_CullMode cullMode;
	int32_t depthBias;
	float slopeScaledDepthBias;
	RHI_FillMode fillMode;
	bool multiSample;
	bool scissor;
	RHI_FrontFace frontFace;
	bool depthClampEnable;

} RHI_RasterizerStateDesc;

typedef struct RHI_GraphicsPipelineDesc {
	RHI_ShaderHandle shaderProgram;
	RHI_RootSignatureHandle rootSignature;
	RHI_VertexLayout *pVertexLayout;
	RHI_BlendStateDesc *pBlendState;
	RHI_DepthStateDesc *pDepthState;
	RHI_RasterizerStateDesc *pRasterizerState;
	RHI_ImageFormat const *pColorFormats;
	uint32_t renderTargetCount;
	RHI_SampleCount sampleCount;
	uint32_t sampleQuality;
	RHI_ImageFormat depthStencilFormat;
	RHI_PrimitiveTopology primitiveTopo;
	bool supportIndirectCommandBuffer;
} RHI_GraphicsPipelineDesc;

typedef struct RHI_ComputePipelineDesc {
	RHI_ShaderHandle shaderProgram;
	RHI_RootSignatureHandle rootSignature;
} RHI_ComputePipelineDesc;

typedef struct RHI_PipelineDesc {
	RHI_PipelineType type;
	union {
		RHI_ComputePipelineDesc computeDesc;
		RHI_GraphicsPipelineDesc graphicsDesc;
		RHI_RaytracingPipelineDesc raytracingDesc;
	};
} RHI_PipelineDesc;

typedef struct RHI_Region3D {
	uint32_t xOffset;
	uint32_t yOffset;
	uint32_t zOffset;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} RHI_Region3D;

typedef struct RHI_SubresourceDataDesc
{
	// Source description
	uint64_t bufferOffset;
	uint32_t rowPitch;
	uint32_t slicePitch;
	// Destination description
	uint32_t arrayLayer;
	uint32_t mipLevel;
	RHI_Region3D region;
} RHI_SubresourceDataDesc;

typedef struct RHI_LoadActionsDesc {
	RHI_ClearValue clearColorValues[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_LoadActionType loadActionsColor[RHI_MAX_RENDER_TARGET_ATTACHMENTS];
	RHI_ClearValue clearDepth;
	RHI_LoadActionType loadActionDepth;
	RHI_LoadActionType loadActionStencil;
} RHI_LoadActionsDesc;

typedef struct RHI_CmdPoolDesc
{
	RHI_QueueHandle queue;
	bool transient;
} RHI_CmdPoolDesc;

typedef struct RHI_CmdDesc
{
	RHI_CmdPoolHandle pool;
#if defined(ORBIS)
	uint32_t maxSize; // ORBIS-only
#endif
	bool secondary;
} RHI_CmdDesc;

typedef struct RHI_DescriptorSetDesc {
	RHI_RootSignatureHandle rootSignature;
	RHI_DescriptorUpdateFrequency updateFrequency;
	uint32_t maxSets;
	uint32_t nodeIndex;
} RHI_DescriptorSetDesc;

typedef struct RHI_DescriptorData {
	const char *pName;
	union {
		struct {
			uint64_t const *pOffsets;
			uint64_t const *pSizes;
		};
		// Descriptor set buffer extraction options
		struct
		{
			uint32_t    mDescriptorSetBufferIndex;
			RHI_ShaderHandle mDescriptorSetShader;
			RHI_ShaderStage mDescriptorSetShaderStage;
		};

		uint32_t UAVMipSlice;
		bool bindStencilResource;
	};
	union {
		RHI_TextureHandle const *pTextures;
		RHI_SamplerHandle const *pSamplers;
		RHI_BufferHandle const *pBuffers;
		/// Array of pipline descriptors
		RHI_PipelineHandle* pPipelines;
		/// DescriptorSet buffer extraction
		RHI_DescriptorSetHandle* pDescriptorSet;
		/// Custom binding (raytracing acceleration structure ...)
		RHI_AcclerationStructureHandle const *pAccelerationStructures;
	};
	uint32_t count;
	uint32_t index = (uint32_t)-1;
	bool extractBuffer;
} RHI_DescriptorData;

typedef struct RHI_BufferBarrier {
	RHI_BufferHandle buffer;
	RHI_ResourceState newState;
	bool split;
} RHI_BufferBarrier;

typedef struct RHI_TextureBarrier {
	RHI_TextureHandle texture;
	RHI_ResourceState newState;
	bool split;
} RHI_TextureBarrier;

typedef struct RHI_RenderTargetBarrier {
	RHI_RenderTargetHandle  renderTarget;
	RHI_ResourceState  newState;
	bool split;
} RHI_RenderTargetBarrier;

typedef struct RHI_BufferDesc {
	uint64_t mSize;
	uint32_t mAlignment;
	RHI_ResourceMemoryUsage mMemoryUsage;
	RHI_BufferCreationFlags mFlags;
	RHI_ResourceState mStartState;
	uint64_t mFirstElement;
	uint64_t mElementCount;
	uint64_t mStructStride;
	/// ICB draw type
	RHI_IndirectArgumentType mICBDrawType;
	/// ICB max vertex buffers slots count
	uint32_t mICBMaxVertexBufferBind;
	/// ICB max vertex buffers slots count
	uint32_t mICBMaxFragmentBufferBind;
	RHI_BufferHandle counterBuffer;
	RHI_ImageFormat mFormat;
	RHI_DescriptorType mDescriptors;
	const char *pDebugName;
	uint32_t *pSharedNodeIndices;
	uint32_t mNodeIndex;
	uint32_t mSharedNodeIndexCount;
} RHI_BufferDesc;

typedef struct RHI_TextureDesc {
	RHI_TextureCreationFlags mFlags;
	uint32_t mWidth;
	uint32_t mHeight;
	uint32_t mDepth;
	uint32_t mArraySize;
	uint32_t mMipLevels;
	RHI_SampleCount mSampleCount;
	uint32_t mSampleQuality;
	RHI_ImageFormat mFormat;
	RHI_ClearValue mClearValue;
	RHI_ResourceState mStartState;
	RHI_DescriptorType mDescriptors;
	const void *pNativeHandle;
	const char *pDebugName;
	uint32_t *pSharedNodeIndices;
	uint32_t mSharedNodeIndexCount;
	uint32_t mNodeIndex;
	bool mHostVisible;
} RHI_TextureDesc;

typedef struct RHI_QueryPoolDesc {
	RHI_QueryType type;
	uint32_t queryCount;
	uint32_t nodeIndex;
} RHI_QueryPoolDesc;

typedef struct RHI_QueryDesc {
	uint32_t index;
} RHI_QueryDesc;

typedef struct RHI_IndirectDrawArguments {
	uint32_t mVertexCount;
	uint32_t mInstanceCount;
	uint32_t mStartVertex;
	uint32_t mStartInstance;
} RHI_IndirectDrawArguments;

typedef struct RHI_IndirectDrawIndexArguments {
	uint32_t mIndexCount;
	uint32_t mInstanceCount;
	uint32_t mStartIndex;
	uint32_t mVertexOffset;
	uint32_t mStartInstance;
} RHI_IndirectDrawIndexArguments;

typedef struct RHI_IndirectDispatchArguments {
	uint32_t mGroupCountX;
	uint32_t mGroupCountY;
	uint32_t mGroupCountZ;
} RHI_IndirectDispatchArguments;

typedef struct RHI_IndirectArgumentDescriptor {
	RHI_IndirectArgumentType mType;
	const char* pName;
	uint32_t mRootParameterIndex;
	uint32_t mCount;
	uint32_t mDivisor;

} RHI_IndirectArgumentDescriptor;

typedef struct RHI_CommandSignatureDesc {
	RHI_CmdPoolHandle cmdPool;
	RHI_RootSignatureHandle rootSignature;
	uint32_t indirectArgCount;
	RHI_IndirectArgumentDescriptor *pArgDescs;
} RHI_CommandSignatureDesc;

typedef struct RHI_ReadRange {
	uint64_t offset;
	uint64_t size;
} RHI_ReadRange;

typedef struct RHI_Extent3D {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} RHI_Extent3D;

typedef void *RHI_IconHandle;

typedef struct RHI_WindowHandle
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	Display*                 display;
	Window                   window;
	Atom                     xlib_wm_delete_window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_connection_t*        connection;
	xcb_window_t             window;
	xcb_screen_t*            screen;
	xcb_intern_atom_reply_t* atom_wm_delete_window;
#else
	void*                    window;    //hWnd
#endif
} RHI_WindowHandle;

typedef struct RHI_RectDesc {
	int left;
	int top;
	int right;
	int bottom;
} RHI_RectDesc;

typedef struct RHI_SwapChainDesc {
	/// Window handle
	RHI_WindowHandle windowHandle;
	/// Queues which should be allowed to present
	RHI_QueueHandle *pPresentQueues;
	/// Number of present queues
	uint32_t presentQueueCount;
	/// Number of backbuffers in this swapchain
	uint32_t imageCount;
	/// Width of the swapchain
	uint32_t width;
	/// Height of the swapchain
	uint32_t height;
	/// Color format of the swapchain
	RHI_ImageFormat colorFormat;
	/// Clear value
	RHI_ClearValue colorClearValue;
	/// Set whether swap chain will be presented using vsync
	bool enableVsync;
} RHI_SwapChainDesc;

#endif