import json, re, os
from string import Template

export_template = """$header
#pragma once
#ifndef ${prefix}_EXPORT_H_
#define ${prefix}_EXPORT_H_

#if !defined(${prefix}_API)
#    if defined(_WIN32)
#        if defined(${prefix}_IMPLEMENTATION)
#            define ${prefix}_API __declspec(dllexport)
#        else
#            define ${prefix}_API __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(${prefix}_IMPLEMENTATION)
#            define ${prefix}_API __attribute__((visibility(\"default\")))
#        else
#            define ${prefix}_API
#        endif
#    endif  // defined(_WIN32)
#endif  // defined(${prefix}_API)

#endif // ${prefix}_EXPORT_H_

"""

enums_template = """$header

#pragma once
#ifndef ${prefix}_${module}_H_
#define ${prefix}_${module}_H_

$enums

#endif // ${prefix}_${module}_H_

"""

structs_template = """$header

#pragma once
#ifndef ${prefix}_${module}_H_
#define ${prefix}_${module}_H_

$includes

$typedef_decls

$structs

#endif // ${prefix}_${module}_H_

"""

funcs_template = """$header

#pragma once
#ifndef ${prefix}_${module}_H_
#define ${prefix}_${module}_H_

$includes

#if defined(__cplusplus)
extern "C" {
#endif
$init_funcs
#if !defined(${prefix}_SKIP_DECLARATIONS)

$funcs

#endif // !defined(${prefix}_SKIP_DECLARATIONS)

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // ${prefix}_${module}_H_

"""

impl_template = """$header
#include "../rhi.h"
#include "../resourceloader.h"
#include "../ray.h"

#if !defined(${prefix}_SKIP_DECLARATIONS)

$includes

struct SubresourceDataDesc
{
	uint64_t                           mSrcOffset;
	uint32_t                           mMipLevel;
	uint32_t                           mArrayLayer;
#if defined(DIRECT3D11) || defined(METAL) || defined(VULKAN)
	uint32_t                           mRowPitch;
	uint32_t                           mSlicePitch;
#endif
};

static void API_CHECK();

extern void addBuffer(Renderer *pRenderer, const BufferDesc *desc, Buffer **pp_buffer);
extern void removeBuffer(Renderer *pRenderer, Buffer *p_buffer);
extern void addTexture(Renderer *pRenderer, const TextureDesc *pDesc, Texture **pp_texture);
extern void removeTexture(Renderer *pRenderer, Texture *p_texture);
extern void mapBuffer(Renderer *pRenderer, Buffer *pBuffer, ReadRange *pRange);
extern void unmapBuffer(Renderer *pRenderer, Buffer *pBuffer);
extern void cmdUpdateBuffer(Cmd *pCmd, Buffer *pBuffer, uint64_t dstOffset, Buffer *pSrcBuffer, uint64_t srcOffset, uint64_t size);
extern void cmdUpdateSubresource(Cmd *pCmd, Texture *pTexture, Buffer *pSrcBuffer, const SubresourceDataDesc *pSubresourceDesc);

${prefix}_Result ${prefix}_init(${prefix}_RendererApi renderer)
{
    API_CHECK(); // windows static_assert(offsetof) is broken so do it at runtime there
    return ${prefix}_RESULT_SUCCESS;
}

void ${prefix}_exit(void)
{
    return;
}

#ifdef ${prefix}_GLFW
void ${prefix}_setWindowHandleGLFW(const GLFWwindow * window, ${prefix}_WindowHandle windowHandle)
{
    if(!window)
        return;
#ifdef _WIN32
    windowHandle.window = (void *)glfwGetWin32Window(window);
#else
    windowHandle.window = (void *)(uintptr_t)glfwGetX11Window(window);
    windowHandle.display = glfwGetX11Display();
#endif
}
#endif // ${prefix}_GLFW

#ifdef ${prefix}_SDL
void ${prefix}_setWindowHandleSDL(const SDL_Window * window, ${prefix}_WindowHandle windowHandle)
{
    if(!window)
        return;

    SDL_SysWMinfo sysInfo;

    SDL_VERSION(&sysInfo.version);
    SDL_GetWindowWMInfo(window, &sysInfo);
#ifdef _WIN32
    windowHandle.window = (void *)sysInfo.info.win.window;
#elif __ANDROID__
    windowHandle.window = (void *)sysInfo.info.android.window;
    windowHandle.activity = SDL_AndroidGetActivity();
#else
    windowHandle.window = (void *)(uintptr_t)sysInfo.info.x11.window;
    windowHandle.display = sysInfo.info.x11.display;
#endif
}
#endif // ${prefix}_SDL

static ShaderStage __shaderStageFlagsToShaderStage(uint32_t flags) {
    uint32_t stage = SHADER_STAGE_NONE;
#if defined(METAL)
    if (flags & ${prefix}_SHADER_STAGE_VERT) {
        stage |= SHADER_STAGE_VERT;
    }
    if (flags & ${prefix}_SHADER_STAGE_FRAG) {
        stage |= SHADER_STAGE_FRAG;
    }
    if (flags & ${prefix}_SHADER_STAGE_COMP) {
        stage |= SHADER_STAGE_COMP;
    }
#else
    if(flags & ${prefix}_SHADER_STAGE_VERT) stage |= SHADER_STAGE_VERT;
    if(flags & ${prefix}_SHADER_STAGE_FRAG) stage |= SHADER_STAGE_FRAG;
    if(flags & ${prefix}_SHADER_STAGE_COMP) stage |= SHADER_STAGE_COMP;
    if(flags & ${prefix}_SHADER_STAGE_TESC) stage |= SHADER_STAGE_TESC;
    if(flags & ${prefix}_SHADER_STAGE_TESE) stage |= SHADER_STAGE_TESE;
    if(flags & ${prefix}_SHADER_STAGE_GEOM) stage |= SHADER_STAGE_GEOM;
    if(flags & ${prefix}_SHADER_STAGE_RAYTRACING) stage |= SHADER_STAGE_RAYTRACING;
#endif
    return (ShaderStage) stage;
}

#ifdef METAL
static void __shaderStageToShaderStage(RHI_ShaderStageDesc const *src, ShaderStageDesc *dst) {
	dst->pName = src->pName;
	dst->pCode = src->pCode;
	dst->pEntryPoint = src->pEntryPoint;
}
#endif

static void __binaryShaderStageToBinaryShaderStage(${prefix}_BinaryShaderStageDesc const *src, BinaryShaderStageDesc *dst) {
    dst->pByteCode = src->pByteCode;
    dst->mByteCodeSize = src->mByteCodeSize;
    dst->pEntryPoint = src->pEntryPoint;
#ifdef METAL
    dst->pSource = src->source;
#endif
}

$func_impls

#define API_CHK(x) static_assert(x, "Mismatched size!")

static void API_CHECK() {
$api_checks
}
#undef API_CHK

#else // !defined(${prefix}_SKIP_DECLARATIONS)

#ifdef _MSC_VER
#  define snprintf _snprintf
#  define popen _popen
#  define pclose _pclose
#  define _CRT_SECURE_NO_WARNINGS
#endif

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

#define DLL_FUNCS() \\
$dll_funcs

DLL_FUNCS()

#undef DLL_FUNC

#define DLL_FUNC(name) DLL_LIBRARY_FIND(##name);

const char *getRendererDLL(${prefix}_RendererApi renderer)
{
    switch(renderer)
    {
#ifdef _WIN32
        case ${prefix}_API_D3D12:
            return "${prefix}_d3d12.dll";
        case ${prefix}_API_VULKAN:
            return "${prefix}_vulkan.dll";
        case ${prefix}_API_D3D11:
            return "${prefix}_d3d11.dll";
#elif defined(__APPLE__)
        case ${prefix}_API_METAL:
            return "${prefix}_metal.dylib";
        case ${prefix}_API_VULKAN:
            return "${prefix}_vulkan.dylib";
#else
        case ${prefix}_API_VULKAN:
            return "${prefix}_vulkan.so";
#endif
        default:
            return NULL;
    }
}

/* Implementation function. */
${prefix}_Result ${prefix}_init(${prefix}_RendererApi renderer)
{
    static int initialized = 0;
    static ${prefix}_Result result = ${prefix}_RESULT_SUCCESS;

    if(initialized)
    {
        return result;
    }

    initialized = 1;

    /* Load library. */
    lib = dynamic_library_open(getRendererDLL(renderer));

    if (lib == NULL)
    {
        result = ${prefix}_RESULT_ERROR;
        return result;
    }

    DLL_FUNCS();

    return result;
}

void ${prefix}_exit(void)
{
    if(lib != NULL)
    {
        /*  Ignore errors. */
        dynamic_library_close(lib);
        lib = NULL;
    }
}

#undef DLL_FUNC

#endif // !defined(${prefix}_SKIP_DECLARATIONS)

"""

handles = ["Renderer", 
    "Fence", 
    "Semaphore", 
    "Queue", 
    "CmdPool", 
    "Cmd", 
    "RenderTarget", 
    "Sampler", 
    "Shader", 
    "RootSignature", 
    "Pipeline", 
    "Raytracing", 
    "RaytracingHitGroup", 
    "DescriptorInfo", 
    "DescriptorSet", 
    "Buffer", 
    "Texture", 
    "AccelerationStructure", 
    "QueryPool", 
    "CommandSignature", 
    "SwapChain",
    "VirtualTexture",
    "PipelineCache",
    "RaytracingShaderTable"]

ignored_structs = ["VirtualTexture", # Virtual texture structs expose too many backend details, skip for now and use custom get funcs to get data out of them
                   "VirtualTexturePage"] # We manage the size diff between metal and other backends in RHI_addShaderBinary

def extract_array(_type):
    tsplit = _type.split(" ")
    if "[" in tsplit[-1] and "]" in tsplit[-1]:
        return tsplit[-1].replace("*", "")
    else:
        return None

def prefix_type(_type, prefix):

    base_types = [
        "uint32_t",
        "uint8_t",
        "bool",
        "float",
        "char",
        "int",
        "void",
        "unsigned int",
        "uint64_t",
        "uint16_t",
        "size_t",
        "int32_t",
        "double"
    ]

    if _type == "TinyImageFormat":
        return f"{prefix}_ImageFormat"
    elif _type == "TinyImageFormat *":
        return f"{prefix}_ImageFormat *"
    elif 'anonymous' in _type:
        return ""
    elif 'MTL' in _type:
        return _type

    t = _type.replace('struct ', '')
    tsplit = t.split(" ")
    pos = 0
    ptr = 0
    array = False
    if len(tsplit) == 1:
        t = tsplit[0]
    elif len(tsplit) == 2:
        if tsplit[0] == "const":
            t = tsplit[1]
            pos = 1
        elif tsplit[-1] == "*":
            t = tsplit[0]
            ptr = 1
        elif tsplit[-1] == "**":
            t = tsplit[0]
            ptr = 2
        elif tsplit[-1] == "***":
            t = tsplit[0]
            ptr = 3
        elif "[" in tsplit[-1] and "]" in tsplit[-1]:
            t = tsplit[0]
            if "*" in tsplit[-1]:
                ptr = 1
                tsplit[-1] = "*"
            else:
                tsplit.pop()
            array = True
    elif len(tsplit) == 3:
        t = tsplit[1]
        pos = 1
        if tsplit[-1] == "*":
            ptr = 1
        elif tsplit[-1] == "**":
            ptr = 2

    if t in base_types:
        if array and ptr > 0:
            t += " "+ptr*"*"
        return _type if not array else t
    elif t == 'Hair':
        return t
    elif t == 'ShadowData':
        return t
      
    if t in handles:
        t += "Handle"
        
        if ptr > 0:
            if ptr < 2:
                tsplit.pop()
            else:
                tsplit[-1] = (ptr-1)*"*"

    t = f"{prefix}_{t}"

    tsplit[pos] = t
    return " ".join(tsplit)

def gen_export(prefix="RHI"):
    s = Template(export_template)
    return s.substitute(header=gen_header(), prefix=prefix)

def gen_header(gen="bindgen.py"):
    header = 75*"/"+"\n"
    header += "//\n"
    header += f"//  Autogenerated by {gen}\n"
    header += "//\n"
    header += 75*"/"+"\n"
    return header


def gen_enums(inp, module, prefix):

    enums = ""
    
    if module == 'ENUMS':
        enums += f"""
#include <stdint.h>

static const int {prefix}_RED = 0x1;
static const int {prefix}_GREEN = 0x2;
static const int {prefix}_BLUE = 0x4;
static const int {prefix}_ALPHA = 0x8;
static const int {prefix}_ALL = ({prefix}_RED | {prefix}_GREEN | {prefix}_BLUE | {prefix}_ALPHA);
static const int {prefix}_NONE = 0;

static const int {prefix}_BS_NONE = -1;
static const int {prefix}_DS_NONE = -1;
static const int {prefix}_RS_NONE = -1;

typedef enum RHI_Result
{{
    RHI_RESULT_SUCCESS = 0,
    RHI_RESULT_ERROR = -1,
}} RHI_Result;

"""

    for decl in inp['decls']:
        if decl['kind'] == 'consts':
            enums += "enum\n{\n"
            enumline = ""
            for item in decl['items']:
                if 'name' in item:
                    enumline += f"    {prefix}_{item['name']}"
                if 'value' in item:
                    enumline += f" = {item['value']},\n"
                else:
                    enumline += ",\n"
            enumline = enumline[:-2]
            enums += enumline+"\n"

            if 'name' in decl:
                enums += f"}} {prefix}_{decl['name']};\n"
            else:
                enums += "};\n"
            enums += "\n"
        
    for decl in inp['decls']:
        if decl['kind'] == 'enum':
            enums += f"typedef enum {prefix}_{decl['name']}\n{{\n"
            enumline = ""
            for item in decl['items']:
                enumline += f"    {prefix}_{item['name']}"
                if 'value' in item:
                    # Not very elegant
                    value = item['value'].replace('DESCRIPTOR', prefix+'_DESCRIPTOR').replace('SHADER_STAGE', prefix+'_SHADER_STAGE')
                    enumline += f" = {value},\n"
                else:
                    enumline += ",\n"
            enumline = enumline[:-2]
            enums += enumline+"\n"

            enums += f"}} {prefix}_{decl['name']};\n\n"

    s = Template(enums_template)
    return s.substitute(header=gen_header(), prefix=prefix, module=module, enums=enums)

def get_ifdef_endif(structname, membername, ifdef=True):
    s = ""
    backend = ""

    if structname == 'RendererDesc':
        if membername == 'ppInstanceLayers':
            backend = "defined(VULKAN)"
        elif membername == 'ppInstanceExtensions':
            backend = "defined(VULKAN)"
        elif membername == 'ppDeviceExtensions':
            backend = "defined(VULKAN)"
        elif membername == 'mInstanceLayerCount':
            backend = "defined(VULKAN)"
        elif membername == 'mInstanceExtensionCount':
            backend = "defined(VULKAN)"
        elif membername == 'mDeviceExtensionCount':
            backend = "defined(VULKAN)"
        elif membername == 'mRequestAllAvailableQueues':
            backend = "defined(VULKAN)"
        elif membername == 'mDxFeatureLevel':
            backend = "defined(DIRECT3D12) || defined(DIRECT3D11)"
    elif structname == 'RaytracingDispatchDesc':
        if membername == 'pTopLevelAccelerationStructure':
            backend = "defined(METAL)"
        elif membername == 'pSets':
            backend = "defined(METAL)"
        elif membername == 'pIndexes':
            backend = "defined(METAL)"
    elif structname == 'GPUSettings':
        if membername == 'mArgumentBufferMaxTextures':
            backend = "defined(METAL)"
        elif membername == 'mHeaps':
            backend = "defined(METAL)"
        elif membername == 'mPlacementHeaps':
            backend = "defined(METAL)"
    elif structname == 'ShaderResource':
        if membername == 'alignment':
            backend = "defined(METAL)"
        elif membername == 'mtlTextureType':
            backend = "defined(METAL)"
        elif membername == 'mtlArgumentBufferType':
            backend = "defined(METAL)"
        elif membername == 'mIsArgumentBufferField':
            backend = "defined(METAL)"
        elif membername == 'mtlArgumentDescriptors':
            backend = "defined(METAL)"

    if backend:
        if ifdef:
            s += f"#if {backend}\n"
        else:
            s += f"#endif // {backend}\n"

    return s

def gen_structs(inp, module, prefix):

    includes = ""

    if module == "STRUCTS":
        includes = """
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include \"enums.h\"
#include \"imageformat.h\"
        """
    elif module == "RESOURCELOADER_STRUCTS":
        includes = """
#include \"enums.h\"
#include \"resourceloader_enums.h\"
#include \"structs.h\"
        """
    elif module == "RAY_STRUCTS":
        includes = """
#include \"enums.h\"
#include \"ray_enums.h\"
#include \"structs.h\"
        """
    elif module == "SHADERREFLECTION_STRUCTS":
        includes = """
#include \"enums.h\"
#include \"shaderreflection_enums.h\"
#include \"structs.h\"
        """

    def recurse_struct(structline, indent, decl):
        for field in decl['fields']:
            if 'fields' in field:
                # inner struct or union
                name = ""
                if 'name' in field:
                    name = field['name'] + ' '
                structline += indent*"    "+f"{field['kind']} {name}{{\n"
                indent += 1
                structline = recurse_struct(structline, indent, field)
                indent -= 1
                structline += indent*"    "+"};\n"
            else:
                ftype = prefix_type(field['type'], prefix)
                array = extract_array(field['type'])

                # This isn't a very elegant way to detect named anonymous structs!
                if 'anonymous' in field['type']:
                    indent -= 1
                    structline = structline[0:-2]
                elif field['type'] == 'ArgumentDescriptor':
                    structline += "#ifdef METAL\n"
                elif('name' in field) and ('name' in decl):
                    structline += get_ifdef_endif(decl['name'], field['name'], True)

                if 'name' in field:
                    if array:
                        structline += indent*"    "+f"{ftype} {field['name']}{array}"
                    else:
                        structline += indent*"    "+f"{ftype} {field['name']}"
                    if 'bitfieldvalue' in field:
                        structline += f" : {field['bitfieldvalue']};\n"
                    elif 'defaultvalue' in field:
                        structline += f" = {field['defaultvalue']};\n"
                    else:
                        structline += ";\n"

                if field['type'] == 'ArgumentDescriptor':
                    structline += "#endif // METAL\n"
                elif('name' in field) and ('name' in decl):
                    structline += get_ifdef_endif(decl['name'], field['name'], False)

        return structline

    indent = 0
    
    typedef_decls = ""

    if module == 'STRUCTS':
        for handle in handles:
            typedef_decls += f"typedef struct {prefix}_{handle} *{prefix}_{handle}Handle;\n"
        typedef_decls += "\n"
        
    structs = ""
    
    if module == 'STRUCTS':
        structs += "typedef void(*RHI_LogFn)(RHI_LogType, const char*, const char*);\n"
        structs += """typedef struct RHI_WindowHandle {
    void * display;  // X11
    void * window;   // native window
    void * activity; // android
} RHI_WindowHandle;
"""
        structs += f"""
struct {prefix}_SubresourceDataDesc
{{
    uint64_t mSrcOffset;
    uint32_t mMipLevel;
    uint32_t mArrayLayer;
// #if defined(DIRECT3D11) || defined(METAL) || defined(VULKAN)
    uint32_t mRowPitch;
    uint32_t mSlicePitch;
}};

"""

    if module == 'RESOURCELOADER_STRUCTS':
        structs += f"""
typedef uint64_t {prefix}_SyncToken;

"""

    for decl in inp['decls']:
        if decl['kind'] == 'struct':
            if decl['name'] in handles:
                continue
            if decl['name'] in ignored_structs:
                continue

            metal_ifdef = False
            if decl['name'] == "ArgumentDescriptor":
                # ifdef METAL for those denoise functions
                metal_ifdef = True
            if metal_ifdef:
                structs += "#ifdef METAL\n"

            structs += f"typedef struct {prefix}_{decl['name']} {{\n"
            indent += 1
            if 'union' in decl:
                structs += indent*"    "+"union {\n"
                indent += 1

            structline = ""
            structline = recurse_struct(structline, indent, decl)
            structs += structline

            if 'union' in decl:
                indent -= 1
                structs += indent*"    "+"};\n"
            indent -= 1
            structs += indent*"    "+f"}} {prefix}_{decl['name']};\n"

            if metal_ifdef:
                structs += "#endif // METAL\n"
            structs += "\n"

    s = Template(structs_template)
    return s.substitute(header=gen_header(), includes=includes, prefix=prefix, module=module, typedef_decls=typedef_decls, structs=structs )


def fix_duplicate_funcs(name, param):
    if name == 'addResource':
        if 'Buffer' in param['name']:
            return 'addBufferResource'
        elif 'Texture' in param['name']:
            return 'addTextureResource'
        elif 'Geom' in param['name']:
            return 'addGeomResource'
    elif name == 'beginUpdateResource':
        if 'Buffer' in param['name']:
            return 'beginUpdateBufferResource'
        elif 'Texture' in param['name']:
            return 'beginUpdateTextureResource'
    elif name == 'endUpdateResource':
        if 'Buffer' in param['name']:
            return 'endUpdateBufferResource'
        elif 'Texture' in param['name']:
            return 'endUpdateTextureResource'
    elif name == 'removeResource':
        if 'Buffer' in param['name']:
            return 'removeBufferResource'
        elif 'Texture' in param['name']:
            return 'removeTextureResource'
        elif 'Geom' in param['name']:
            return 'removeGeomResource'
    return name


def gen_funcs(inp, module, prefix):

    includes = ""

    if module == "RHI":
        includes += """
#include \"export.h\"
#include \"enums.h\"
#include \"imageformat.h\"
#include \"structs.h\"
#include \"resourceloader.h\"
#include \"shaderreflection.h\"
        """
    elif module == "RESOURCELOADER":
        includes += """
#include \"export.h\"
#include \"enums.h\"
#include \"resourceloader_enums.h\"
#include \"structs.h\"
#include \"resourceloader_structs.h\"
        """
    elif module == "RAY":
        includes += """
#include \"export.h\"
#include \"enums.h\"
#include \"structs.h\"
#include \"ray_enums.h\"
#include \"ray_structs.h\"
"""
    elif module == "SHADERREFLECTION":
        includes += """
#include \"export.h\"
#include \"shaderreflection_enums.h\"
#include \"shaderreflection_structs.h\"
"""

    init_funcs = ""

    if module == 'RHI':
        init_funcs += "RHI_API RHI_Result RHI_init(RHI_RendererApi renderer);\n"
        init_funcs += "RHI_API void RHI_exit(void);\n\n"
    else:
        init_funcs += "\n"
        
    funcs = ""

    for decl in inp['decls']:
        if decl['kind'] == 'func':
            # No impl for this func
            if module == 'RAY' and decl['name'] == 'removeAccelerationStructureScratch':
                continue
            split = decl['type'].split(' ')
            returntype = split[0]
            if returntype == 'enum' or returntype == 'struct':
                returntype = split[1]
            returntype = prefix_type(returntype, prefix)
            name = decl['name']

            metal_ifdef = False
            if 'SSVGFD' in name:
                # ifdef METAL for those denoise functions
                metal_ifdef = True
            if metal_ifdef:
                funcs += "#ifdef METAL\n"

            if len(decl['params']) != 0:
                name = fix_duplicate_funcs(decl['name'], decl['params'][0])
            if module == 'RESOURCELOADER' and decl['name'] == 'addPipelineCache':
                name = 'loadPipelineCache'
            if module == 'RESOURCELOADER' and decl['name'] == 'addShader':
                name = 'loadShader'
            funcline = f"RHI_API {returntype} {prefix}_{name}("
            for param in decl['params']:
                _type = prefix_type(param['type'], prefix)
                funcline += f"{_type} {param['name']}, "
            if len(decl['params']) != 0:
                funcline = funcline[:-2]
            funcs += f"{funcline});\n"

            if metal_ifdef:
                funcs += "#endif // METAL\n"

    extra_funcs = f"""

// accessors TheForge C API has opaque handles, this is largely okay as its mostly a push API
// however a few things do need passing back (mostly for rendertarget and swapchain)
// these calls implment the required accessors.
{prefix}_API {prefix}_RenderTargetHandle {prefix}_swapChainGetRenderTarget({prefix}_SwapChainHandle swapChain, int index);
{prefix}_API bool {prefix}_swapChainGetVSync({prefix}_SwapChainHandle handle);
{prefix}_API {prefix}_TextureHandle {prefix}_renderTargetGetTexture({prefix}_RenderTargetHandle renderTarget);
{prefix}_API void {prefix}_renderTargetGetDesc({prefix}_RenderTargetHandle renderTarget, {prefix}_RenderTargetDesc &desc);
{prefix}_API uint32_t {prefix}_textureGetWidth({prefix}_TextureHandle handle);
{prefix}_API uint32_t {prefix}_textureGetHeight({prefix}_TextureHandle handle);

{prefix}_API {prefix}_RendererApi {prefix}_getRendererApi({prefix}_RendererHandle handle);
{prefix}_API bool {prefix}_canShaderReadFrom({prefix}_RendererHandle handle, {prefix}_ImageFormat format);
{prefix}_API bool {prefix}_canShaderWriteTo({prefix}_RendererHandle handle, {prefix}_ImageFormat format);
{prefix}_API bool {prefix}_canRenderTargetWriteTo({prefix}_RendererHandle handle, {prefix}_ImageFormat format);

// do not free return owned by the shader!
{prefix}_API {prefix}_PipelineReflection const* RHI_shaderGetPipelineReflection(RHI_ShaderHandle shader);

"""
    if module == 'RHI':
        funcs += extra_funcs

    s = Template(funcs_template)
    return s.substitute(header=gen_header(), includes=includes, prefix=prefix, module=module, init_funcs=init_funcs, funcs=funcs )


def gen_impl(ray_inp, renderer_inp, resourceloader_inp, shaderreflection_inp, module, prefix):

    def gen_impl_funcs(inp, module, prefix):

        indent = 0
        func_impls = ""

        for decl in inp['decls']:
            if decl['kind'] == 'func':
                # No impl for this func
                if module == 'RAY' and decl['name'] == 'removeAccelerationStructureScratch':
                    continue
                #returntype = decl['type'].split(' ')[0]
                split = decl['type'].split(' ')
                returntype = split[0]
                if returntype == 'enum' or returntype == 'struct':
                    returntype = split[1]
                returntype = prefix_type(returntype, prefix)
                name = decl['name']

                metal_ifdef = False
                if 'SSVGFD' in name:
                    # ifdef METAL for those denoise functions
                    metal_ifdef = True
                if metal_ifdef:
                    func_impls += "#ifdef METAL\n"

                if len(decl['params']) != 0:
                    name = fix_duplicate_funcs(decl['name'], decl['params'][0])

                if module == 'RESOURCELOADER' and decl['name'] == 'addPipelineCache':
                    name = 'loadPipelineCache'
                if module == 'RESOURCELOADER' and decl['name'] == 'addShader':
                    name = 'loadShader'

                funcline = f"RHI_API {returntype} {prefix}_{name}("
                for param in decl['params']:
                    funcline += f"{prefix_type(param['type'], prefix)} {param['name']}, "

                if len(decl['params']) != 0:
                    funcline = funcline[:-2]
                func_impls += f"{funcline}) {{\n"

                if len(decl['params']) != 0:
                    if decl['params'][0]['type'] == "Renderer *":
                        func_impls += f"    auto renderer = (Renderer *){decl['params'][0]['name']};\n"
                        if returntype != "void":
                            func_impls += f"    if(!renderer){{return ({returntype})0;}}\n"
                        else:
                            func_impls += f"    if(!renderer){{return;}}\n"

                castline = "    "
                if(name == 'addSwapChain'):
                    castline += """
    WindowHandle windowHandle{};
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    windowHandle.display = (Display*)p_desc->mWindowHandle.display;
    windowHandle.window = (Window)p_desc->mWindowHandle.window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_connection_t*        connection;
	xcb_window_t             window;
	xcb_screen_t*            screen;
	xcb_intern_atom_reply_t* atom_wm_delete_window;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    windowHandle.window = (ANativeWindow*)p_desc->mWindowHandle.window;
    windowHandle.activity = (ANativeActivity*)p_desc->mWindowHandle.activity;
#else
    windowHandle.window = p_desc->mWindowHandle.window;
#endif
    SwapChainDesc desc{};
    desc.mWindowHandle = windowHandle;
    desc.ppPresentQueues = (Queue **)p_desc->ppPresentQueues;
    desc.mPresentQueueCount = p_desc->mPresentQueueCount;
    desc.mImageCount = p_desc->mImageCount;
    desc.mWidth = p_desc->mWidth;
    desc.mHeight = p_desc->mHeight;
    desc.mColorFormat = (TinyImageFormat)p_desc->mColorFormat;
    ClearValue cv;
    memcpy(&desc.mColorClearValue, &p_desc->mColorClearValue, sizeof(ClearValue));
    desc.mEnableVsync = p_desc->mEnableVsync;
    desc.mUseFlipSwapEffect = p_desc->mUseFlipSwapEffect;
    addSwapChain((Renderer *)pRenderer, &desc, (SwapChain **)p_swap_chain);
}

"""
                    func_impls += castline
                elif(name == 'addShaderBinary'):
                    castline += """
    BinaryShaderDesc desc{};
    desc.mStages = __shaderStageFlagsToShaderStage(p_desc->mStages);
    if (desc.mStages & SHADER_STAGE_VERT) {
        __binaryShaderStageToBinaryShaderStage(&p_desc->mVert, &desc.mVert);
    }
    if (desc.mStages & SHADER_STAGE_FRAG) {
        __binaryShaderStageToBinaryShaderStage(&p_desc->mFrag, &desc.mFrag);
    }
#ifndef METAL
    if(desc.mStages & SHADER_STAGE_GEOM)
        __binaryShaderStageToBinaryShaderStage(&p_desc->mGeom, &desc.mGeom);
    if(desc.mStages & SHADER_STAGE_HULL)
        __binaryShaderStageToBinaryShaderStage(&p_desc->mHull, &desc.mHull);
    if(desc.mStages & SHADER_STAGE_DOMN)
        __binaryShaderStageToBinaryShaderStage(&p_desc->mDomain, &desc.mDomain);
#endif
    if (desc.mStages & SHADER_STAGE_COMP) {
        __binaryShaderStageToBinaryShaderStage(&p_desc->mComp, &desc.mComp);
    }
    addShaderBinary((Renderer *)pRenderer, &desc, (Shader **)p_shader_program);
}

"""
                    func_impls += castline
                elif(name == 'addShader'):
                    castline += """
#ifdef METAL
	ShaderDesc desc{};
	desc.mStages = __ShaderStageFlagsToShaderStage(pDesc->stages);
	if (desc.mStages & SHADER_STAGE_VERT) {
		__ShaderStageToShaderStage(&pDesc->vert, &desc.mVert);
	}
	if (desc.mStages & SHADER_STAGE_FRAG) {
		__ShaderStageToShaderStage(&pDesc->frag, &desc.mFrag);
	}
	if (desc.mStages & SHADER_STAGE_COMP) {
		__ShaderStageToShaderStage(&pDesc->comp, &desc.mComp);
	}

	addShader(renderer, &desc, (Shader **) pShader);
#else
	//LOGERROR("AddShader is only supported on Metal backends, Use AddShaderBinary");
#endif
}

"""
                    func_impls += castline
                elif(name == 'cmdUpdateSubresource'):
                    castline += """
    SubresourceDataDesc desc{};
    desc.mSrcOffset = pSubresourceDesc->mSrcOffset;
    desc.mMipLevel = pSubresourceDesc->mMipLevel;
    desc.mArrayLayer = pSubresourceDesc->mArrayLayer;
#if defined(DIRECT3D11) || defined(METAL) || defined(VULKAN)
    desc.mRowPitch = pSubresourceDesc->mRowPitch;
    desc.mSlicePitch = pSubresourceDesc->mSlicePitch;
#endif
    cmdUpdateSubresource((Cmd *)pCmd, (Texture *)pTexture, (Buffer *)pSrcBuffer, &desc);
}
"""
                    func_impls += castline
                else:
                    if returntype != "void":
                        castline += f"return ({returntype})"
                    castline += f"{decl['name']}("
                    for param in decl['params']:
                        castline += f"({param['type']}){param['name']}, "
                    if len(decl['params']) != 0:
                        castline = castline[:-2]
                    func_impls += f"{castline});\n"
                    func_impls += "}\n"
                    if metal_ifdef:
                        func_impls += "#endif // METAL\n"
                    else:
                        func_impls += "\n"

        return func_impls

    def gen_api_checks(inp, prefix):

        api_checks = ""

        for decl in inp['decls']:
            if decl['kind'] == 'struct':
                if decl['name'] in handles:
                    continue
                if decl['name'] in ignored_structs:
                    continue
                # Different size on Metal
                if decl['name'] == 'BinaryShaderDesc':
                    continue
                if decl['name'] == 'BinaryShaderStageDesc':
                    continue
                # Different sized WindowHandle
                if decl['name'] == 'SwapChainDesc':
                    continue
                # TODO: revisit
                if decl['name'] == 'Geometry':
                    continue
                if decl['name'] == 'ArgumentDescriptor':
                    continue
                api_checks += f"    API_CHK(sizeof({prefix}_{decl['name']}) == sizeof({decl['name']}));\n"
                if 'fields' in decl:
                    for field in decl['fields']:
                        if 'name' in field:
                            # The check doesn't work with bitfield values
                            if 'bitfieldvalue' in field:
                                continue
                            if('name' in field) and ('name' in decl):
                                api_checks += get_ifdef_endif(decl['name'], field['name'], True)

                            api_checks += f"    API_CHK(offsetof({prefix}_{decl['name']}, {field['name']}) == offsetof({decl['name']}, {field['name']}));\n"
                            if('name' in field) and ('name' in decl):
                                api_checks += get_ifdef_endif(decl['name'], field['name'], False)

        return api_checks

    includes = """
#include <string.h>
#include <OS/Interfaces/IOperatingSystem.h>
#include <ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h>
#include <Renderer/IRenderer.h>
#include <Renderer/IResourceLoader.h>
#include <Renderer/IRay.h>
    """

    func_impls = ""
    func_impls += gen_impl_funcs(renderer_inp, prefix, prefix)
    func_impls += "// Resource loader functions\n"
    func_impls += gen_impl_funcs(resourceloader_inp, "RESOURCELOADER", prefix)
    func_impls += "// Ray functions\n"
    func_impls += gen_impl_funcs(ray_inp, "RAY", prefix)
    func_impls += "// Shader reflection functions\n"
    func_impls += gen_impl_funcs(shaderreflection_inp, "SHADERREFLECTION", prefix)

    extra_func_impls = f"""

{prefix}_API {prefix}_RenderTargetHandle {prefix}_swapChainGetRenderTarget({prefix}_SwapChainHandle swapChain, int index)
{{
    return ({prefix}_RenderTargetHandle) ((SwapChain *) swapChain)->ppRenderTargets[index];
}}

{prefix}_API bool {prefix}_swapChainGetVSync({prefix}_SwapChainHandle handle)
{{
    return ((SwapChain*)handle)->mEnableVsync;
}}

{prefix}_API {prefix}_TextureHandle {prefix}_renderTargetGetTexture({prefix}_RenderTargetHandle renderTarget)
{{
    return ({prefix}_TextureHandle) ((RenderTarget *) renderTarget)->pTexture;
}}

{prefix}_API void {prefix}_renderTargetGetDesc({prefix}_RenderTargetHandle renderTarget, {prefix}_RenderTargetDesc &desc)
{{
    RenderTarget *rt = (RenderTarget*)renderTarget;
    desc.mWidth = rt->mWidth;
    desc.mHeight = rt->mHeight;
    desc.mDepth = rt->mDepth;
    desc.mArraySize = rt->mArraySize;
    desc.mMipLevels = rt->mMipLevels;
    desc.mSampleCount = (RHI_SampleCount)rt->mSampleCount;
    desc.mFormat = (RHI_ImageFormat)rt->mFormat;
    ClearValue cv;
    memcpy(&desc.mClearValue, &rt->mClearValue, sizeof(ClearValue));
    desc.mSampleQuality = rt->mSampleQuality;
    desc.mDescriptors = ({prefix}_DescriptorType)rt->mDescriptors;
}}

{prefix}_API uint32_t {prefix}_textureGetWidth({prefix}_TextureHandle handle)
{{
    return ((Texture*)handle)->mWidth;
}}

{prefix}_API uint32_t {prefix}_textureGetHeight({prefix}_TextureHandle handle)
{{
    return ((Texture*)handle)->mHeight;
}}

{prefix}_API uint32_t {prefix}_textureGetDepth({prefix}_TextureHandle handle)
{{
    return ((Texture*)handle)->mDepth;
}}

{prefix}_API uint32_t {prefix}_textureGetMipLevels({prefix}_TextureHandle handle)
{{
    return ((Texture*)handle)->mMipLevels;
}}

{prefix}_API {prefix}_RendererApi {prefix}_getRendererApi({prefix}_RendererHandle handle)
{{
    auto renderer = (Renderer *) handle;
    if (!renderer)
    {{
        return {prefix}_RENDERER_API_VULKAN;
    }}

    return ({prefix}_RendererApi) renderer->mApi;
}}

{prefix}_API bool {prefix}_canShaderReadFrom({prefix}_RendererHandle handle, {prefix}_ImageFormat format)
{{
    auto renderer = (Renderer *) handle;
    if (!renderer)
    {{
        return false;
    }}
    if( (int)format > (int)TinyImageFormat_Count )
    {{
        return false;
    }}
    return renderer->pCapBits->canShaderReadFrom[(TinyImageFormat)format];
}}
{prefix}_API bool {prefix}_canShaderWriteTo({prefix}_RendererHandle handle, {prefix}_ImageFormat format)
{{
    auto renderer = (Renderer *) handle;
    if (!renderer)
    {{
        return false;
    }}
    if( (int)format > (int)TinyImageFormat_Count )
    {{
        return false;
    }}
    return renderer->pCapBits->canShaderWriteTo[(TinyImageFormat)format];
}}

{prefix}_API bool {prefix}_canRenderTargetWriteTo({prefix}_RendererHandle handle, {prefix}_ImageFormat format)
{{
    auto renderer = (Renderer *) handle;
    if (!renderer)
    {{
        return false;
    }}
    if( (int)format > (int)TinyImageFormat_Count )
    {{
        return false;
    }}
    return renderer->pCapBits->canRenderTargetWriteTo[(TinyImageFormat)format];
}}

    """

    func_impls += extra_func_impls
    
    api_checks = ""
    
    api_checks += gen_api_checks(renderer_inp, prefix)
    api_checks += gen_api_checks(resourceloader_inp, prefix)
    api_checks += gen_api_checks(ray_inp, prefix)
    api_checks += gen_api_checks(shaderreflection_inp, prefix)

    dll_funcs = ""

    for decl in renderer_inp['decls']:
        if decl['kind'] == 'func':
            dll_funcs += f"    DLL_FUNC({prefix}_{decl['name']}); \\\n"
            
    for decl in resourceloader_inp['decls']:
        if decl['kind'] == 'func':
            name = decl['name']
            if len(decl['params']) != 0:
                name = fix_duplicate_funcs(decl['name'], decl['params'][0])
            dll_funcs += f"    DLL_FUNC({prefix}_{name}); \\\n"
            
    for decl in ray_inp['decls']:
        if decl['kind'] == 'func':
            dll_funcs += f"    DLL_FUNC({prefix}_{decl['name']}); \\\n"
            
    for decl in shaderreflection_inp['decls']:
        if decl['kind'] == 'func':
            dll_funcs += f"    DLL_FUNC({prefix}_{decl['name']}); \\\n"

    s = Template(impl_template)
    return s.substitute(header=gen_header(), includes=includes, prefix=prefix, module=module, func_impls=func_impls, api_checks=api_checks, dll_funcs=dll_funcs)

  
def gen_imageformat(inp, prefix):

    enums = f"typedef enum {prefix}_ImageFormat\n{{\n"
    for decl in inp['decls']:
        if decl['kind'] == 'enum':
            if decl['name'] == 'TinyImageFormat':
                enumline = ""
                for item in decl['items']:
                    enumline += f"    {prefix}_{item['name'].replace('TinyImageFormat', 'IMAGEFORMAT')}"
                    if 'value' in item:
                        enumline += f" = {item['value']},\n"
                    else:
                        enumline += ",\n"
                enumline = enumline[:-2]
                enums += enumline
                
                enums += f"\n}} {prefix}_ImageFormat;\n\n"

    s = Template(enums_template)
    return s.substitute(header=gen_header(), prefix=prefix, module="IMAGEFORMAT", enums=enums )
    
def combine_resourceloader_structs(inp, inp_types):
    decl_types = []
    for decl in inp_types['decls']:
        if decl['kind'] == 'struct':
            decl_types.append(decl)

    inp['decls'] = decl_types + inp['decls']
    
    return inp
    
def gen_platform(prefix):

    includes = f"""
#ifdef {prefix}_GLFW
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif // {prefix}_GLFW

#ifdef {prefix}_SDL
#include <SDL.h>
#endif // {prefix}_SDL
"""

    funcs = f"""
#ifdef {prefix}_GLFW
{prefix}_API void {prefix}_setWindowHandleGLFW(const GLFWwindow * window, RHI_WindowHandle windowHandle);
#endif // {prefix}_GLFW

#ifdef {prefix}_SDL
{prefix}_API void {prefix}_setWindowHandleSDL(const SDL_Window * window, RHI_WindowHandle windowHandle);
#endif // {prefix}_SDL
"""

    s = Template(funcs_template)
    return s.substitute(header=gen_header(), includes=includes, prefix=prefix, module="PLATFORM", init_funcs="", funcs=funcs )
    

def gen_c(input_path="./", output_path="./rhi", prefix="RHI"):
    try:
        print(f">>> {input_path} => {output_path}")
        ray_inp = ""
        renderer_inp = ""
        resourceloader_inp = ""
        shaderreflection_inp = ""
        imageformat_inp = ""
        #resourceloadertypes_inp = ""
        with open(input_path+"IRay.json", 'r') as f_inp:
            ray_inp = json.load(f_inp)
        with open(input_path+"IRenderer.json", 'r') as f_inp:
            renderer_inp = json.load(f_inp)
        with open(input_path+"IResourceLoader.json", 'r') as f_inp:
            resourceloader_inp = json.load(f_inp)
        with open(input_path+"IShaderReflection.json", 'r') as f_inp:
            shaderreflection_inp = json.load(f_inp)
        with open(input_path+"imageformat.json", 'r') as f_inp:
            imageformat_inp = json.load(f_inp)
            
        gen = gen_platform(prefix)
        with open(output_path+"platform.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_export(prefix)
        with open(output_path+"export.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_imageformat(imageformat_inp, prefix)
        with open(output_path+"imageformat.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_enums(renderer_inp, 'ENUMS', prefix)
        with open(output_path+"enums.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_enums(ray_inp, 'RAY_ENUMS', prefix)
        with open(output_path+"ray_enums.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_enums(resourceloader_inp, 'RESOURCELOADER_ENUMS', prefix)
        with open(output_path+"resourceloader_enums.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)
            
        gen = gen_enums(shaderreflection_inp, 'SHADERREFLECTION_ENUMS', prefix)
        with open(output_path+"shaderreflection_enums.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_structs(renderer_inp, 'STRUCTS', prefix)
        with open(output_path+"structs.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_structs(ray_inp, 'RAY_STRUCTS', prefix)
        with open(output_path+"ray_structs.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_structs(resourceloader_inp, 'RESOURCELOADER_STRUCTS', prefix)
        with open(output_path+"resourceloader_structs.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)
            
        gen = gen_structs(shaderreflection_inp, 'SHADERREFLECTION_STRUCTS', prefix)
        with open(output_path+"shaderreflection_structs.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_funcs(renderer_inp, prefix, prefix)
        with open(output_path+prefix.lower()+".h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_funcs(ray_inp, 'RAY', prefix)
        with open(output_path+"ray.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

        gen = gen_funcs(resourceloader_inp, 'RESOURCELOADER', prefix)
        with open(output_path+"resourceloader.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)
            
        gen = gen_funcs(shaderreflection_inp, 'SHADERREFLECTION', prefix)
        with open(output_path+"shaderreflection.h", 'w', newline='\n') as f_outp:
            f_outp.write(gen)
        
        gen = gen_impl(ray_inp, renderer_inp, resourceloader_inp, shaderreflection_inp, 'RHI', prefix)
        with open(output_path+"private/"+prefix.lower()+".cpp", 'w', newline='\n') as f_outp:
            f_outp.write(gen)

    except EnvironmentError as err:
        print(f"{err}")


def main():
    if not os.path.isdir('rhi/'):
        os.mkdir('rhi')
    if not os.path.isdir('rhi/private'):
        os.mkdir('rhi/private')
    gen_c('./', 'rhi/')

if __name__ == '__main__':
    main()

