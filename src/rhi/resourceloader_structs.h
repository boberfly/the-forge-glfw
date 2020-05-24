#pragma once
#ifndef RHI_RESOURCELOADER_STRUCTS_H_
#define RHI_RESOURCELOADER_STRUCTS_H_

#include "enums.h"
#include "resourceloader_enums.h"
#include "structs.h"

typedef struct RHI_MappedMemoryRange
{
	uint8_t* pData;
	RHI_BufferHandle pBuffer;
	uint64_t mOffset;
	uint64_t mSize;
} RHI_MappedMemoryRange;

typedef struct RHI_BufferUpdateInternalData {
	RHI_MappedMemoryRange mMappedRange;
	bool mBufferNeedsUnmap;
} RHI_BufferUpdateInternalData;

typedef struct RHI_BufferLoadDesc
{
	RHI_BufferHandle *pBuffer;
	const void* pData;
	RHI_BufferDesc mDesc;
	/// Force Reset buffer to NULL
	bool mForceReset;
	/// Whether to skip uploading any data to the buffer.
	/// Automatically set to true if using addResource (rather than begin/endAddResource)
	/// with pData = NULL and mForceReset = false
	bool mSkipUpload;

	RHI_BufferUpdateInternalData mInternalData;
} RHI_BufferLoadDesc;

typedef struct RHI_RawImageData
{
	unsigned char const *pRawData;
	RHI_ImageFormat format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t arraySize;
	uint32_t mipLevels;
	bool mipsAfterSlices;

	// The stride between subsequent rows.
	// If using a beginUpdateResource/endUpdateResource pair,
	// copies to pRawData should use this stride.
	// A stride of 0 means the data is tightly packed.
	uint32_t mRowStride;
} RHI_RawImageData;

typedef struct RHI_BinaryImageData
{
	unsigned char* pBinaryData;
	uint32_t mSize;
	const char* pExtension;
} RHI_BinaryImageData;

typedef struct RHI_TextureLoadDesc
{
	RHI_TextureHandle* pTexture;

	/// Load empty texture
	RHI_TextureDesc* pDesc;

	/// Load texture from disk
	const void* pFilePath;
	uint32_t    mNodeIndex;
	/// Load texture from raw data
	RHI_RawImageData const* pRawImageData;
	/// Load texture from binary data (with header)
	RHI_BinaryImageData const* pBinaryImageData;

	// Following is ignored if pDesc != NULL.  pDesc->mFlags will be considered instead.
	RHI_TextureCreationFlags mCreationFlag;

	struct
	{
		RHI_MappedMemoryRange mMappedRange;
	} mInternalData;
} RHI_TextureLoadDesc;

typedef struct RHI_VirtualTexturePageInfo
{
	uint32_t pageAlive;
	uint32_t TexID;
	uint32_t mipLevel;
	uint32_t padding1;
} RHI_VirtualTexturePageInfo;

typedef struct RHI_BufferUpdateDesc
{
	RHI_BufferHandle     buffer;
	uint64_t    mDstOffset;
	uint64_t    mSize;

	/// To be filled by the caller
	/// Example:
	/// BufferUpdateDesc update = { pBuffer, bufferDstOffset };
	/// beginUpdateResource(&update);
	/// ParticleVertex* vertices = (ParticleVertex*)update.pMappedData;
	///   for (uint32_t i = 0; i < particleCount; ++i)
	///	    vertices[i] = { rand() };
	/// endUpdateResource(&update, &token);
	void*                    pMappedData;

	// Internal
	RHI_BufferUpdateInternalData mInternalData;
} RHI_BufferUpdateDesc;

typedef struct RHI_TextureUpdateDesc
{
	RHI_TextureHandle texture;
	RHI_RawImageData* pRawImageData;

	void* pMappedData;
	struct
	{
		RHI_MappedMemoryRange mMappedRange;
	} mInternalData;
} RHI_TextureUpdateDesc;

typedef struct RHI_ShaderStageLoadDesc
{
	const char*           pFileName;
	RHI_ShaderMacro*      pMacros;
	uint32_t              mMacroCount;
	RHI_ResourceDirectory mRoot;
	const char*           pEntryPointName;
} RHI_ShaderStageLoadDesc;

typedef struct RHI_ShaderLoadDesc
{
	RHI_ShaderStageLoadDesc mStages[RHI_SHADER_STAGE_COUNT];
	RHI_ShaderTarget        mTarget;
} RHI_ShaderLoadDesc;

typedef struct RHI_SyncToken
{
	uint64_t waitIndex[RHI_LOAD_PRIORITY_COUNT];
} RHI_SyncToken;

typedef struct RHI_ResourceLoaderDesc
{
	uint64_t mBufferSize;
	uint32_t mBufferCount;
} RHI_ResourceLoaderDesc;

#endif