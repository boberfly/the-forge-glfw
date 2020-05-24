#pragma once
#ifndef RHI_RESOURCELOADER_H_
#define RHI_RESOURCELOADER_H_

#include "export.h"
#include "resourceloader_enums.h"
#include "resourceloader_structs.h"

#if defined(__cplusplus)
extern "C" {
#endif

RHI_API void RHI_initResourceLoaderInterface(RHI_RendererHandle handle, RHI_ResourceLoaderDesc *pDesc);
RHI_API void RHI_exitResourceLoaderInterface(RHI_RendererHandle handle);

RHI_API void RHI_addBufferResource(RHI_BufferLoadDesc *pBufferDesc, RHI_SyncToken *token, RHI_LoadPriority priority);
RHI_API void RHI_addTextureResource(RHI_TextureLoadDesc *pTextureDesc, RHI_SyncToken *token, RHI_LoadPriority priority);
//RHI_API void RHI_addGeometryResource(RHI_GeometryLoadDesc *pGeomDesc, RHI_SyncToken *token, RHI_LoadPriority priority);

RHI_API void RHI_beginUpdateBufferResource(RHI_BufferUpdateDesc const *pBuffer);
RHI_API void RHI_beginUpdateTextureResource(RHI_TextureUpdateDesc const *pTexture);
RHI_API void RHI_endUpdateBufferResource(RHI_BufferUpdateDesc *pBuffer, RHI_SyncToken *token);
RHI_API void RHI_endUpdateTextureResource(RHI_TextureUpdateDesc *pTexture, RHI_SyncToken *token);

RHI_API void RHI_removeBufferResource(RHI_BufferHandle pBuffer);
RHI_API void RHI_removeTextureResource(RHI_TextureHandle pTexture);
//RHI_API void RHI_removeGeometryResource(RHI_GeometryHandle pGeom);

RHI_API bool RHI_allResourceLoadsCompleted();

RHI_API void RHI_waitForAllResourceLoads();

RHI_API RHI_SyncToken RHI_getLastTokenCompleted();
RHI_API bool RHI_isTokenCompleted(const RHI_SyncToken *token);
RHI_API void RHI_waitForToken(const RHI_SyncToken *token);

void RHI_addShaderResource(RHI_RendererHandle handle, const RHI_ShaderLoadDesc* pDesc, RHI_ShaderHandle *pShader);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
