#include "../rhi.h"

#if !defined(RHI_SKIP_DECLARATIONS)

#include <string.h> // memcpy
#include <Renderer/IRenderer.h>
#include <Renderer/IResourceLoader.h>
#include <ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h>

static void API_CHECK();

// these are hidden but we want them
extern void addBuffer(Renderer *pRenderer, const BufferDesc *desc, Buffer **pp_buffer);
extern void removeBuffer(Renderer *pRenderer, Buffer *p_buffer);
extern void addTexture(Renderer *pRenderer, const TextureDesc *pDesc, Texture **pp_texture);
extern void removeTexture(Renderer *pRenderer, Texture *p_texture);

extern void mapBuffer(Renderer *pRenderer, Buffer *pBuffer, ReadRange *pRange);
extern void unmapBuffer(Renderer *pRenderer, Buffer *pBuffer);
extern void cmdUpdateBuffer(Cmd *pCmd,
							Buffer *pBuffer,
							uint64_t dstOffset,
							Buffer *pSrcBuffer,
							uint64_t srcOffset,
							uint64_t size);
extern void cmdUpdateSubresource(Cmd *pCmd,
								 Texture *pTexture,
								 Buffer *pSrcBuffer,
								 SubresourceDataDesc *pSubresourceDesc);

RHI_Result RHI_init(RHI_RendererApi renderer)
{
	return RHI_RESULT_SUCCESS;
}

void RHI_exit(void)
{
	return;
}

/*
static void LogFunc(LogType type, const char *m0, const char *m1) {
	switch (type) {
		case LogType::LOG_TYPE_INFO: LOGINFO("%s %s", m0, m1);
		case LogType::LOG_TYPE_DEBUG: LOGDEBUG("%s %s", m0, m1);
		case LogType::LOG_TYPE_WARN: LOGWARNING("%s %s", m0, m1);
		case LogType::LOG_TYPE_ERROR: LOGERROR("%s %s", m0, m1);
	}
}
*/

static ShaderStage RHI_shaderStageToShaderStage(RHI_ShaderStage stage) {
#if defined(METAL)
	switch (stage) {
		case RHI_SHADER_STAGE_NONE: return SHADER_STAGE_NONE;
		case RHI_SHADER_STAGE_VERT: return SHADER_STAGE_VERT;
		case RHI_SHADER_STAGE_FRAG: return SHADER_STAGE_FRAG;
		case RHI_SHADER_STAGE_COMP: return SHADER_STAGE_COMP;
		default: //LOGERROR("Shader stage is not supported on Metal backend");
			return SHADER_STAGE_NONE;
	}
#else
	switch(stage) {
	case RHI_SHADER_STAGE_NONE:				return SHADER_STAGE_NONE;
	case RHI_SHADER_STAGE_VERT:				return SHADER_STAGE_VERT;
	case RHI_SHADER_STAGE_TESC:				return SHADER_STAGE_TESC;
	case RHI_SHADER_STAGE_TESE:				return SHADER_STAGE_TESE;
	case RHI_SHADER_STAGE_GEOM:				return SHADER_STAGE_GEOM;
	case RHI_SHADER_STAGE_FRAG:				return SHADER_STAGE_FRAG;
	case RHI_SHADER_STAGE_COMP:				return SHADER_STAGE_COMP;
	case RHI_SHADER_STAGE_RAYTRACING:	return SHADER_STAGE_RAYTRACING;
	default:
		//LOGERROR("Shader stage is not supported on Metal backend");
		return SHADER_STAGE_NONE;
	}
#endif
}

static ShaderStage RHI_shaderStageFlagsToShaderStage(uint32_t flags) {
	uint32_t stage = SHADER_STAGE_NONE;
#if defined(METAL)
	if (flags & RHI_SHADER_STAGE_VERT) {
		stage |= SHADER_STAGE_VERT;
	}
	if (flags & RHI_SHADER_STAGE_FRAG) {
		stage |= SHADER_STAGE_FRAG;
	}
	if (flags & RHI_SHADER_STAGE_COMP) {
		stage |= SHADER_STAGE_COMP;
	}
#else
	if(flags & RHI_SHADER_STAGE_VERT) stage |= SHADER_STAGE_VERT;
	if(flags & RHI_SHADER_STAGE_FRAG) stage |= SHADER_STAGE_FRAG;
	if(flags & RHI_SHADER_STAGE_COMP) stage |= SHADER_STAGE_COMP;
	if(flags & RHI_SHADER_STAGE_TESC) stage |= SHADER_STAGE_TESC;
	if(flags & RHI_SHADER_STAGE_TESE) stage |= SHADER_STAGE_TESE;
	if(flags & RHI_SHADER_STAGE_GEOM) stage |= SHADER_STAGE_GEOM;
	if(flags & RHI_SHADER_STAGE_RAYTRACING) stage |= SHADER_STAGE_RAYTRACING;
#endif
	return (ShaderStage) stage;
}

#ifdef METAL
static void RHI_shaderStageToShaderStage(RHI_ShaderStageDesc const *src, ShaderStageDesc *dst) {
	dst->pName = src->name;
	dst->pCode = src->code;
	dst->pEntryPoint = src->entryPoint;
}
#endif

static void RHI_binaryShaderStageToBinaryShaderStage(RHI_BinaryShaderStageDesc const *src, BinaryShaderStageDesc *dst) {
	dst->pByteCode = src->byteCode;
	dst->mByteCodeSize = src->byteCodeSize;
	dst->pEntryPoint = src->entryPoint;
#ifdef METAL
	dst->pSource = src->source;
#endif
}

RHI_API RHI_RendererHandle RHI_initRenderer(
		char const *appName,
		RHI_RendererDesc const *settings) {

	API_CHECK(); // windows static_assert(offsetof) is broken so do it at runtime there

/*
	RendererDesc desc{
			nullptr, //&LogFunc,
			(RendererApi) 0,
			(ShaderTarget) settings->shaderTarget,
	};

			(RendererApi) RENDERER_API_VULKAN,
			(ShaderTarget) settings->shaderTarget,
*/

	RendererDesc desc = {0};
	Renderer *renderer;
	initRenderer(appName, &desc, &renderer);

	return (RHI_Renderer *) renderer;
}

RHI_API void RHI_removeRenderer(RHI_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeRenderer(renderer);
}

RHI_API void RHI_addFence(RHI_RendererHandle handle, RHI_FenceHandle *pFence) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addFence(renderer, (Fence **) pFence);

}

RHI_API void RHI_removeFence(RHI_RendererHandle handle, RHI_FenceHandle fence) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	removeFence(renderer, (Fence *) fence);

}
RHI_API void RHI_addSemaphore(RHI_RendererHandle handle, RHI_SemaphoreHandle *pSemaphore) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addSemaphore(renderer, (Semaphore **) pSemaphore);
}

RHI_API void RHI_removeSemaphore(RHI_RendererHandle handle, RHI_SemaphoreHandle semaphore) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeSemaphore(renderer, (Semaphore *) semaphore);
}

RHI_API void RHI_addQueue(RHI_RendererHandle handle, RHI_QueueDesc *pQDesc, RHI_QueueHandle *pQueue) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addQueue(renderer, (QueueDesc *) pQDesc, (Queue **) pQueue);
}

RHI_API void RHI_removeQueue(RHI_RendererHandle handle, RHI_Queue *queue) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeQueue(renderer, (Queue *) queue);
}

RHI_API void RHI_addCmdPool(RHI_RendererHandle handle,
							const RHI_CmdPoolDesc *desc,
							RHI_CmdPoolHandle *pCmdPool) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addCmdPool(renderer, (const CmdPoolDesc*)desc, (CmdPool **) pCmdPool);
}

RHI_API void RHI_removeCmdPool(RHI_RendererHandle handle, RHI_CmdPoolHandle cmdPool) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeCmdPool(renderer, (CmdPool *) cmdPool);
}

RHI_API void RHI_addCmd(RHI_CmdPoolHandle handle, const RHI_CmdDesc *desc, RHI_CmdHandle *pCmd) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addCmd(renderer, (const CmdDesc*)desc, (Cmd **) pCmd);
}

RHI_API void RHI_removeCmd(RHI_RendererHandle handle, RHI_CmdHandle cmd) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeCmd(renderer, (Cmd *) cmd);
}

RHI_API void RHI_addCmd_n(RHI_RendererHandle handle,
						  const RHI_CmdDesc *desc,
						  uint32_t cmdCount,
						  RHI_CmdHandle **ppCmds) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addCmd_n(renderer, (const CmdDesc*)desc, cmdCount, (Cmd ***) ppCmds);
}

RHI_API void RHI_removeCmd_n(RHI_RendererHandle handle, uint32_t cmdCount, RHI_CmdHandle *pCmds) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeCmd_n(renderer, cmdCount, (Cmd **) pCmds);
}

RHI_API void RHI_addRenderTarget(RHI_RendererHandle handle,
								 RHI_RenderTargetDesc const *pDesc,
								 RHI_RenderTargetHandle *pRenderTarget) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	TinyImageFormat tfif = (TinyImageFormat)pDesc->format;
	ClearValue cv;
	memcpy(&cv, &pDesc->clearValue, sizeof(ClearValue));

	RenderTargetDesc desc{
			(TextureCreationFlags) pDesc->flags,
			pDesc->width,
			pDesc->height,
			pDesc->depth,
			pDesc->arraySize,
			pDesc->mipLevels,
			(SampleCount) pDesc->sampleCount,
			tfif,
			cv,
			pDesc->sampleQuality,
			(DescriptorType) pDesc->descriptors,
			nullptr,
			nullptr, // TODO (wchar_t const*) pDesc->debugName,
			0,
			0,
			0,
	};
	addRenderTarget(renderer, &desc, (RenderTarget **) pRenderTarget);
}

RHI_API void RHI_removeRenderTarget(RHI_RendererHandle handle,
																								RHI_RenderTargetHandle renderTarget) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeRenderTarget(renderer, (RenderTarget *) renderTarget);
}

RHI_API void RHI_addSampler(RHI_RendererHandle handle,
							const RHI_SamplerDesc *pDesc,
							RHI_SamplerHandle *pSampler) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addSampler(renderer, (SamplerDesc *) pDesc, (Sampler **) pSampler);
}
RHI_API void RHI_removeSampler(RHI_RendererHandle handle, RHI_SamplerHandle sampler) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeSampler(renderer, (Sampler *) sampler);
}

RHI_API void RHI_addShader(RHI_RendererHandle handle,
						   const RHI_ShaderDesc *pDesc,
						   RHI_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
#ifdef METAL
	ShaderDesc desc{};
	desc.mStages = RHI_ShaderStageFlagsToShaderStage(pDesc->stages);
	if (desc.mStages & SHADER_STAGE_VERT) {
		RHI_ShaderStageToShaderStage(&pDesc->vert, &desc.mVert);
	}
	if (desc.mStages & SHADER_STAGE_FRAG) {
		RHI_ShaderStageToShaderStage(&pDesc->frag, &desc.mFrag);
	}
	if (desc.mStages & SHADER_STAGE_COMP) {
		RHI_ShaderStageToShaderStage(&pDesc->comp, &desc.mComp);
	}

	addShader(renderer, &desc, (Shader **) pShader);
#else
	//LOGERROR("AddShader is only supported on Metal backends, Use AddShaderBinary");
#endif
}

RHI_API void RHI_addShaderBinary(RHI_RendererHandle handle,
								 const RHI_BinaryShaderDesc *pDesc,
								 RHI_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	BinaryShaderDesc desc{};
	desc.mStages = RHI_shaderStageFlagsToShaderStage(pDesc->stages);
	if (desc.mStages & SHADER_STAGE_VERT) {
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->vert, &desc.mVert);
	}
	if (desc.mStages & SHADER_STAGE_FRAG) {
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->frag, &desc.mFrag);
	}
#ifndef METAL
	if(desc.mStages & SHADER_STAGE_GEOM)
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->geom, &desc.mGeom);
	if(desc.mStages & SHADER_STAGE_HULL)
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->hull, &desc.mHull);
	if(desc.mStages & SHADER_STAGE_DOMN)
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->domain, &desc.mDomain);
#endif
	if (desc.mStages & SHADER_STAGE_COMP) {
		RHI_binaryShaderStageToBinaryShaderStage(&pDesc->comp, &desc.mComp);
	}

	addShaderBinary(renderer, &desc, (Shader **) pShader);
}

RHI_API void RHI_removeShader(RHI_RendererHandle handle, RHI_ShaderHandle pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeShader(renderer, (Shader *) pShader);
}

RHI_API void RHI_addRootSignature(RHI_RendererHandle handle,
								  const RHI_RootSignatureDesc *pRootDesc,
								  RHI_RootSignatureHandle *pRootSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addRootSignature(renderer, (RootSignatureDesc *) pRootDesc, (RootSignature **) pRootSignature);
}

RHI_API void RHI_removeRootSignature(RHI_RendererHandle handle,
																								 RHI_RootSignatureHandle rootSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeRootSignature(renderer, (RootSignature *) rootSignature);
}


RHI_API void RHI_addPipeline(RHI_RendererHandle handle,
							 const RHI_PipelineDesc *pPipelineDesc,
							 RHI_PipelineHandle *pPipeline) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addPipeline(renderer, (PipelineDesc *) pPipelineDesc, (Pipeline **) pPipeline);
}
RHI_API void RHI_removePipeline(RHI_RendererHandle handle, RHI_PipelineHandle pipeline) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removePipeline(renderer, (Pipeline *) pipeline);
}

RHI_API void RHI_addDescriptorSet(RHI_RendererHandle handle,
								  RHI_DescriptorSetDesc const *pDesc,
								  RHI_DescriptorSetHandle *pDescriptorSet) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addDescriptorSet(renderer, (DescriptorSetDesc const *) pDesc, (DescriptorSet **) pDescriptorSet);
}

RHI_API void RHI_removeDescriptorSet(RHI_RendererHandle handle,
									 RHI_DescriptorSetHandle descriptorSet) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeDescriptorSet(renderer, (DescriptorSet *) descriptorSet);
}

RHI_API void RHI_updateDescriptorSet(RHI_RendererHandle handle,
									 uint32_t index,
									 RHI_DescriptorSetHandle descriptorSet,
									 uint32_t count,
									 RHI_DescriptorData const *pParams) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	updateDescriptorSet(renderer, index, (DescriptorSet *) descriptorSet, count, (DescriptorData const *) pParams);
}

RHI_API void RHI_beginCmd(RHI_CmdHandle cmd) {
	beginCmd((Cmd *) cmd);
}
RHI_API void RHI_endCmd(RHI_CmdHandle cmd) {
	endCmd((Cmd *) cmd);
}
RHI_API void RHI_cmdBindRenderTargets(RHI_CmdHandle cmd,
									  uint32_t renderTargetCount,
									  RHI_RenderTargetHandle *pRenderTargets,
									  RHI_RenderTargetHandle depthStencil,
									  const RHI_LoadActionsDesc *loadActions,
									  uint32_t *pColorArraySlices,
									  uint32_t *pColorMipSlices,
									  uint32_t depthArraySlice,
									  uint32_t depthMipSlice) {

	cmdBindRenderTargets((Cmd *) cmd,
						 renderTargetCount,
						 (RenderTarget **) pRenderTargets,
						 (RenderTarget *) depthStencil,
						 (LoadActionsDesc *) loadActions,
						 pColorArraySlices,
						 pColorMipSlices,
						 depthArraySlice,
						 depthMipSlice);

}
RHI_API void RHI_cmdSetViewport(RHI_CmdHandle cmd,
								float x,
								float y,
								float width,
								float height,
								float minDepth,
								float maxDepth) {
	cmdSetViewport((Cmd *) cmd, x, y, width, height, minDepth, maxDepth);

}
RHI_API void RHI_cmdSetScissor(RHI_CmdHandle cmd,
							   uint32_t x,
							   uint32_t y,
							   uint32_t width,
							   uint32_t height) {
	cmdSetScissor((Cmd *) cmd, x, y, width, height);

}
RHI_API void RHI_cmdBindPipeline(RHI_CmdHandle cmd, RHI_PipelineHandle pipeline) {
	cmdBindPipeline((Cmd *) cmd, (Pipeline *) pipeline);
}

RHI_API void RHI_cmdBindDescriptorSet(RHI_CmdHandle cmd,
									  uint32_t index,
									  RHI_DescriptorSetHandle descriptorSet) {
	cmdBindDescriptorSet((Cmd *) cmd, index, (DescriptorSet *) descriptorSet);
}
RHI_API void RHI_cmdBindPushConstants(RHI_CmdHandle cmd,
									  RHI_RootSignatureHandle rootSignature,
									  const char *name,
									  const void *constants) {
	cmdBindPushConstants((Cmd *)cmd, (RootSignature*)rootSignature, name, constants);
}
RHI_API void RHI_cmdBindPushConstantsByIndex(RHI_CmdHandle cmd,
											 RHI_RootSignatureHandle rootSignature,
											 uint32_t paramIndex,
											 const void *constants) {
	cmdBindPushConstantsByIndex((Cmd *)cmd, (RootSignature*)rootSignature, paramIndex, constants);

}

RHI_API void RHI_cmdBindIndexBuffer(RHI_CmdHandle cmd, RHI_BufferHandle buffer, uint32_t indexType, uint64_t offset) {

	cmdBindIndexBuffer((Cmd *) cmd, (Buffer *) buffer, indexType, offset);
}
RHI_API void RHI_cmdBindVertexBuffer(RHI_CmdHandle cmd,
									 uint32_t bufferCount,
									 RHI_BufferHandle *pBuffers,
									 const uint32_t* pStrides, 
									 const uint64_t *pOffsets) {
	cmdBindVertexBuffer((Cmd *) cmd, bufferCount, (Buffer **) pBuffers, (uint32_t *) pStrides, (uint64_t *) pOffsets);
}
RHI_API void RHI_cmdDraw(RHI_CmdHandle cmd, uint32_t vertexCount, uint32_t firstVertex) {
	cmdDraw((Cmd *) cmd, vertexCount, firstVertex);
}
RHI_API void RHI_cmdDrawInstanced(RHI_CmdHandle cmd,
								  uint32_t vertexCount,
								  uint32_t firstVertex,
								  uint32_t instanceCount,
								  uint32_t firstInstance) {
	cmdDrawInstanced((Cmd *) cmd, vertexCount, firstVertex, instanceCount, firstInstance);
}
RHI_API void RHI_cmdDrawIndexed(RHI_CmdHandle cmd,
								uint32_t indexCount,
								uint32_t firstIndex,
								uint32_t firstVertex) {
	cmdDrawIndexed((Cmd *) cmd, indexCount, firstIndex, firstVertex);

}
RHI_API void RHI_cmdDrawIndexedInstanced(RHI_CmdHandle cmd,
										 uint32_t indexCount,
										 uint32_t firstIndex,
										 uint32_t instanceCount,
										 uint32_t firstVertex,
										 uint32_t firstInstance) {
	cmdDrawIndexedInstanced((Cmd *) cmd, indexCount, firstIndex, instanceCount, firstVertex, firstInstance);
}
RHI_API void RHI_cmdDispatch(RHI_CmdHandle cmd,
							 uint32_t groupCountX,
							 uint32_t groupCountY,
							 uint32_t groupCountZ) {
	cmdDispatch((Cmd *) cmd, groupCountX, groupCountY, groupCountZ);
}
RHI_API void RHI_cmdResourceBarrier(RHI_CmdHandle cmd,
									uint32_t bufferBarrierCount,
									RHI_BufferBarrier *pBufferBarriers,
									uint32_t textureBarrierCount,
									RHI_TextureBarrier *pTextureBarriers,
									uint32_t rtBarrierCount,
									RHI_RenderTargetBarrier *pRtBarriers) {
	cmdResourceBarrier((Cmd *) cmd,
					   bufferBarrierCount,
					   (BufferBarrier *) pBufferBarriers,
					   textureBarrierCount,
					   (TextureBarrier *) pTextureBarriers,
					   rtBarrierCount,
					   (RenderTargetBarrier *) pRtBarriers);
}

RHI_API void RHI_cmdUpdateVirtualTexture(RHI_CmdHandle cmd, RHI_TextureHandle texture) {
	cmdUpdateVirtualTexture((Cmd*)cmd, (Texture*)texture);
}

RHI_API void RHI_waitQueueIdle(RHI_QueueHandle queue) {

	waitQueueIdle((Queue *) queue);
}
RHI_API void RHI_getFenceStatus(RHI_RendererHandle handle,
								RHI_FenceHandle fence,
								RHI_FenceStatus *pFenceStatus) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	getFenceStatus(renderer, (Fence *) fence, (FenceStatus *) pFenceStatus);
}

RHI_API void RHI_waitForFences(RHI_RendererHandle handle,
							   uint32_t fenceCount,
							   RHI_FenceHandle *pFences) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	waitForFences(renderer, fenceCount, (Fence **) pFences);
}

RHI_API void RHI_cmdBeginDebugMarker(RHI_CmdHandle cmd, float r, float g, float b, const char *pName) {
	cmdBeginDebugMarker((Cmd *) cmd, r, g, b, pName);
}
RHI_API void RHI_cmdExecuteIndirect(RHI_CmdHandle cmd,
									RHI_CommandSignatureHandle commandSignature,
									uint32_t maxCommandCount,
									RHI_BufferHandle indirectBuffer,
									uint64_t bufferOffset,
									RHI_BufferHandle counterBuffer,
									uint64_t counterBufferOffset) {
	cmdExecuteIndirect((Cmd *) cmd,
					   (CommandSignature *) commandSignature,
					   maxCommandCount,
					   (Buffer *) indirectBuffer,
					   bufferOffset,
					   (Buffer *) counterBuffer,
					   counterBufferOffset);
}

RHI_API void RHI_cmdBeginQuery(RHI_CmdHandle cmd,
							   RHI_QueryPoolHandle queryPool,
							   RHI_QueryDesc *pQuery) {
	cmdBeginQuery((Cmd *) cmd,
				  (QueryPool *) queryPool,
				  (QueryDesc *) pQuery);
}
RHI_API void RHI_cmdEndQuery(RHI_CmdHandle cmd,
							 RHI_QueryPoolHandle queryPool,
							 RHI_QueryDesc *pQuery) {
	cmdEndQuery((Cmd *) cmd,
				(QueryPool *) queryPool,
				(QueryDesc *) pQuery);
}
RHI_API void RHI_cmdResolveQuery(RHI_CmdHandle cmd,
								 RHI_QueryPoolHandle queryPool,
								 RHI_BufferHandle readbackBuffer,
								 uint32_t startQuery,
								 uint32_t queryCount) {
	cmdResolveQuery((Cmd *) cmd,
					(QueryPool *) queryPool,
					(Buffer *) readbackBuffer,
					startQuery,
					queryCount);

}


RHI_API void RHI_cmdAddDebugMarker(RHI_CmdHandle cmd, float r, float g, float b, const char *pName) {
	cmdAddDebugMarker((Cmd *) cmd, r, g, b, pName);
}

RHI_API void RHI_cmdEndDebugMarker(RHI_CmdHandle cmd) {
	cmdEndDebugMarker((Cmd *) cmd);
}

RHI_API void RHI_queueSubmit(RHI_QueueHandle queue, const RHI_QueueSubmitDesc *desc) {
	queueSubmit((Queue *) queue, (const QueueSubmitDesc*)desc);
}
RHI_API void RHI_getTimestampFrequency(RHI_QueueHandle queue, double *pFrequency) {
	getTimestampFrequency((Queue *) queue, pFrequency);
}

RHI_API void RHI_addIndirectCommandSignature(RHI_RendererHandle handle,
											 const RHI_CommandSignatureDesc *pDesc,
											 RHI_CommandSignatureHandle *pCommandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addIndirectCommandSignature(renderer,
								(CommandSignatureDesc const *) pDesc,
								(CommandSignature **) pCommandSignature);
}

RHI_API void RHI_removeIndirectCommandSignature(RHI_RendererHandle handle,
												RHI_CommandSignatureHandle commandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeIndirectCommandSignature(renderer, (CommandSignature *) commandSignature);
}


RHI_API void RHI_calculateMemoryStats(RHI_RendererHandle handle, char **stats) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	calculateMemoryStats(renderer, stats);
}

RHI_API void RHI_freeMemoryStats(RHI_RendererHandle handle, char *stats) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	freeMemoryStats(renderer, stats);
}

RHI_API void RHI_setBufferName(RHI_RendererHandle handle,
							   RHI_BufferHandle buffer,
							   const char *pName) {

#ifndef METAL
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	setBufferName(renderer, (Buffer*)buffer, pName);
#endif
}
RHI_API void RHI_setTextureName(RHI_RendererHandle handle,
								RHI_TextureHandle texture,
								const char *pName) {
#ifndef METAL
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	setTextureName(renderer, (Texture*)texture, pName);
#endif
}
RHI_API void RHI_addSwapChain(RHI_RendererHandle handle,
							  const RHI_SwapChainDesc *pDesc,
							  RHI_SwapChainHandle *pSwapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	SwapChainDesc scDesc;
	memcpy(&scDesc, pDesc, sizeof(RHI_SwapChainDesc));

	addSwapChain(renderer, &scDesc, (SwapChain **) pSwapChain);
}


RHI_API void RHI_removeSwapChain(RHI_RendererHandle handle, RHI_SwapChainHandle swapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	removeSwapChain(renderer, (SwapChain *) swapChain);
}

RHI_API void RHI_toggleVSync(RHI_RendererHandle handle, RHI_SwapChainHandle *pSwapchain) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	toggleVSync(renderer, (SwapChain **) pSwapchain);
}

RHI_API RHI_ImageFormat RHI_getRecommendedSwapchainFormat(bool hintHDR) {
	return (RHI_ImageFormat)getRecommendedSwapchainFormat(hintHDR);
}
RHI_API void RHI_acquireNextImage(RHI_RendererHandle handle,
								  RHI_SwapChainHandle swapChain,
								  RHI_SemaphoreHandle signalSemaphore,
								  RHI_FenceHandle fence,
								  uint32_t *pImageIndex) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	acquireNextImage(renderer, (SwapChain *) swapChain, (Semaphore *) signalSemaphore, (Fence *) fence, pImageIndex);
}
RHI_API void RHI_queuePresent(RHI_QueueHandle queue, const RHI_QueuePresentDesc *desc) {

	queuePresent((Queue *) queue, (const QueuePresentDesc *) desc);
}

RHI_API RHI_RenderTargetHandle RHI_swapChainGetRenderTarget(RHI_SwapChainHandle swapChain,
															int index) {
	return (RHI_RenderTargetHandle) ((SwapChain *) swapChain)->ppRenderTargets[index];
}
RHI_API bool RHI_swapChainGetVSync(RHI_SwapChainHandle handle) {
	return ((SwapChain*)handle)->mEnableVsync;
}

RHI_API RHI_TextureHandle RHI_renderTargetGetTexture(RHI_RenderTargetHandle renderTarget) {
	return (RHI_TextureHandle) ((RenderTarget *) renderTarget)->pTexture;
}

RHI_API void RHI_renderTargetGetDesc(RHI_RenderTargetHandle renderTarget, RHI_RenderTargetDesc &desc) {
	RenderTarget *rt = (RenderTarget*)renderTarget;
	desc.width = rt->mWidth;
	desc.height = rt->mHeight;
	desc.depth = rt->mDepth;
	desc.arraySize = rt->mArraySize;
	desc.mipLevels = rt->mMipLevels;
	desc.sampleCount = (RHI_SampleCount)rt->mSampleCount;
	desc.format = (RHI_ImageFormat)rt->mFormat;
	ClearValue cv;
	memcpy(&desc.clearValue, &rt->mClearValue, sizeof(ClearValue));
	desc.sampleQuality = rt->mSampleQuality;
	desc.descriptors = (RHI_DescriptorType)rt->mDescriptors;
}

RHI_API uint32_t RHI_textureGetWidth(RHI_TextureHandle handle) {
	return ((Texture*)handle)->mWidth;
}
RHI_API uint32_t RHI_textureGetHeight(RHI_TextureHandle handle) {
	return ((Texture*)handle)->mHeight;
}
RHI_API uint32_t RHI_textureGetDepth(RHI_TextureHandle handle) {
	return ((Texture*)handle)->mDepth;
}
RHI_API uint32_t RHI_textureGetMipLevels(RHI_TextureHandle handle) {
	return ((Texture*)handle)->mMipLevels;
}

RHI_API void RHI_addBuffer(RHI_RendererHandle handle,
						   RHI_BufferDesc const *pDesc,
						   RHI_BufferHandle *pBuffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addBuffer(renderer, (BufferDesc const *) pDesc, (Buffer **) pBuffer);
}
RHI_API void RHI_addTexture(RHI_RendererHandle handle,
							RHI_TextureDesc const *pDesc,
							RHI_TextureHandle *pTexture) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	addTexture(renderer, (TextureDesc const *) pDesc, (Texture **) pTexture);
}
RHI_API void RHI_removeBuffer(RHI_RendererHandle handle, RHI_BufferHandle buffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	removeBuffer(renderer, (Buffer *) buffer);
}
RHI_API void RHI_removeTexture(RHI_RendererHandle handle, RHI_TextureHandle texture) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	removeTexture(renderer, (Texture *) texture);
}

RHI_API void RHI_mapBuffer(RHI_RendererHandle handle,
						   RHI_BufferHandle buffer,
						   RHI_ReadRange *pRange) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	mapBuffer(renderer, (Buffer *) buffer, (ReadRange *) pRange);
}

RHI_API void RHI_unmapBuffer(RHI_RendererHandle handle, RHI_BufferHandle buffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	unmapBuffer(renderer, (Buffer *) buffer);

}

RHI_API void RHI_initResourceLoaderInterface(RHI_RendererHandle handle,
											 RHI_ResourceLoaderDesc *pDesc) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}

	initResourceLoaderInterface(renderer, (ResourceLoaderDesc *) pDesc);
}
RHI_API void RHI_exitResourceLoaderInterface(RHI_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	exitResourceLoaderInterface(renderer);
}

RHI_API void RHI_addBufferResource(RHI_BufferLoadDesc *pBufferLoadDesc,
								   RHI_SyncToken *token, RHI_LoadPriority priority) {
	addResource((BufferLoadDesc *) pBufferLoadDesc, (SyncToken *) token, (LoadPriority) priority);
}
RHI_API void RHI_addTextureResource(RHI_TextureLoadDesc *pTextureLoadDesc,
									RHI_SyncToken *token, RHI_LoadPriority priority) {
	addResource((TextureLoadDesc *) pTextureLoadDesc, (SyncToken *) token, (LoadPriority) priority);
}
//RHI_API void RHI_addGeometryResource(RHI_GeometryLoadDesc const *pGeomLoadDesc,
//									  RHI_SyncToken *token, RHI_LoadPriority priority) {
//	addResource((TextureLoadDesc *) pGeomLoadDesc, (SyncToken *) token, (LoadPriority) priority);
//}
RHI_API void RHI_beginUpdateBufferResource(RHI_BufferUpdateDesc const *pBuffer) {
	beginUpdateResource((BufferUpdateDesc *) pBuffer);
}
RHI_API void RHI_beginUpdateTextureResource(RHI_TextureUpdateDesc const *pTexture) {
	beginUpdateResource((TextureUpdateDesc *) pTexture);
}
RHI_API void RHI_endUpdateBufferResource(RHI_BufferUpdateDesc *pBuffer, RHI_SyncToken *token) {
	endUpdateResource((BufferUpdateDesc *) pBuffer, (SyncToken *) token);
}
RHI_API void RHI_endUpdateTextureResource(RHI_TextureUpdateDesc *pTexture, RHI_SyncToken *token) {
	endUpdateResource((TextureUpdateDesc *) pTexture, (SyncToken *) token);
}
RHI_API void RHI_removeBufferResource(RHI_BufferHandle pBuffer) {
	removeResource((Buffer*)pBuffer);
}
RHI_API void RHI_removeTextureResource(RHI_TextureHandle pTexture) {
	removeResource((Texture*)pTexture);
}
//RHI_API void RHI_RemoveGeometryResource(RHI_GeometryHandle pGeom);

RHI_API bool RHI_allResourceLoadsCompleted() {
	return allResourceLoadsCompleted();
}
RHI_API void RHI_waitForAllResourceLoads() {
	waitForAllResourceLoads();
}

RHI_API RHI_SyncToken RHI_getLastTokenCompleted(){
	SyncToken token = getLastTokenCompleted();
	RHI_SyncToken rhiToken;
	for(int i = 0; i < RHI_LOAD_PRIORITY_COUNT; ++i)
	{
		rhiToken.waitIndex[i] = token.mWaitIndex[i];
	}
	return rhiToken;
}

RHI_API bool RHI_isTokenCompleted(const RHI_SyncToken* token){
	return isTokenCompleted((const SyncToken*) token);
}
RHI_API void RHI_waitForToken(const RHI_SyncToken* token){
	waitForToken((const SyncToken*) token);
}

RHI_API void RHI_addShaderResource(RHI_RendererHandle handle, const RHI_ShaderLoadDesc* pDesc, RHI_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return;
	}
	addShader(renderer, (const ShaderLoadDesc*)pDesc, (Shader**)pShader);
}

RHI_API RHI_RendererApi RHI_getRendererApi(RHI_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return RHI_RENDERER_API_VULKAN;
	}

	return (RHI_RendererApi) renderer->mApi;
}

RHI_API bool RHI_canShaderReadFrom(RHI_RendererHandle handle, RHI_ImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return false;
	}
	if( (int)format > (int)TinyImageFormat_Count ) {
		return false;
	}
	return renderer->pCapBits->canShaderReadFrom[(TinyImageFormat)format];
}
RHI_API bool RHI_canColorWriteTo(RHI_RendererHandle handle, RHI_ImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return false;
	}
	if( (int)format > (int)TinyImageFormat_Count ) {
		return false;
	}
	return renderer->pCapBits->canShaderWriteTo[(TinyImageFormat)format];
}

RHI_API bool RHI_canShaderWriteTo(RHI_RendererHandle handle, RHI_ImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer) {
		return false;
	}
	if( (int)format > (int)TinyImageFormat_Count ) {
		return false;
	}
	return renderer->pCapBits->canRenderTargetWriteTo[(TinyImageFormat)format];
}

/*
#if AL2O3_PLATFORM == AL2O3_PLATFORM_WINDOWS
#define API_CHK(x) ASSERT(x)
#else
*/
#define API_CHK(x) static_assert(x, "Mismatched size!")
//#endif

static void API_CHECK() {
	API_CHK(sizeof(RHI_ComputePipelineDesc) == sizeof(ComputePipelineDesc));
	API_CHK(sizeof(RHI_GraphicsPipelineDesc) == sizeof(GraphicsPipelineDesc));
	API_CHK(sizeof(RHI_RaytracingPipelineDesc) == sizeof(RaytracingPipelineDesc));

	API_CHK(offsetof(RHI_ComputePipelineDesc, shaderProgram) == offsetof(ComputePipelineDesc, pShaderProgram));
	API_CHK(offsetof(RHI_ComputePipelineDesc, rootSignature) == offsetof(ComputePipelineDesc, pRootSignature));

	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, shaderProgram) == offsetof(GraphicsPipelineDesc, pShaderProgram));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, rootSignature) == offsetof(GraphicsPipelineDesc, pRootSignature));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, pVertexLayout) == offsetof(GraphicsPipelineDesc, pVertexLayout));
	API_CHK(offsetof(RHI_GraphicsPipelineDesc, pBlendState) == offsetof(GraphicsPipelineDesc, pBlendState));
	API_CHK(offsetof(RHI_GraphicsPipelineDesc, pDepthState) == offsetof(GraphicsPipelineDesc, pDepthState));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, pRasterizerState) == offsetof(GraphicsPipelineDesc, pRasterizerState));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, pColorFormats) == offsetof(GraphicsPipelineDesc, pColorFormats));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, renderTargetCount) == offsetof(GraphicsPipelineDesc, mRenderTargetCount));
	API_CHK(offsetof(RHI_GraphicsPipelineDesc, sampleCount) == offsetof(GraphicsPipelineDesc, mSampleCount));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, sampleQuality) == offsetof(GraphicsPipelineDesc, mSampleQuality));
	API_CHK(offsetof(RHI_GraphicsPipelineDesc, depthStencilFormat)
							== offsetof(GraphicsPipelineDesc, mDepthStencilFormat));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, primitiveTopo) == offsetof(GraphicsPipelineDesc, mPrimitiveTopo));
	API_CHK(
			offsetof(RHI_GraphicsPipelineDesc, supportIndirectCommandBuffer) == offsetof(GraphicsPipelineDesc, mSupportIndirectCommandBuffer));

	API_CHK(offsetof(RHI_RaytracingPipelineDesc, raytracing) == offsetof(RaytracingPipelineDesc, pRaytracing));
	API_CHK(offsetof(RHI_RaytracingPipelineDesc, globalRootSignature)
							== offsetof(RaytracingPipelineDesc, pGlobalRootSignature));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, rayGenShader) == offsetof(RaytracingPipelineDesc, pRayGenShader));
	API_CHK(offsetof(RHI_RaytracingPipelineDesc, rayGenRootSignature)
							== offsetof(RaytracingPipelineDesc, pRayGenRootSignature));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, pMissShaders) == offsetof(RaytracingPipelineDesc, ppMissShaders));
	API_CHK(offsetof(RHI_RaytracingPipelineDesc, pMissRootSignatures)
							== offsetof(RaytracingPipelineDesc, ppMissRootSignatures));
	API_CHK(offsetof(RHI_RaytracingPipelineDesc, hitGroups) == offsetof(RaytracingPipelineDesc, pHitGroups));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, missShaderCount) == offsetof(RaytracingPipelineDesc, mMissShaderCount));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, hitGroupCount) == offsetof(RaytracingPipelineDesc, mHitGroupCount));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, payloadSize) == offsetof(RaytracingPipelineDesc, mPayloadSize));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, attributeSize) == offsetof(RaytracingPipelineDesc, mAttributeSize));
	API_CHK(offsetof(RHI_RaytracingPipelineDesc, maxTraceRecursionDepth)
							== offsetof(RaytracingPipelineDesc, mMaxTraceRecursionDepth));
	API_CHK(
			offsetof(RHI_RaytracingPipelineDesc, maxRaysCount) == offsetof(RaytracingPipelineDesc, mMaxRaysCount));

	API_CHK(sizeof(RHI_VertexLayout) == sizeof(VertexLayout));
	API_CHK(offsetof(RHI_VertexLayout, attribCount) == offsetof(VertexLayout, mAttribCount));
	API_CHK(offsetof(RHI_VertexLayout, attribs) == offsetof(VertexLayout, mAttribs));
	API_CHK(offsetof(RHI_VertexAttrib, semantic) == offsetof(VertexAttrib, mSemantic));
	API_CHK(offsetof(RHI_VertexAttrib, semanticNameLength) == offsetof(VertexAttrib, mSemanticNameLength));
	API_CHK(offsetof(RHI_VertexAttrib, semanticName) == offsetof(VertexAttrib, mSemanticName));
	API_CHK(offsetof(RHI_VertexAttrib, format) == offsetof(VertexAttrib, mFormat));
	API_CHK(offsetof(RHI_VertexAttrib, location) == offsetof(VertexAttrib, mLocation));
	API_CHK(offsetof(RHI_VertexAttrib, offset) == offsetof(VertexAttrib, mOffset));
	API_CHK(offsetof(RHI_VertexAttrib, rate) == offsetof(VertexAttrib, mRate));

	API_CHK(sizeof(RHI_RootSignatureDesc) == sizeof(RootSignatureDesc));
	API_CHK(offsetof(RHI_RootSignatureDesc, pShaders) == offsetof(RootSignatureDesc, ppShaders));
	API_CHK(offsetof(RHI_RootSignatureDesc, shaderCount) == offsetof(RootSignatureDesc, mShaderCount));
	API_CHK(
			offsetof(RHI_RootSignatureDesc, maxBindlessTextures) == offsetof(RootSignatureDesc, mMaxBindlessTextures));
	API_CHK(
			offsetof(RHI_RootSignatureDesc, ppStaticSamplerNames) == offsetof(RootSignatureDesc, ppStaticSamplerNames));
	API_CHK(offsetof(RHI_RootSignatureDesc, pStaticSamplers) == offsetof(RootSignatureDesc, ppStaticSamplers));
	API_CHK(
			offsetof(RHI_RootSignatureDesc, staticSamplerCount) == offsetof(RootSignatureDesc, mStaticSamplerCount));
	API_CHK(offsetof(RHI_RootSignatureDesc, flags) == offsetof(RootSignatureDesc, mFlags));

	API_CHK(sizeof(RHI_SamplerDesc) == sizeof(SamplerDesc));
	API_CHK(offsetof(RHI_SamplerDesc, minFilter) == offsetof(SamplerDesc, mMinFilter));
	API_CHK(offsetof(RHI_SamplerDesc, magFilter) == offsetof(SamplerDesc, mMagFilter));
	API_CHK(offsetof(RHI_SamplerDesc, mipMapMode) == offsetof(SamplerDesc, mMipMapMode));
	API_CHK(offsetof(RHI_SamplerDesc, addressU) == offsetof(SamplerDesc, mAddressU));
	API_CHK(offsetof(RHI_SamplerDesc, addressV) == offsetof(SamplerDesc, mAddressV));
	API_CHK(offsetof(RHI_SamplerDesc, addressW) == offsetof(SamplerDesc, mAddressW));
	API_CHK(offsetof(RHI_SamplerDesc, mipLodBias) == offsetof(SamplerDesc, mMipLodBias));
	API_CHK(offsetof(RHI_SamplerDesc, maxAnisotropy) == offsetof(SamplerDesc, mMaxAnisotropy));
	API_CHK(offsetof(RHI_SamplerDesc, compareFunc) == offsetof(SamplerDesc, mCompareFunc));

	API_CHK(sizeof(RHI_QueueDesc) == sizeof(QueueDesc));
	API_CHK(offsetof(RHI_QueueDesc, flags) == offsetof(QueueDesc, mFlag));
	API_CHK(offsetof(RHI_QueueDesc, priority) == offsetof(QueueDesc, mPriority));
	API_CHK(offsetof(RHI_QueueDesc, type) == offsetof(QueueDesc, mType));
	API_CHK(offsetof(RHI_QueueDesc, nodeIndex) == offsetof(QueueDesc, mNodeIndex));

	API_CHK(sizeof(RHI_BlendStateDesc) == sizeof(BlendStateDesc));
	API_CHK(offsetof(RHI_BlendStateDesc, srcFactors) == offsetof(BlendStateDesc, mSrcFactors));
	API_CHK(offsetof(RHI_BlendStateDesc, dstFactors) == offsetof(BlendStateDesc, mDstFactors));
	API_CHK(offsetof(RHI_BlendStateDesc, srcAlphaFactors) == offsetof(BlendStateDesc, mSrcAlphaFactors));
	API_CHK(offsetof(RHI_BlendStateDesc, dstAlphaFactors) == offsetof(BlendStateDesc, mDstAlphaFactors));
	API_CHK(offsetof(RHI_BlendStateDesc, blendModes) == offsetof(BlendStateDesc, mBlendModes));
	API_CHK(offsetof(RHI_BlendStateDesc, blendAlphaModes) == offsetof(BlendStateDesc, mBlendAlphaModes));
	API_CHK(offsetof(RHI_BlendStateDesc, masks) == offsetof(BlendStateDesc, mMasks));
	API_CHK(offsetof(RHI_BlendStateDesc, renderTargetMask) == offsetof(BlendStateDesc, mRenderTargetMask));
	API_CHK(offsetof(RHI_BlendStateDesc, alphaToCoverage) == offsetof(BlendStateDesc, mAlphaToCoverage));
	API_CHK(offsetof(RHI_BlendStateDesc, independentBlend) == offsetof(BlendStateDesc, mIndependentBlend));

	API_CHK(sizeof(RHI_DepthStateDesc) == sizeof(DepthStateDesc));
	API_CHK(offsetof(RHI_DepthStateDesc, depthTest) == offsetof(DepthStateDesc, mDepthTest));
	API_CHK(offsetof(RHI_DepthStateDesc, depthWrite) == offsetof(DepthStateDesc, mDepthWrite));
	API_CHK(offsetof(RHI_DepthStateDesc, depthFunc) == offsetof(DepthStateDesc, mDepthFunc));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilTest) == offsetof(DepthStateDesc, mStencilTest));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilReadMask) == offsetof(DepthStateDesc, mStencilReadMask));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilWriteMask) == offsetof(DepthStateDesc, mStencilWriteMask));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilFrontFunc) == offsetof(DepthStateDesc, mStencilFrontFunc));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilFrontFail) == offsetof(DepthStateDesc, mStencilFrontFail));
	API_CHK(offsetof(RHI_DepthStateDesc, depthFrontFail) == offsetof(DepthStateDesc, mDepthFrontFail));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilFrontPass) == offsetof(DepthStateDesc, mStencilFrontPass));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilBackFunc) == offsetof(DepthStateDesc, mStencilBackFunc));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilBackFail) == offsetof(DepthStateDesc, mStencilBackFail));
	API_CHK(offsetof(RHI_DepthStateDesc, depthBackFail) == offsetof(DepthStateDesc, mDepthBackFail));
	API_CHK(offsetof(RHI_DepthStateDesc, stencilBackPass) == offsetof(DepthStateDesc, mStencilBackPass));


	API_CHK(sizeof(RHI_RasterizerStateDesc) == sizeof(RasterizerStateDesc));
	API_CHK(offsetof(RHI_RasterizerStateDesc, cullMode) == offsetof(RasterizerStateDesc, mCullMode));
	API_CHK(offsetof(RHI_RasterizerStateDesc, depthBias) == offsetof(RasterizerStateDesc, mDepthBias));
	API_CHK(offsetof(RHI_RasterizerStateDesc, slopeScaledDepthBias)
							== offsetof(RasterizerStateDesc, mSlopeScaledDepthBias));
	API_CHK(offsetof(RHI_RasterizerStateDesc, fillMode) == offsetof(RasterizerStateDesc, mFillMode));
	API_CHK(offsetof(RHI_RasterizerStateDesc, multiSample) == offsetof(RasterizerStateDesc, mMultiSample));
	API_CHK(offsetof(RHI_RasterizerStateDesc, scissor) == offsetof(RasterizerStateDesc, mScissor));
	API_CHK(offsetof(RHI_RasterizerStateDesc, frontFace) == offsetof(RasterizerStateDesc, mFrontFace));

	API_CHK(sizeof(RHI_DescriptorSetDesc) == sizeof(DescriptorSetDesc));
	API_CHK(offsetof(RHI_DescriptorSetDesc, rootSignature) == offsetof(DescriptorSetDesc, pRootSignature));
	API_CHK(offsetof(RHI_DescriptorSetDesc, updateFrequency) == offsetof(DescriptorSetDesc, mUpdateFrequency));
	API_CHK(offsetof(RHI_DescriptorSetDesc, maxSets) == offsetof(DescriptorSetDesc, mMaxSets));
	API_CHK(offsetof(RHI_DescriptorSetDesc, nodeIndex) == offsetof(DescriptorSetDesc, mNodeIndex));

	API_CHK(sizeof(RHI_LoadActionsDesc) == sizeof(LoadActionsDesc));
	API_CHK(offsetof(RHI_LoadActionsDesc, clearColorValues) == offsetof(LoadActionsDesc, mClearColorValues));
	API_CHK(offsetof(RHI_LoadActionsDesc, loadActionsColor) == offsetof(LoadActionsDesc, mLoadActionsColor));
	API_CHK(offsetof(RHI_LoadActionsDesc, clearDepth) == offsetof(LoadActionsDesc, mClearDepth));
	API_CHK(offsetof(RHI_LoadActionsDesc, loadActionDepth) == offsetof(LoadActionsDesc, mLoadActionDepth));
	API_CHK(offsetof(RHI_LoadActionsDesc, loadActionStencil) == offsetof(LoadActionsDesc, mLoadActionStencil));

	API_CHK(sizeof(RHI_DescriptorData) == sizeof(DescriptorData));
	API_CHK(offsetof(RHI_DescriptorData, pName) == offsetof(DescriptorData, pName));
	API_CHK(offsetof(RHI_DescriptorData, pOffsets) == offsetof(DescriptorData, pOffsets));
	API_CHK(offsetof(RHI_DescriptorData, pSizes) == offsetof(DescriptorData, pSizes));
	API_CHK(offsetof(RHI_DescriptorData, mDescriptorSetBufferIndex) == offsetof(DescriptorData, mDescriptorSetBufferIndex));
	API_CHK(offsetof(RHI_DescriptorData, mDescriptorSetShader) == offsetof(DescriptorData, mDescriptorSetShader));
	API_CHK(offsetof(RHI_DescriptorData, mDescriptorSetShaderStage) == offsetof(DescriptorData, mDescriptorSetShaderStage));
	API_CHK(offsetof(RHI_DescriptorData, UAVMipSlice) == offsetof(DescriptorData, mUAVMipSlice));
	API_CHK(offsetof(RHI_DescriptorData, bindStencilResource) == offsetof(DescriptorData, mBindStencilResource));
	API_CHK(offsetof(RHI_DescriptorData, pTextures) == offsetof(DescriptorData, ppTextures));
	API_CHK(offsetof(RHI_DescriptorData, pSamplers) == offsetof(DescriptorData, ppSamplers));
	API_CHK(offsetof(RHI_DescriptorData, pBuffers) == offsetof(DescriptorData, ppBuffers));
	API_CHK(offsetof(RHI_DescriptorData, pPipelines) == offsetof(DescriptorData, ppPipelines));
	API_CHK(offsetof(RHI_DescriptorData, pDescriptorSet) == offsetof(DescriptorData, ppDescriptorSet));
	API_CHK(
			offsetof(RHI_DescriptorData, pAccelerationStructures) == offsetof(DescriptorData, ppAccelerationStructures));
	API_CHK(offsetof(RHI_DescriptorData, count) == offsetof(DescriptorData, mCount));
	API_CHK(offsetof(RHI_DescriptorData, index) == offsetof(DescriptorData, mIndex));
	API_CHK(offsetof(RHI_DescriptorData, extractBuffer) == offsetof(DescriptorData, mExtractBuffer));

	API_CHK(sizeof(RHI_BufferBarrier) == sizeof(BufferBarrier));
	API_CHK(offsetof(RHI_BufferBarrier, buffer) == offsetof(BufferBarrier, pBuffer));
	API_CHK(offsetof(RHI_BufferBarrier, newState) == offsetof(BufferBarrier, mNewState));
	API_CHK(offsetof(RHI_BufferBarrier, split) == offsetof(BufferBarrier, mSplit));


	API_CHK(sizeof(RHI_TextureBarrier) == sizeof(TextureBarrier));
	API_CHK(offsetof(RHI_TextureBarrier, texture) == offsetof(TextureBarrier, pTexture));
	API_CHK(offsetof(RHI_TextureBarrier, newState) == offsetof(TextureBarrier, mNewState));
	API_CHK(offsetof(RHI_TextureBarrier, split) == offsetof(TextureBarrier, mSplit));

	API_CHK(sizeof(RHI_QueryPoolDesc) == sizeof(QueryPoolDesc));
	API_CHK(offsetof(RHI_QueryPoolDesc, type) == offsetof(QueryPoolDesc, mType));
	API_CHK(offsetof(RHI_QueryPoolDesc, queryCount) == offsetof(QueryPoolDesc, mQueryCount));
	API_CHK(offsetof(RHI_QueryPoolDesc, nodeIndex) == offsetof(QueryPoolDesc, mNodeIndex));

	API_CHK(sizeof(RHI_QueryDesc) == sizeof(QueryDesc));
	API_CHK(offsetof(RHI_QueryDesc, index) == offsetof(QueryDesc, mIndex));

	API_CHK(sizeof(RHI_CommandSignatureDesc) == sizeof(CommandSignatureDesc));
	API_CHK(offsetof(RHI_CommandSignatureDesc, cmdPool) == offsetof(CommandSignatureDesc, pCmdPool));
	API_CHK(
			offsetof(RHI_CommandSignatureDesc, rootSignature) == offsetof(CommandSignatureDesc, pRootSignature));
	API_CHK(
			offsetof(RHI_CommandSignatureDesc, indirectArgCount) == offsetof(CommandSignatureDesc, mIndirectArgCount));
	API_CHK(offsetof(RHI_CommandSignatureDesc, pArgDescs) == offsetof(CommandSignatureDesc, pArgDescs));


	API_CHK(sizeof(RHI_RectDesc) == sizeof(RectDesc));
	API_CHK(offsetof(RHI_RectDesc, left) == offsetof(RectDesc, left));
	API_CHK(offsetof(RHI_RectDesc, top) == offsetof(RectDesc, top));
	API_CHK(offsetof(RHI_RectDesc, right) == offsetof(RectDesc, right));
	API_CHK(offsetof(RHI_RectDesc, bottom) == offsetof(RectDesc, bottom));

	API_CHK(sizeof(RHI_SwapChainDesc) == sizeof(SwapChainDesc));
	API_CHK(offsetof(RHI_SwapChainDesc, windowHandle) == offsetof(SwapChainDesc, mWindowHandle));
	API_CHK(offsetof(RHI_SwapChainDesc, pPresentQueues) == offsetof(SwapChainDesc, ppPresentQueues));
	API_CHK(offsetof(RHI_SwapChainDesc, presentQueueCount) == offsetof(SwapChainDesc, mPresentQueueCount));
	API_CHK(offsetof(RHI_SwapChainDesc, imageCount) == offsetof(SwapChainDesc, mImageCount));
	API_CHK(offsetof(RHI_SwapChainDesc, width) == offsetof(SwapChainDesc, mWidth));
	API_CHK(offsetof(RHI_SwapChainDesc, height) == offsetof(SwapChainDesc, mHeight));
	API_CHK(offsetof(RHI_SwapChainDesc, colorFormat) == offsetof(SwapChainDesc, mColorFormat));
	API_CHK(offsetof(RHI_SwapChainDesc, colorClearValue) == offsetof(SwapChainDesc, mColorClearValue));
	API_CHK(offsetof(RHI_SwapChainDesc, enableVsync) == offsetof(SwapChainDesc, mEnableVsync));

	API_CHK(sizeof(RHI_RawImageData) == sizeof(RawImageData));
	API_CHK(offsetof(RHI_RawImageData, pRawData) == offsetof(RawImageData, pRawData));
	API_CHK(offsetof(RHI_RawImageData, format) == offsetof(RawImageData, mFormat));
	API_CHK(offsetof(RHI_RawImageData, width) == offsetof(RawImageData, mWidth));
	API_CHK(offsetof(RHI_RawImageData, height) == offsetof(RawImageData, mHeight));
	API_CHK(offsetof(RHI_RawImageData, depth) == offsetof(RawImageData, mDepth));
	API_CHK(offsetof(RHI_RawImageData, arraySize) == offsetof(RawImageData, mArraySize));
	API_CHK(offsetof(RHI_RawImageData, mipLevels) == offsetof(RawImageData, mMipLevels));
	API_CHK(offsetof(RHI_RawImageData, mipsAfterSlices) == offsetof(RawImageData, mMipsAfterSlices));
	API_CHK(offsetof(RHI_RawImageData, mRowStride) == offsetof(RawImageData, mRowStride));

	API_CHK(sizeof(RHI_ShaderReflection) == sizeof(ShaderReflection));
	API_CHK(sizeof(RHI_ShaderVariable) == sizeof(ShaderVariable));
	API_CHK(sizeof(RHI_ClearValue) == sizeof(ClearValue));

	API_CHK(sizeof(RHI_PipelineReflection) == sizeof(PipelineReflection));
	API_CHK(offsetof(RHI_PipelineReflection, mShaderStages) == offsetof(PipelineReflection, mShaderStages));
	API_CHK(offsetof(RHI_PipelineReflection, mStageReflections) == offsetof(PipelineReflection, mStageReflections));
	API_CHK(offsetof(RHI_PipelineReflection, mStageReflectionCount)
							== offsetof(PipelineReflection, mStageReflectionCount));
	API_CHK(offsetof(RHI_PipelineReflection, mVertexStageIndex) == offsetof(PipelineReflection, mVertexStageIndex));
	API_CHK(offsetof(RHI_PipelineReflection, mHullStageIndex) == offsetof(PipelineReflection, mHullStageIndex));
	API_CHK(offsetof(RHI_PipelineReflection, mDomainStageIndex) == offsetof(PipelineReflection, mDomainStageIndex));
	API_CHK(
			offsetof(RHI_PipelineReflection, mGeometryStageIndex) == offsetof(PipelineReflection, mGeometryStageIndex));
	API_CHK(offsetof(RHI_PipelineReflection, mPixelStageIndex) == offsetof(PipelineReflection, mPixelStageIndex));
	API_CHK(offsetof(RHI_PipelineReflection, pShaderResources) == offsetof(PipelineReflection, pShaderResources));
	API_CHK(offsetof(RHI_PipelineReflection, mShaderResourceCount)
							== offsetof(PipelineReflection, mShaderResourceCount));
	API_CHK(offsetof(RHI_PipelineReflection, pVariables) == offsetof(PipelineReflection, pVariables));
	API_CHK(offsetof(RHI_PipelineReflection, mVariableCount) == offsetof(PipelineReflection, mVariableCount));

	API_CHK(sizeof(RHI_BufferDesc) == sizeof(BufferDesc));
	API_CHK(offsetof(RHI_BufferDesc, mSize) == offsetof(BufferDesc, mSize));
	API_CHK(offsetof(RHI_BufferDesc, mAlignment) == offsetof(BufferDesc, mAlignment));
	API_CHK(offsetof(RHI_BufferDesc, mMemoryUsage) == offsetof(BufferDesc, mMemoryUsage));
	API_CHK(offsetof(RHI_BufferDesc, mFlags) == offsetof(BufferDesc, mFlags));
	API_CHK(offsetof(RHI_BufferDesc, mStartState) == offsetof(BufferDesc, mStartState));
	API_CHK(offsetof(RHI_BufferDesc, mFirstElement) == offsetof(BufferDesc, mFirstElement));
	API_CHK(offsetof(RHI_BufferDesc, mElementCount) == offsetof(BufferDesc, mElementCount));
	API_CHK(offsetof(RHI_BufferDesc, mStructStride) == offsetof(BufferDesc, mStructStride));
	API_CHK(offsetof(RHI_BufferDesc, mICBDrawType) == offsetof(BufferDesc, mICBDrawType));
	API_CHK(offsetof(RHI_BufferDesc, mICBMaxVertexBufferBind) == offsetof(BufferDesc, mICBMaxVertexBufferBind));
	API_CHK(offsetof(RHI_BufferDesc, mICBMaxFragmentBufferBind) == offsetof(BufferDesc, mICBMaxFragmentBufferBind));
	API_CHK(offsetof(RHI_BufferDesc, counterBuffer) == offsetof(BufferDesc, pCounterBuffer));
	API_CHK(offsetof(RHI_BufferDesc, mFormat) == offsetof(BufferDesc, mFormat));
	API_CHK(offsetof(RHI_BufferDesc, mDescriptors) == offsetof(BufferDesc, mDescriptors));
	API_CHK(offsetof(RHI_BufferDesc, pDebugName) == offsetof(BufferDesc, pDebugName));
	API_CHK(offsetof(RHI_BufferDesc, pSharedNodeIndices) == offsetof(BufferDesc, pSharedNodeIndices));
	API_CHK(offsetof(RHI_BufferDesc, mNodeIndex) == offsetof(BufferDesc, mNodeIndex));
	API_CHK(offsetof(RHI_BufferDesc, mSharedNodeIndexCount) == offsetof(BufferDesc, mSharedNodeIndexCount));

	API_CHK(sizeof(RHI_TextureLoadDesc) == sizeof(TextureLoadDesc));
	API_CHK(offsetof(RHI_TextureLoadDesc, pTexture) == offsetof(TextureLoadDesc, ppTexture));
	API_CHK(offsetof(RHI_TextureLoadDesc, pDesc) == offsetof(TextureLoadDesc, pDesc));
	API_CHK(offsetof(RHI_TextureLoadDesc, mNodeIndex) == offsetof(TextureLoadDesc, mNodeIndex));
	API_CHK(offsetof(RHI_TextureLoadDesc, pRawImageData) == offsetof(TextureLoadDesc, pRawImageData));
	API_CHK(offsetof(RHI_TextureLoadDesc, pBinaryImageData) == offsetof(TextureLoadDesc, pBinaryImageData));
	API_CHK(offsetof(RHI_TextureLoadDesc, mCreationFlag) == offsetof(TextureLoadDesc, mCreationFlag));

	API_CHK(sizeof(RHI_TextureDesc) == sizeof(TextureDesc));
	API_CHK(offsetof(RHI_TextureDesc, mFlags) == offsetof(TextureDesc, mFlags));
	API_CHK(offsetof(RHI_TextureDesc, mWidth) == offsetof(TextureDesc, mWidth));
	API_CHK(offsetof(RHI_TextureDesc, mHeight) == offsetof(TextureDesc, mHeight));
	API_CHK(offsetof(RHI_TextureDesc, mDepth) == offsetof(TextureDesc, mDepth));
	API_CHK(offsetof(RHI_TextureDesc, mArraySize) == offsetof(TextureDesc, mArraySize));
	API_CHK(offsetof(RHI_TextureDesc, mMipLevels) == offsetof(TextureDesc, mMipLevels));
	API_CHK(offsetof(RHI_TextureDesc, mSampleCount) == offsetof(TextureDesc, mSampleCount));
	API_CHK(offsetof(RHI_TextureDesc, mSampleQuality) == offsetof(TextureDesc, mSampleQuality));
	API_CHK(offsetof(RHI_TextureDesc, mFormat) == offsetof(TextureDesc, mFormat));
	API_CHK(offsetof(RHI_TextureDesc, mClearValue) == offsetof(TextureDesc, mClearValue));
	API_CHK(offsetof(RHI_TextureDesc, mStartState) == offsetof(TextureDesc, mStartState));
	API_CHK(offsetof(RHI_TextureDesc, mDescriptors) == offsetof(TextureDesc, mDescriptors));
	API_CHK(offsetof(RHI_TextureDesc, pNativeHandle) == offsetof(TextureDesc, pNativeHandle));
	API_CHK(offsetof(RHI_TextureDesc, pDebugName) == offsetof(TextureDesc, pDebugName));
	API_CHK(offsetof(RHI_TextureDesc, pSharedNodeIndices) == offsetof(TextureDesc, pSharedNodeIndices));
	API_CHK(offsetof(RHI_TextureDesc, mSharedNodeIndexCount) == offsetof(TextureDesc, mSharedNodeIndexCount));
	API_CHK(offsetof(RHI_TextureDesc, mNodeIndex) == offsetof(TextureDesc, mNodeIndex));
	API_CHK(offsetof(RHI_TextureDesc, mHostVisible) == offsetof(TextureDesc, mHostVisible));
}
#undef API_CHK

#else // !defined(RHI_SKIP_DECLARATIONS)

#ifdef _MSC_VER
#  define snprintf _snprintf
#  define popen _popen
#  define pclose _pclose
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#  include <windows.h>

/* Utility macros. */

typedef HMODULE DynamicLibrary;

#  define dynamic_library_open(path)         LoadLibrary(path)
#  define dynamic_library_close(lib)         FreeLibrary(lib)
#  define dynamic_library_find(lib, symbol)  GetProcAddress(lib, symbol)
#else
#  include <dlfcn.h>

typedef void* DynamicLibrary;

#  define dynamic_library_open(path)         dlopen(path, RTLD_NOW)
#  define dynamic_library_close(lib)         dlclose(lib)
#  define dynamic_library_find(lib, symbol)  dlsym(lib, symbol)
#endif

#define DLL_LIBRARY_FIND_CHECKED(name)         name = (t##name *)dynamic_library_find(lib, #name);         assert(name);

#define DLL_LIBRARY_FIND(name)         name = (t##name *)dynamic_library_find(lib, #name);

static DynamicLibrary lib;

#define DLL_FUNC(name) t##name *##name;

#define DLL_FUNCS() \
    DLL_FUNC(RHI_rendererCreate); \
    DLL_FUNC(RHI_rendererDestroy); \
    DLL_FUNC(RHI_addFence); \
    DLL_FUNC(RHI_removeFence); \
    DLL_FUNC(RHI_addSemaphore); \
    DLL_FUNC(RHI_removeSemaphore); \
    DLL_FUNC(RHI_addQueue); \
    DLL_FUNC(RHI_removeQueue); \
    DLL_FUNC(RHI_addCmdPool); \
    DLL_FUNC(RHI_removeCmdPool); \
    DLL_FUNC(RHI_addCmd); \
    DLL_FUNC(RHI_removeCmd); \
    DLL_FUNC(RHI_addCmd_n); \
    DLL_FUNC(RHI_removeCmd_n); \
    DLL_FUNC(RHI_addRenderTarget); \
    DLL_FUNC(RHI_removeRenderTarget); \
    DLL_FUNC(RHI_addSampler); \
    DLL_FUNC(RHI_removeSampler); \
    DLL_FUNC(RHI_addShader); \
    DLL_FUNC(RHI_addShaderBinary); \
    DLL_FUNC(RHI_removeShader); \
    DLL_FUNC(RHI_addRootSignature); \
    DLL_FUNC(RHI_removeRootSignature); \
    DLL_FUNC(RHI_addPipeline); \
    DLL_FUNC(RHI_removePipeline); \
    DLL_FUNC(RHI_addDescriptorSet); \
    DLL_FUNC(RHI_removeDescriptorSet); \
    DLL_FUNC(RHI_updateDescriptorSet); \
    DLL_FUNC(RHI_addQueryPool); \
    DLL_FUNC(RHI_removeQueryPool); \
    DLL_FUNC(RHI_addBuffer); \
    DLL_FUNC(RHI_addTexture); \
    DLL_FUNC(RHI_removeBuffer); \
    DLL_FUNC(RHI_removeTexture); \
    DLL_FUNC(RHI_beginCmd); \
    DLL_FUNC(RHI_endCmd); \
    DLL_FUNC(RHI_cmdBindRenderTargets); \
    DLL_FUNC(RHI_cmdSetViewport); \
    DLL_FUNC(RHI_cmdSetScissor); \
    DLL_FUNC(RHI_cmdBindPipeline); \
    DLL_FUNC(RHI_cmdBindDescriptorSet); \
    DLL_FUNC(RHI_cmdBindPushConstants); \
    DLL_FUNC(RHI_cmdBindPushConstantsByIndex); \
    DLL_FUNC(RHI_cmdBindIndexBuffer); \
    DLL_FUNC(RHI_cmdBindVertexBuffer); \
    DLL_FUNC(RHI_cmdDraw); \
    DLL_FUNC(RHI_cmdDrawInstanced); \
    DLL_FUNC(RHI_cmdDrawIndexed); \
    DLL_FUNC(RHI_cmdDrawIndexedInstanced); \
    DLL_FUNC(RHI_cmdDispatch); \
    DLL_FUNC(RHI_cmdResourceBarrier); \
    DLL_FUNC(RHI_cmdBeginDebugMarker); \
    DLL_FUNC(RHI_cmdExecuteIndirect); \
    DLL_FUNC(RHI_cmdBeginQuery); \
    DLL_FUNC(RHI_cmdEndQuery); \
    DLL_FUNC(RHI_cmdResolveQuery); \
    DLL_FUNC(RHI_cmdEndDebugMarker); \
    DLL_FUNC(RHI_cmdAddDebugMarker); \
    DLL_FUNC(RHI_queueSubmit); \
    DLL_FUNC(RHI_getTimestampFrequency); \
    DLL_FUNC(RHI_waitQueueIdle); \
    DLL_FUNC(RHI_getFenceStatus); \
    DLL_FUNC(RHI_waitForFences); \
    DLL_FUNC(RHI_addIndirectCommandSignature); \
    DLL_FUNC(RHI_removeIndirectCommandSignature); \
    DLL_FUNC(RHI_calculateMemoryStats); \
    DLL_FUNC(RHI_freeMemoryStats); \
    DLL_FUNC(RHI_setBufferName); \
    DLL_FUNC(RHI_setTextureName); \
    DLL_FUNC(RHI_mapBuffer); \
    DLL_FUNC(RHI_unmapBuffer); \
    DLL_FUNC(RHI_addSwapChain); \
    DLL_FUNC(RHI_removeSwapChain); \
    DLL_FUNC(RHI_toggleVSync); \
    DLL_FUNC(RHI_getRecommendedSwapchainFormat); \
    DLL_FUNC(RHI_acquireNextImage); \
    DLL_FUNC(RHI_queuePresent); \
    DLL_FUNC(RHI_swapChainGetRenderTarget); \
    DLL_FUNC(RHI_renderTargetGetTexture); \
    DLL_FUNC(RHI_renderTargetGetDesc); \
    DLL_FUNC(RHI_getRendererApi); \
    DLL_FUNC(RHI_canShaderReadFrom); \
    DLL_FUNC(RHI_canColorWriteTo); \
    DLL_FUNC(RHI_canShaderWriteTo); \
    DLL_FUNC(RHI_shaderGetPipelineReflection); \
    DLL_FUNC(RHI_initResourceLoaderInterface); \
    DLL_FUNC(RHI_removeResourceLoaderInterface); \
    DLL_FUNC(RHI_addShader); \
    DLL_FUNC(RHI_addBufferResource); \
    DLL_FUNC(RHI_addTextureResource); \
    DLL_FUNC(RHI_addGeometryResource); \
    DLL_FUNC(RHI_beginUpdateBufferResource); \
    DLL_FUNC(RHI_beginUpdateTextureResource); \
    DLL_FUNC(RHI_endUpdateBufferResource); \
    DLL_FUNC(RHI_endUpdateTextureResource); \
    DLL_FUNC(RHI_removeBufferResource); \
    DLL_FUNC(RHI_removeTextureResource); \
    DLL_FUNC(RHI_removeGeometryResource); \
    DLL_FUNC(RHI_allResourceLoadsCompleted); \
    DLL_FUNC(RHI_waitForAllResourceLoads); \
    DLL_FUNC(RHI_getLastTokenCompleted); \
    DLL_FUNC(RHI_isTokenCompleted); \
    DLL_FUNC(RHI_waitForToken); \

DLL_FUNCS();

#undef DLL_FUNC

#define DLL_FUNC(name) DLL_LIBRARY_FIND(##name);

const char *getRendererDLL(RHI_RendererApi renderer)
{
    switch(renderer)
    {
#ifdef _WIN32
        case RHI_API_D3D12:
            return "RHI_D3D12.dll";
        case RHI_API_VULKAN:
            return "RHI_vulkan.dll";
        case RHI_API_D3D11:
            return "RHI_D3D11.dll";
#elif defined(__APPLE__)
        case RHI_API_METAL:
            return "RHI_metal.dylib";
        case RHI_API_VULKAN:
            return "RHI_vulkan.dylib";
#else
        case RHI_API_VULKAN:
            return "RHI_vulkan.so";
#endif
        default:
            return NULL;
    }
}

/* Implementation function. */
RHI_Result RHI_init(RHI_RendererApi renderer)
{
	static int initialized = 0;
	static RHI_Result result = RHI_RESULT_SUCCESS;

	if(initialized)
	{
		return result;
	}

	initialized = 1;

	/* Load library. */
	lib = dynamic_library_open(getRendererDLL(renderer));

	if (lib == NULL)
	{
		result = RHI_RESULT_ERROR;
		return result;
	}

	DLL_FUNCS();

	return result;
}

void RHI_exit(void)
{
	if(lib != NULL)
	{
		/*  Ignore errors. */
		dynamic_library_close(lib);
		lib = NULL;
	}
}

#undef DLL_FUNC

#endif
