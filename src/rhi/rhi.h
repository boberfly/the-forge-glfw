#pragma once
#ifndef RHI_RHI_H_
#define RHI_RHI_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "export.h"
#include "enums.h"
#include "imageformat.h"
#include "structs.h"
#include "resourceloader.h"
#include "shaderreflection.h"

#if defined(__cplusplus)
extern "C" {
#endif

RHI_API RHI_Result RHI_init(RHI_RendererApi renderer);
RHI_API void RHI_exit(void);

#if !defined(RHI_SKIP_DECLARATIONS)

RHI_API RHI_RendererHandle RHI_initRenderer(char const *appName,
											RHI_RendererDesc const *settings);
RHI_API void RHI_removeRenderer(RHI_RendererHandle handle);

RHI_API void RHI_addFence(RHI_RendererHandle handle, RHI_FenceHandle *pFence);
RHI_API void RHI_removeFence(RHI_RendererHandle handle, RHI_FenceHandle fence);
RHI_API void RHI_addSemaphore(RHI_RendererHandle handle, RHI_SemaphoreHandle *pSemaphore);
RHI_API void RHI_removeSemaphore(RHI_RendererHandle handle, RHI_SemaphoreHandle semaphore);

RHI_API void RHI_addQueue(RHI_RendererHandle handle,
						  RHI_QueueDesc *pQDesc,
						  RHI_QueueHandle *pQueue);
RHI_API void RHI_removeQueue(RHI_RendererHandle handle, RHI_QueueHandle queue);

RHI_API void RHI_addCmdPool(RHI_RendererHandle handle,
							const RHI_CmdPoolDesc *desc,
							RHI_CmdPoolHandle *pCmdPool);
RHI_API void RHI_removeCmdPool(RHI_RendererHandle handle, RHI_CmdPoolHandle cmdPool);
RHI_API void RHI_addCmd(RHI_RendererHandle handle, const RHI_CmdDesc *desc, RHI_CmdHandle *pCmd);
RHI_API void RHI_removeCmd(RHI_RendererHandle handle, RHI_CmdHandle cmd);
RHI_API void RHI_addCmd_n(RHI_RendererHandle handle,
						  const RHI_CmdDesc *desc,
						  uint32_t cmdCount,
						  RHI_CmdHandle **ppCmds);
RHI_API void RHI_removeCmd_n(RHI_RendererHandle handle, uint32_t cmdCount, RHI_CmdHandle *pCmds);

RHI_API void RHI_addRenderTarget(RHI_RendererHandle handle,
								 const RHI_RenderTargetDesc *pDesc,
								 RHI_RenderTargetHandle *pRenderTarget);
RHI_API void RHI_removeRenderTarget(RHI_RendererHandle handle,
									RHI_RenderTargetHandle renderTarget);

RHI_API void RHI_addSampler(RHI_RendererHandle handle,
							const RHI_SamplerDesc *pDesc,
							RHI_SamplerHandle *pSampler);
RHI_API void RHI_removeSampler(RHI_RendererHandle handle, RHI_SamplerHandle sampler);

RHI_API void RHI_addShader(RHI_RendererHandle handle,
						   const RHI_ShaderDesc *p_desc,
						   RHI_ShaderHandle *pShader);
RHI_API void RHI_addShaderBinary(RHI_RendererHandle handle,
								 const RHI_BinaryShaderDesc *p_desc,
								 RHI_ShaderHandle *pShader);
RHI_API void RHI_removeShader(RHI_RendererHandle handle, RHI_ShaderHandle pShader);

RHI_API void RHI_addRootSignature(RHI_RendererHandle handle,
								  const RHI_RootSignatureDesc *pRootDesc,
								  RHI_RootSignatureHandle *ppRootSignature);
RHI_API void RHI_removeRootSignature(RHI_RendererHandle handle,
									 RHI_RootSignatureHandle rootSignature);
RHI_API void RHI_addPipeline(RHI_RendererHandle handle,
							 const RHI_PipelineDesc *pPipelineDesc,
							 RHI_PipelineHandle *pPipeline);

RHI_API void RHI_removePipeline(RHI_RendererHandle handle, RHI_PipelineHandle pipeline);
RHI_API void RHI_addDescriptorSet(RHI_RendererHandle handle,
								  RHI_DescriptorSetDesc const *pDesc,
								  RHI_DescriptorSetHandle *pDescriptorSet);
RHI_API void RHI_removeDescriptorSet(RHI_RendererHandle handle,
									 RHI_DescriptorSetHandle descriptorSet);
RHI_API void RHI_updateDescriptorSet(RHI_RendererHandle handle,
									 uint32_t index,
									 RHI_DescriptorSetHandle descriptorSet,
									 uint32_t count,
									 RHI_DescriptorData const *pParams);
RHI_API void RHI_addQueryPool(RHI_RendererHandle handle,
							  const RHI_QueryPoolDesc *pDesc,
							  RHI_QueryPoolHandle *pQueryPool);
RHI_API void RHI_removeQueryPool(RHI_RendererHandle handle, RHI_QueryPoolHandle queryPool);


RHI_API void RHI_addBuffer(RHI_RendererHandle handle, RHI_BufferDesc const* pDesc, RHI_BufferHandle* pBuffer);
RHI_API void RHI_addTexture(RHI_RendererHandle handle, RHI_TextureDesc const* pDesc, RHI_TextureHandle* pTexture);
RHI_API void RHI_removeBuffer(RHI_RendererHandle handle, RHI_BufferHandle buffer);
RHI_API void RHI_removeTexture(RHI_RendererHandle handle, RHI_TextureHandle texture);

RHI_API void RHI_beginCmd(RHI_CmdHandle cmd);
RHI_API void RHI_endCmd(RHI_CmdHandle cmd);
RHI_API void RHI_cmdBindRenderTargets(RHI_CmdHandle cmd,
									  uint32_t renderTargetCount,
									  RHI_RenderTargetHandle *pRenderTargets,
									  RHI_RenderTargetHandle depthStencil,
									  const RHI_LoadActionsDesc *loadActions,
									  uint32_t *pColorArraySlices,
									  uint32_t *pColorMipSlices,
									  uint32_t depthArraySlice,
									  uint32_t depthMipSlice);
RHI_API void RHI_cmdSetViewport(RHI_CmdHandle cmd,
								float x,
								float y,
								float width,
								float height,
								float minDepth,
								float maxDepth);
RHI_API void RHI_cmdSetScissor(RHI_CmdHandle cmd,
							   uint32_t x,
							   uint32_t y,
							   uint32_t width,
							   uint32_t height);
RHI_API void RHI_cmdBindPipeline(RHI_CmdHandle cmd, RHI_PipelineHandle pipeline);
RHI_API void RHI_cmdBindDescriptorSet(RHI_CmdHandle cmd,
									  uint32_t index,
									  RHI_DescriptorSetHandle descriptorSet);
RHI_API void RHI_cmdBindPushConstants(RHI_CmdHandle cmd, 
									  RHI_RootSignatureHandle rootSignature, 
									  const char* name, 
									  const void* constants);
RHI_API void RHI_cmdBindPushConstantsByIndex(RHI_CmdHandle cmd, 
											 RHI_RootSignatureHandle rootSignature, 
											 uint32_t paramIndex, 
											 const void* constants);
RHI_API void RHI_cmdBindIndexBuffer(RHI_CmdHandle cmd, RHI_BufferHandle buffer, uint32_t indexType, uint64_t offset);
RHI_API void RHI_cmdBindVertexBuffer(RHI_CmdHandle cmd,
									 uint32_t bufferCount,
									 RHI_BufferHandle *pBuffers,
									 const uint32_t *pStrides,
									 const uint64_t *pOffsets);
RHI_API void RHI_cmdDraw(RHI_CmdHandle cmd, uint32_t vertexCount, uint32_t firstVertex);
RHI_API void RHI_cmdDrawInstanced(RHI_CmdHandle cmd,
								  uint32_t vertexCount,
								  uint32_t firstVertex,
								  uint32_t instanceCount,
								  uint32_t firstInstance);
RHI_API void RHI_cmdDrawIndexed(RHI_CmdHandle cmd,
								uint32_t indexCount,
								uint32_t firstIndex,
								uint32_t firstVertex);
RHI_API void RHI_cmdDrawIndexedInstanced(RHI_CmdHandle cmd,
										 uint32_t indexCount,
										 uint32_t firstIndex,
										 uint32_t instanceCount,
										 uint32_t firstVertex,
										 uint32_t firstInstance);
RHI_API void RHI_cmdDispatch(RHI_CmdHandle cmd,
							 uint32_t groupCountX,
							 uint32_t groupCountY,
							 uint32_t groupCountZ);
RHI_API void RHI_cmdResourceBarrier(RHI_CmdHandle cmd,
									uint32_t bufferBarrierCount,
									RHI_BufferBarrier *pBufferBarriers,
									uint32_t textureBarrierCount,
									RHI_TextureBarrier *pTextureBarriers,
									uint32_t rtBarrierCount,
									RHI_RenderTargetBarrier *rtBarriers);
RHI_API void RHI_cmdUpdateVirtualTexture(RHI_CmdHandle cmd, RHI_TextureHandle texture);
RHI_API void RHI_cmdBeginDebugMarker(RHI_CmdHandle cmd, float r, float g, float b, const char *pName);
RHI_API void RHI_cmdExecuteIndirect(RHI_CmdHandle cmd,
									RHI_CommandSignatureHandle commandSignature,
									uint32_t maxCommandCount,
									RHI_BufferHandle indirectBuffer,
									uint64_t bufferOffset,
									RHI_BufferHandle counterBuffer,
									uint64_t counterBufferOffset);
RHI_API void RHI_cmdBeginQuery(RHI_CmdHandle cmd,
							   RHI_QueryPoolHandle queryPool,
							   RHI_QueryDesc *pQuery);
RHI_API void RHI_cmdEndQuery(RHI_CmdHandle cmd,
							 RHI_QueryPoolHandle queryPool,
							 RHI_QueryDesc *pQuery);
RHI_API void RHI_cmdResolveQuery(RHI_CmdHandle cmd,
								 RHI_QueryPoolHandle queryPool,
								 RHI_BufferHandle readbackBuffer,
								 uint32_t startQuery,
								 uint32_t queryCount);
RHI_API void RHI_cmdEndDebugMarker(RHI_CmdHandle cmd);
RHI_API void RHI_cmdAddDebugMarker(RHI_CmdHandle cmd, float r, float g, float b, const char *pName);

RHI_API void RHI_queueSubmit(RHI_QueueHandle queue,
							 const RHI_QueueSubmitDesc *desc);
RHI_API void RHI_getTimestampFrequency(RHI_QueueHandle queue, double *pFrequency);
RHI_API void RHI_waitQueueIdle(RHI_QueueHandle queue);

RHI_API void RHI_getFenceStatus(RHI_RendererHandle handle,
								RHI_FenceHandle fence,
								RHI_FenceStatus *p_fence_status);
RHI_API void RHI_waitForFences(RHI_RendererHandle handle,
							   uint32_t fenceCount,
							   RHI_FenceHandle *pFences);

RHI_API void RHI_addIndirectCommandSignature(RHI_RendererHandle handle,
											 const RHI_CommandSignatureDesc *p_desc,
											 RHI_CommandSignatureHandle* pCommandSignature);
RHI_API void RHI_removeIndirectCommandSignature(RHI_RendererHandle handle,
												RHI_CommandSignatureHandle commandSignature);


RHI_API void RHI_calculateMemoryStats(RHI_RendererHandle handle, char **stats);
RHI_API void RHI_freeMemoryStats(RHI_RendererHandle handle, char *stats);
RHI_API void RHI_setBufferName(RHI_RendererHandle handle,
							   RHI_BufferHandle buffer,
							   const char *pName);
RHI_API void RHI_setTextureName(RHI_RendererHandle handle,
								RHI_TextureHandle texture,
								const char *pName);
RHI_API void RHI_mapBuffer(RHI_RendererHandle handle, RHI_BufferHandle buffer, RHI_ReadRange* pRange);
RHI_API void RHI_unmapBuffer(RHI_RendererHandle handle, RHI_BufferHandle buffer);

RHI_API void RHI_addSwapChain(RHI_RendererHandle handle, const RHI_SwapChainDesc* pDesc, RHI_SwapChainHandle* pSwapChain);
RHI_API void RHI_removeSwapChain(RHI_RendererHandle handle, RHI_SwapChainHandle swapChain);
RHI_API void RHI_toggleVSync(RHI_RendererHandle handle, RHI_SwapChainHandle* pSwapchain);
//RHI_API bool RHI_isImageFormatSupported(RHI_ImageFormat format);
RHI_API RHI_ImageFormat RHI_getRecommendedSwapchainFormat(bool hintHDR);
RHI_API void RHI_acquireNextImage(RHI_RendererHandle handle, RHI_SwapChainHandle swapChain, RHI_SemaphoreHandle signalSemaphore, RHI_FenceHandle fence, uint32_t* pImageIndex);
RHI_API void RHI_queuePresent(RHI_QueueHandle queue, const RHI_QueuePresentDesc *desc);


// accessors TheForge C API has opaque handles, this is largely okay as its mostly a push API
// however a few things do need passing back (mostly for rendertarget and swapchain)
// these calls implment the required accessors.
RHI_API RHI_RenderTargetHandle RHI_swapChainGetRenderTarget(RHI_SwapChainHandle swapChain, int index);
RHI_API bool RHI_swapChainGetVSync(RHI_SwapChainHandle handle);
RHI_API RHI_TextureHandle RHI_renderTargetGetTexture(RHI_RenderTargetHandle renderTarget);
RHI_API void RHI_renderTargetGetDesc(RHI_RenderTargetHandle renderTarget, RHI_RenderTargetDesc &desc);
RHI_API uint32_t RHI_textureGetWidth(RHI_TextureHandle handle);
RHI_API uint32_t RHI_textureGetHeight(RHI_TextureHandle handle);

RHI_API RHI_RendererApi RHI_getRendererApi(RHI_RendererHandle handle);
RHI_API bool RHI_canShaderReadFrom(RHI_RendererHandle handle, RHI_ImageFormat format);
RHI_API bool RHI_canColorWriteTo(RHI_RendererHandle handle, RHI_ImageFormat format);
RHI_API bool RHI_canShaderWriteTo(RHI_RendererHandle handle, RHI_ImageFormat format);

// do not free return owned by the shader!
RHI_API RHI_PipelineReflection const* RHI_shaderGetPipelineReflection(RHI_ShaderHandle shader);

#endif // !defined(RHI_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // end
