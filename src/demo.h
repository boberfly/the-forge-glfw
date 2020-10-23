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

#pragma once

#include "rhi/rhi.h"
#include <OS/Interfaces/ITime.h>
#include <Middleware_3/UI/AppUI.h>
#include <glm/glm.hpp>

//image count
const uint32_t gImageCount = 3;

//forward declare
struct GLFWwindow;

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoord;

	Vertex(float x, float y, float z,
		float u, float v)
		: pos(x, y, z), texCoord(u, v) {}
};

class Demo
{
public:
	~Demo();
	bool init(GLFWwindow *pWindow);
	void onSize(const int32_t width, const int32_t height);
	void onMouseButton(int32_t button, int32_t action);
	void onRender();

private:

	bool createSwapchainResources();

	RHI_RendererHandle mRenderer = NULL;
	RHI_QueueHandle mGraphicsQueue = NULL;
	RHI_CmdPoolHandle mCmdPools[gImageCount] = { NULL };
	RHI_CmdHandle mCmds[gImageCount] = { NULL };
	RHI_SwapChainHandle mSwapChain = NULL;
	RHI_RenderTargetHandle mDepthBuffer = NULL;
	RHI_LoadActionsDesc mLoadActions = {};
	RHI_FenceHandle mRenderCompleteFences[gImageCount] = { NULL };
	RHI_SemaphoreHandle mImageAcquiredSemaphore = NULL;
	RHI_SemaphoreHandle mRenderCompleteSemaphores[gImageCount] = { NULL };

	RHI_TextureHandle mTexture = NULL;
	RHI_ShaderHandle mShader = NULL;
	RHI_RootSignatureHandle mRootSignature = NULL;
	RHI_DescriptorSetHandle mDescriptorSet = NULL;
	RHI_PipelineHandle mGraphicsPipeline = NULL;
	RHI_BufferHandle mVertexBuffer = NULL;
	RHI_BufferHandle mIndexBuffer = NULL;
	RHI_SamplerHandle mSampler = NULL;

	Timer mTimer;

	int32_t mFbWidth = 0;
	int32_t mFbHeight = 0;

	uint32_t mIndexCount = 0;
	uint32_t mFrameIndex = 0;

	GLFWwindow *mWindow = NULL;

	//matrices
	glm::mat4 mProjMatrix = glm::mat4(1.0f);
	glm::mat4 mViewMatrix = glm::mat4(1.0f);
	glm::mat4 mModelMatrix = glm::mat4(1.0f);
	float mRotation = 0.0f;
	float mRotationSpeed = 0.5f;

	//UI
	UIApp mAppUI;
	GuiComponent *mGuiWindow = NULL;
	bool mVSyncEnabled = true;
	//we need to use the-forge sony math var for this
	float2 mMousePosition = { 0.0f, 0.0f };
};
