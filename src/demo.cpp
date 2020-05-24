//-----------------------------------------------------------------------------
// Copyright 2020 Tim Barnes
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//----------------------------------------------------------------------------

#include "demo.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h> 
#include <OS/Interfaces/ILog.h>
#include <OS/Interfaces/IInput.h>

//The-forge memory allocator
extern bool MemAllocInit();
extern void MemAllocExit();

Demo::~Demo()
{
	
	if (mRenderer != NULL)
	{
		RHI_waitQueueIdle(mGraphicsQueue);
		mAppUI.Unload();
		mAppUI.Exit();

		//resources
		RHI_removeTextureResource(mTexture);
		RHI_removeBufferResource(mVertexBuffer);
		RHI_removeBufferResource(mIndexBuffer);
		//forge objects
		RHI_removeShader(mRenderer, mShader);
		RHI_removeRootSignature(mRenderer, mRootSignature);
		RHI_removeDescriptorSet(mRenderer, mDescriptorSet);
		RHI_removePipeline(mRenderer, mGraphicsPipeline);
		RHI_removeSampler(mRenderer, mSampler);
		RHI_removeSwapChain(mRenderer, mSwapChain);
		RHI_removeRenderTarget(mRenderer, mDepthBuffer);

		for (uint32_t i = 0; i < gImageCount; ++i)
		{
			RHI_removeFence(mRenderer, mRenderCompleteFences[i]);
			RHI_removeSemaphore(mRenderer, mRenderCompleteSemaphores[i]);
		}

		RHI_removeSemaphore(mRenderer, mImageAcquiredSemaphore);
		RHI_removeCmd_n(mRenderer, gImageCount, mCmds);
		RHI_removeCmdPool(mRenderer, mCmdPool);
		RHI_removeQueue(mRenderer, mGraphicsQueue);

		RHI_exitResourceLoaderInterface(mRenderer);
		RHI_removeRenderer(mRenderer);
	}

	Log::Exit();
   fsExitAPI();
	MemAllocExit();
}

bool Demo::init(GLFWwindow *pWindow)
{
	//store the window pointer
	mWindow = pWindow;

	//init memory allocator
	if (!MemAllocInit())
	{
		printf("Failed to init memory allocator\n");
		return false;
	}

	//init file system
	if (!fsInitAPI())
	{
		printf("Failed to init file system\n");
		return false;
	}

	//init the log
	Log::Init();

	//set the root folder path
	PathHandle programDirectory = fsGetApplicationDirectory();
	if (!fsPlatformUsesBundledResources())
	{
      fsSetResourceDirRootPath(programDirectory);
	}
	else
	{
      LOGF(LogLevel::eERROR, "We don't support bundled resources");
		return false;
	}

	//work out which api we are using
	RHI_RendererApi api;
#if defined(VULKAN)
	api = RHI_RENDERER_API_VULKAN;
#elif defined(DIRECT3D12)
	api = RHI_RENDERER_API_D3D12;
#else
	#error Trying to use a renderer API not supported by this demo
#endif

	//set directories for the selected api
	switch (api)
	{
	case RHI_RENDERER_API_D3D12:
      fsSetRelativePathForResourceDirEnum(RD_SHADER_SOURCES, "shaders/d3d12/");
      fsSetRelativePathForResourceDirEnum(RD_SHADER_BINARIES, "shaders/d3d12/binary/");
		break;
	case RHI_RENDERER_API_VULKAN:
      fsSetRelativePathForResourceDirEnum(RD_SHADER_SOURCES, "shaders/vulkan/");
      fsSetRelativePathForResourceDirEnum(RD_SHADER_BINARIES, "shaders/vulkan/binary/");
		break;
	default:
      LOGF(LogLevel::eERROR, "No support for this API");
		return false;
	}

	//set texture dir
	fsSetRelativePathForResourceDirEnum(RD_TEXTURES, "textures/");
	//set font dir
	fsSetRelativePathForResourceDirEnum(RD_BUILTIN_FONTS, "fonts/");
	//set GPUCfg dir
	fsSetRelativePathForResourceDirEnum(RD_GPU_CONFIG, "gpucfg/");
	//set UI dir
	fsSetRelativePathForResourceDirEnum(RD_MIDDLEWARE_UI, "ui/");
	//set Text dir
	fsSetRelativePathForResourceDirEnum(RD_MIDDLEWARE_TEXT, "text/");

	//get framebuffer size, it may be different from window size
	glfwGetFramebufferSize(pWindow, &mFbWidth, &mFbHeight);

	//init renderer interface
	RHI_RendererDesc rendererDesc = {};
	//rendererDesc.api = api;
	rendererDesc.gpuMode = RHI_GPU_MODE_SINGLE;
	rendererDesc.shaderTarget = RHI_SHADER_TARGET_5_1; //5_1 will do for this demo

	mRenderer = RHI_initRenderer("The-Forge Demo", &rendererDesc);
	if (mRenderer == NULL)
		return false;

	//init resource loader interface
	RHI_initResourceLoaderInterface(mRenderer, nullptr);

	//create graphics queue
	RHI_QueueDesc queueDesc = {};
	queueDesc.type = RHI_QUEUE_TYPE_GRAPHICS;
	queueDesc.flags = RHI_QUEUE_FLAG_NONE;//use RHI_QUEUE_FLAG_INIT_MICROPROFILE to enable profiling;
	RHI_addQueue(mRenderer, &queueDesc, &mGraphicsQueue);

	//create command pool for the graphics queue
	RHI_CmdPoolDesc cmdPoolDesc = {};
	cmdPoolDesc.queue = mGraphicsQueue;
	RHI_addCmdPool(mRenderer, &cmdPoolDesc, &mCmdPool);

	//create command buffer for each potential swapchain image
	RHI_CmdDesc cmdDesc = {};
	cmdDesc.pool = mCmdPool;
	RHI_addCmd_n(mRenderer, &cmdDesc, gImageCount, &mCmds);

	//create sync objects
	for (uint32_t i = 0; i < gImageCount; ++i)
	{
		RHI_addFence(mRenderer, &mRenderCompleteFences[i]);
		RHI_addSemaphore(mRenderer, &mRenderCompleteSemaphores[i]);
	}
	RHI_addSemaphore(mRenderer, &mImageAcquiredSemaphore);

	//UI - create before swapchain as createSwapchainResources calls into mAppUI
	if (!mAppUI.Init((Renderer *)mRenderer))
		return false;

	mAppUI.LoadFont("TitilliumText/TitilliumText-Bold.otf", RD_BUILTIN_FONTS);

	//Load action for the render and depth target
	mLoadActions.loadActionsColor[0] = RHI_LOAD_ACTION_CLEAR;
	mLoadActions.clearColorValues[0].r = 0.2f;
	mLoadActions.clearColorValues[0].g = 0.2f;
	mLoadActions.clearColorValues[0].b = 0.2f;
	mLoadActions.clearColorValues[0].a = 0.0f;
	mLoadActions.loadActionDepth = RHI_LOAD_ACTION_CLEAR;
	mLoadActions.clearDepth.depth = 1.0f;
	mLoadActions.clearDepth.stencil = 0;

	//create swapchain and depth buffer
	if (!createSwapchainResources())
		return false;


	//vertex buffer
	{
		const eastl::vector<Vertex> vertices =
		{
			Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
			Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
			Vertex(1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
			Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

			Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f),
			Vertex(1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
			Vertex(1.0f,  1.0f, 1.0f, 0.0f, 0.0f),
			Vertex(-1.0f,  1.0f, 1.0f, 1.0f, 0.0f),

			Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f),
			Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f),
			Vertex(1.0f, 1.0f,  1.0f, 1.0f, 0.0f),
			Vertex(1.0f, 1.0f, -1.0f, 1.0f, 1.0f),

			Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),
			Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
			Vertex(1.0f, -1.0f,  1.0f, 0.0f, 0.0f),
			Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f),

			Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f),
			Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f),
			Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
			Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

			Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
			Vertex(1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
			Vertex(1.0f,  1.0f,  1.0f, 1.0f, 0.0f),
			Vertex(1.0f, -1.0f,  1.0f, 1.0f, 1.0f),
		};

		RHI_BufferLoadDesc desc = {};
		desc.pBuffer = &mVertexBuffer;
		desc.pData = vertices.data();
		desc.mDesc.mDescriptors = RHI_DESCRIPTOR_TYPE_VERTEX_BUFFER;
		desc.mDesc.mSize = vertices.size() * sizeof(Vertex);
		desc.mDesc.mMemoryUsage = RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY;

		RHI_addBufferResource(&desc, NULL, RHI_LOAD_PRIORITY_NORMAL);
	}

	//index buffer
	{
		const eastl::vector<uint16_t> indices =
		{
			0,  1,  2,
			0,  2,  3,
			4,  5,  6,
			4,  6,  7,
			8,  9, 10,
			8, 10, 11,
			12, 13, 14,
			12, 14, 15,
			16, 17, 18,
			16, 18, 19,
			20, 21, 22,
			20, 22, 23
		};

		mIndexCount = (uint16_t)indices.size();

		RHI_BufferLoadDesc desc = {};
		desc.pBuffer = &mIndexBuffer;
		desc.pData = indices.data();
		desc.mDesc.mDescriptors = RHI_DESCRIPTOR_TYPE_INDEX_BUFFER;
		desc.mDesc.mSize = mIndexCount * sizeof(uint16_t);
		desc.mDesc.mMemoryUsage = RHI_RESOURCE_MEMORY_USAGE_GPU_ONLY;

		RHI_addBufferResource(&desc, NULL, RHI_LOAD_PRIORITY_NORMAL);
	}

	//texture
	{
		PathHandle path = fsGetPathInResourceDirEnum(RD_TEXTURES, "the-forge.dds");
		RHI_TextureLoadDesc desc = {};
		desc.pTexture = &mTexture;
		desc.pFilePath = path;

		RHI_addTextureResource(&desc, NULL, RHI_LOAD_PRIORITY_NORMAL);
	}

	//sampler 
	{
		//trilinear
		RHI_SamplerDesc desc = { RHI_FILTER_LINEAR, RHI_FILTER_LINEAR, RHI_MIPMAP_MODE_LINEAR,
							 RHI_ADDRESS_MODE_CLAMP_TO_EDGE, RHI_ADDRESS_MODE_CLAMP_TO_EDGE, RHI_ADDRESS_MODE_CLAMP_TO_EDGE };

		RHI_addSampler(mRenderer, &desc, &mSampler);
	}

	//shader
	{
		RHI_ShaderLoadDesc desc = {};
		desc.mStages[0] = { "demo.vert", NULL, 0, RHI_RD_SHADER_SOURCES };
		desc.mStages[1] = { "demo.frag", NULL, 0, RHI_RD_SHADER_SOURCES };
		desc.mTarget = RHI_SHADER_TARGET_5_1;

		RHI_addShaderResource(mRenderer, &desc, &mShader);
	}

	//root signature
	{
		const char* pStaticSamplers[] = { "samplerState0" };
		RHI_RootSignatureDesc desc = {};
		desc.staticSamplerCount = 1;
		desc.ppStaticSamplerNames = pStaticSamplers;
		desc.pStaticSamplers = &mSampler;
		desc.shaderCount = 1;
		desc.pShaders = &mShader;

		RHI_addRootSignature(mRenderer, &desc, &mRootSignature);
	}

	//wait for our resource loads to complete, we need the texture for the descriptor set
	RHI_waitForAllResourceLoads();

	//descriptor set
	{
		//create the descriptorset
		RHI_DescriptorSetDesc desc = { mRootSignature, RHI_DESCRIPTOR_UPDATE_FREQ_NONE, 1 };
		RHI_addDescriptorSet(mRenderer, &desc, &mDescriptorSet);

		//now update the data
		RHI_DescriptorData params[1] = {};
		params[0].pName = "texture0";
		params[0].pTextures = &mTexture;
		RHI_updateDescriptorSet(mRenderer, 0, mDescriptorSet, 1, params);
	}

	//pipeline state object
	{
		//vertex layout
		RHI_VertexLayout vertexLayout = {};
		vertexLayout.attribCount = 2;
		vertexLayout.attribs[0].semantic = RHI_SEMANTIC_POSITION;
		vertexLayout.attribs[0].format = RHI_IMAGE_FORMAT_R32G32B32_SFLOAT;
		vertexLayout.attribs[0].binding = 0;
		vertexLayout.attribs[0].location = 0;
		vertexLayout.attribs[0].offset = 0;
		vertexLayout.attribs[1].semantic = RHI_SEMANTIC_TEXCOORD0;
		vertexLayout.attribs[1].format = RHI_IMAGE_FORMAT_R32G32_SFLOAT;
		vertexLayout.attribs[1].binding = 0;
		vertexLayout.attribs[1].location = 1;
		vertexLayout.attribs[1].offset = 12;

		//rasterizer
		RHI_RasterizerStateDesc rasterizerStateDesc = {};
		rasterizerStateDesc.cullMode = RHI_CULL_MODE_BACK;

		//depth state
		RHI_DepthStateDesc depthStateDesc = {};
		depthStateDesc.depthTest = true;
		depthStateDesc.depthWrite = true;
		depthStateDesc.depthFunc = RHI_CMP_LEQUAL;

		//pipeline
		RHI_PipelineDesc desc = {};
		desc.type = RHI_PIPELINE_TYPE_GRAPHICS;
		RHI_GraphicsPipelineDesc& pipelineSettings = desc.graphicsDesc;
		pipelineSettings.primitiveTopo = RHI_PRIMITIVE_TOPO_TRI_LIST;
		pipelineSettings.renderTargetCount = 1;
		pipelineSettings.pDepthState = &depthStateDesc;
		RHI_RenderTargetDesc rtDesc = {};
		RHI_renderTargetGetDesc(RHI_swapChainGetRenderTarget(mSwapChain, 0), rtDesc);
		RHI_RenderTargetDesc depthRtDesc = {};
		RHI_renderTargetGetDesc(mDepthBuffer, depthRtDesc);
		pipelineSettings.pColorFormats = &rtDesc.format;
		pipelineSettings.sampleCount = rtDesc.sampleCount;
		pipelineSettings.sampleQuality = rtDesc.sampleQuality;
		pipelineSettings.depthStencilFormat = depthRtDesc.format;
		pipelineSettings.rootSignature = mRootSignature;
		pipelineSettings.shaderProgram = mShader;
		pipelineSettings.pVertexLayout = &vertexLayout;
		pipelineSettings.pRasterizerState = &rasterizerStateDesc;

		RHI_addPipeline(mRenderer, &desc, &mGraphicsPipeline);
	}

	//add a gui component
	{
		GuiDesc desc = {};
		const float dpiScale = getDpiScale().x;
		desc.mStartPosition = vec2(10.0f, 10.0f) / dpiScale;
		desc.mStartSize = vec2(120.0f, 110.0f) / dpiScale;

		mGuiWindow = mAppUI.AddGuiComponent("Gui Test", &desc);
		mGuiWindow->AddWidget(CheckboxWidget("V-Sync", &mVSyncEnabled));
		mGuiWindow->AddWidget(SliderFloatWidget("Rotation Speed", &mRotationSpeed, 0.0f, 1.0f, 0.1f));
	}


	//matrices
	const float aspect = (float)mFbWidth / (float)mFbHeight;
	mProjMatrix = glm::perspective(45.0f, aspect, 0.1f, 100.00f);
	mViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	return true;
}

void Demo::onSize(const int32_t width, const int32_t height)
{
	//check if we even need to resize
	if (width == mFbWidth && height == mFbHeight)
		return;

	//store new dimensions
	mFbWidth = width;
	mFbHeight = height;

	//wait for the graphics queue to be idle
	RHI_waitQueueIdle(mGraphicsQueue);

	//remove old swapchain and depth buffer
	RHI_removeSwapChain(mRenderer, mSwapChain);
	RHI_removeRenderTarget(mRenderer, mDepthBuffer);
	mAppUI.Unload();

	//create new swapchain and depth buffer
	createSwapchainResources();

	//recalc projection matrix
	const float aspect = (float)mFbWidth / (float)mFbHeight;
	mProjMatrix = glm::perspective(45.0f, aspect, 0.1f, 100.00f);
}

void Demo::onMouseButton(int32_t button, int32_t action)
{
	//the-forge only wants to know about left mouse button for the gui 
	if (button != GLFW_MOUSE_BUTTON_1)
		return;

	bool buttonPressed = false;
	if (action == GLFW_PRESS)
		buttonPressed = true;

	//the-forge has input bindings that are designed for game controllers, it seems to map left mouse button to BUTTON_SOUTH
	mAppUI.OnButton(InputBindings::BUTTON_SOUTH, buttonPressed, &mMousePosition);
}

bool Demo::createSwapchainResources()
{
	RHI_WindowHandle handle;
#ifdef _WIN32
	handle.window = glfwGetWin32Window(mWindow);
#else
	handle.window = glfwGetX11Window(mWindow);
	handle.display = glfwGetX11Display();
#endif

	//create swapchain
	{
		RHI_SwapChainDesc desc = {};
		desc.windowHandle = handle;
		desc.presentQueueCount = 1;
		desc.pPresentQueues = &mGraphicsQueue;
		desc.width = mFbWidth;
		desc.height = mFbHeight;
		desc.imageCount = gImageCount;
		desc.colorFormat = RHI_getRecommendedSwapchainFormat(true);
		desc.enableVsync = true;
		desc.colorClearValue = mLoadActions.clearColorValues[0];
		RHI_addSwapChain(mRenderer, &desc, &mSwapChain);

		if (mSwapChain == NULL)
			return false;
	}

	//create depth buffer
	{
		RHI_RenderTargetDesc desc = {};
		desc.arraySize = 1;
		desc.clearValue = mLoadActions.clearDepth;
		desc.depth = 1;
		desc.format = RHI_IMAGE_FORMAT_D32_SFLOAT;
		desc.height = mFbHeight;
		desc.sampleCount = RHI_SAMPLE_COUNT_1;
		desc.sampleQuality = 0;
		desc.width = mFbWidth;
		desc.flags = RHI_TEXTURE_CREATION_FLAG_ON_TILE;
		RHI_addRenderTarget(mRenderer, &desc, &mDepthBuffer);

		if (mDepthBuffer == NULL)
			return false;
	}

	RenderTarget* rt = (RenderTarget*)RHI_swapChainGetRenderTarget(mSwapChain, 1);
	if (!mAppUI.Load(&rt, 1))
		return false;

	return true;
}

void Demo::onRender()
{
	//delta time
	const float deltaTime = mTimer.GetMSec(true) / 1000.0f;

	//mouse pos
	if (glfwGetWindowAttrib(mWindow, GLFW_FOCUSED))
	{
		double mouseX, mouseY;
		glfwGetCursorPos(mWindow, &mouseX, &mouseY);
		mMousePosition = { (float)mouseX, (float)mouseY };
	}
	else
	{
		mMousePosition = { -1.0f, -1.0f };
	}

	//update UI
	mAppUI.Update(deltaTime);

	//cube rotation - make it spin slowly
	mRotation += deltaTime * mRotationSpeed;
	mModelMatrix = glm::rotate(glm::mat4(1.0f), mRotation * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mModelMatrix = glm::rotate(mModelMatrix, mRotation * glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//recalculate world view projection matrix
	const glm::mat4 worldViewProj = mProjMatrix * mViewMatrix *mModelMatrix;

	//aquire the next swapchain image
	RHI_acquireNextImage(mRenderer, mSwapChain, mImageAcquiredSemaphore, NULL, &mFrameIndex);

	//make it easier on our fingers :)
	RHI_RenderTargetHandle pRenderTarget = RHI_swapChainGetRenderTarget(mSwapChain, mFrameIndex);
	RHI_SemaphoreHandle    pRenderCompleteSemaphore = mRenderCompleteSemaphores[mFrameIndex];
	RHI_FenceHandle        pRenderCompleteFence = mRenderCompleteFences[mFrameIndex];

	// Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
	RHI_FenceStatus fenceStatus;
	RHI_getFenceStatus(mRenderer, pRenderCompleteFence, &fenceStatus);
	if (fenceStatus == RHI_FENCE_STATUS_INCOMPLETE)
		RHI_waitForFences(mRenderer, 1, &pRenderCompleteFence);

	//command buffer for this frame
	RHI_CmdHandle pCmd = mCmds[mFrameIndex];
	RHI_beginCmd(pCmd);

	//transition our render & depth target to a state that we can write to
	RHI_RenderTargetBarrier barriers[] = 
	{
		{ pRenderTarget, RHI_RESOURCE_STATE_RENDER_TARGET },
		{ mDepthBuffer, RHI_RESOURCE_STATE_DEPTH_WRITE },
	};
	RHI_cmdResourceBarrier(pCmd, 0, NULL, 0, NULL, 2, barriers);

	//bind render and depth target and set the viewport and scissor rectangle
	mLoadActions.loadActionsColor[0] = RHI_LOAD_ACTION_CLEAR;
	mLoadActions.loadActionDepth = RHI_LOAD_ACTION_CLEAR;
	RHI_RenderTargetDesc rtDesc = {};
	RHI_renderTargetGetDesc(pRenderTarget, rtDesc);
	RHI_cmdBindRenderTargets(pCmd, 1, &pRenderTarget, mDepthBuffer, &mLoadActions, NULL, NULL, -1, -1);
	RHI_cmdSetViewport(pCmd, 0.0f, 0.0f, (float)rtDesc.width, (float)rtDesc.height, 0.0f, 1.0f);
	RHI_cmdSetScissor(pCmd, 0, 0, rtDesc.width, rtDesc.height);

	//bind descriptor set
	RHI_cmdBindDescriptorSet(pCmd, 0, mDescriptorSet);
	//bind pipeline state object
	RHI_cmdBindPipeline(pCmd, mGraphicsPipeline);
	//bind index buffer
	RHI_cmdBindIndexBuffer(pCmd, mIndexBuffer, RHI_INDEX_TYPE_UINT16, 0);
	//bind vert buffer
	const uint32_t stride = sizeof(Vertex);
	RHI_cmdBindVertexBuffer(pCmd, 1, &mVertexBuffer, &stride, NULL);
	//bind the push constant
	RHI_cmdBindPushConstants(pCmd, mRootSignature, "UniformBlockRootConstant", &worldViewProj);
	//draw our cube
	RHI_cmdDrawIndexed(pCmd, mIndexCount, 0, 0);

	//draw UI - we want the swapchain render target bound without the depth buffer
	mLoadActions.loadActionsColor[0] = RHI_LOAD_ACTION_LOAD;
	mLoadActions.loadActionDepth = RHI_LOAD_ACTION_DONTCARE;
	RHI_cmdBindRenderTargets(pCmd, 1, &pRenderTarget, NULL, &mLoadActions, NULL, NULL, -1, -1);
	mAppUI.Gui(mGuiWindow);
	mAppUI.Draw((Cmd*)pCmd);

	//make sure no render target is bound
	RHI_cmdBindRenderTargets(pCmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
	//transition render target to a present state
	barriers[0] = { pRenderTarget, RHI_RESOURCE_STATE_PRESENT };
	RHI_cmdResourceBarrier(pCmd, 0, NULL, 0, NULL, 1, barriers);

	//end the command buffer
	RHI_endCmd(pCmd);

	//submit the graphics queue
	RHI_QueueSubmitDesc submitDesc = {};
	submitDesc.cmdCount = 1;
	submitDesc.signalSemaphoreCount = 1;
	submitDesc.waitSemaphoreCount = 1;
	submitDesc.pCmds = &pCmd;
	submitDesc.pSignalSemaphores = &pRenderCompleteSemaphore;
	submitDesc.pWaitSemaphores = &mImageAcquiredSemaphore;
	submitDesc.signalFence = pRenderCompleteFence;
	RHI_queueSubmit(mGraphicsQueue, &submitDesc);

	//present the graphics queue
	RHI_QueuePresentDesc presentDesc = {};
	presentDesc.index = mFrameIndex;
	presentDesc.waitSemaphoreCount = 1;
	presentDesc.swapChain = mSwapChain;
	presentDesc.pWaitSemaphores = &pRenderCompleteSemaphore;
	presentDesc.submitDone = true;
	RHI_queuePresent(mGraphicsQueue, &presentDesc);

	//check v-sync
	if (RHI_swapChainGetVSync(mSwapChain) != mVSyncEnabled)
	{
		RHI_waitQueueIdle(mGraphicsQueue);
		RHI_toggleVSync(mRenderer, &mSwapChain);
	}
}
