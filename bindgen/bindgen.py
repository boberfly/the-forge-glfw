import os, shutil, subprocess
from gen_json import gen_json
from gen_c import gen_c

hidden_funcs = """
// Hidden functions that we want if we can't rely on resourceloader.h
void addBuffer(Renderer *pRenderer, const BufferDesc *desc, Buffer **pp_buffer);
void removeBuffer(Renderer *pRenderer, Buffer *p_buffer);
void addTexture(Renderer *pRenderer, const TextureDesc *pDesc, Texture **pp_texture);
void removeTexture(Renderer *pRenderer, Texture *p_texture);
void mapBuffer(Renderer *pRenderer, Buffer *pBuffer, ReadRange *pRange);
void unmapBuffer(Renderer *pRenderer, Buffer *pBuffer);
void cmdUpdateBuffer(Cmd *pCmd, Buffer *pBuffer, uint64_t dstOffset, Buffer *pSrcBuffer, uint64_t srcOffset, uint64_t size);
void cmdUpdateSubresource(Cmd *pCmd, Texture *pTexture, Buffer *pSrcBuffer, SubresourceDataDesc *pSubresourceDesc);

"""

typdef_structs = """
typedef struct Renderer              Renderer;
typedef struct Queue                 Queue;
typedef struct Pipeline              Pipeline;
typedef struct Buffer                Buffer;
typedef struct Texture               Texture;
typedef struct RenderTarget          RenderTarget;
typedef struct ShaderReflectionInfo  ShaderReflectionInfo;
typedef struct Shader                Shader;
typedef struct DescriptorSet         DescriptorSet;
typedef struct DescriptorIndexMap    DescriptorIndexMap;
typedef struct ShaderDescriptors     ShaderDescriptors;

"""

def clean_header(infile):
    hdr = []
    with open(infile, "r", newline="\n") as header:
        for line in header:
            if not "#include" in line:
                if not "#import" in line:
                    hdr.append(line)

    with open(infile, "w", newline="\n") as header:
        header.write("#define FORGE_CALLCONV\n")
        header.write("#define API_INTERFACE\n") # So the functions are visible
        header.write("#define DEFINE_ALIGNED\n") # Removes structs we don't want to expose & keep as opaque handles
        header.write("#define DIRECT3D12\n#define VULKAN\n#define METAL\n") # Enable all backends so we get their custom enums/struct entries to bind
        header.write("typedef enum TinyImageFormat TinyImageFormat;\n") # So that the following types are scraped by clang
        header.write("typedef struct Windowhandle WindowHandle;\n")
        header.write("typedef struct Sampler Sampler;\n")
        header.write("typedef struct CmdPool CmdPool;\n")
        header.write("typedef struct RootSignature RootSignature;\n")
        header.write("typedef struct Cmd Cmd;\n")
        header.write("typedef struct Fence Fence;\n")
        header.write("typedef struct SwapChain SwapChain;\n")
        header.write("typedef struct Semaphore Semaphore;\n")
        header.write("typedef struct Raytracing Raytracing;\n")
        header.write("typedef struct ShaderReflection ShaderReflection;\n")
        header.write("typedef struct PipelineReflection PipelineReflection;\n")
        header.write("typedef struct ShaderResource ShaderResource;\n")
        header.write("typedef struct ShaderVariable ShaderVariable;\n")
        header.write("typedef struct VertexInput VertexInput;\n")
        header.write("typedef struct ArgumentDescriptor ArgumentDescriptor;\n")
        header.write("typedef struct DescriptorSet DescriptorSet;\n")
        header.write("typedef struct Pipeline Pipeline;\n")
        header.write("typedef struct MTLDataType MTLDataType;\n")
        header.write("typedef struct MTLArgumentAccess MTLArgumentAccess;\n")
        header.write("typedef struct MTLTextureType MTLTextureType;\n")
        header.write("typedef struct SubresourceDataDesc SubresourceDataDesc;\n")
        header.write("#include <stddef.h>\n#include <stdbool.h>\n#include <stdint.h>\n") # Base types
        for line in hdr:
            header.write(line)
            
def apply_hidden_funcs(infile):
    hdr = ""
    with open(infile, "r", newline="\n") as header:
        hdr = header.read()

    with open(infile, "w", newline="\n") as header:
        header.write(hdr+hidden_funcs)

def fix_resourceloader_includes(infile):
    hdr = ""
    with open(infile, "r", newline="\n") as header:
        hdr = header.read()

    with open(infile, "w", newline="\n") as header:
        header.write("#include \"./IRenderer.h\"\n")
        # what is uint?
        header.write("#define uint uint32_t\n")
        header.write(hdr)
        
def apply_renderer_include(infile):
    hdr = ""
    with open(infile, "r", newline="\n") as header:
        hdr = header.read()

    with open(infile, "w", newline="\n") as header:
        header.write("#include \"./IRenderer.h\"\n")
        header.write(hdr)

def apply_tinyimage_include(infile):
    hdr = ""
    with open(infile, "r", newline="\n") as header:
        hdr = header.read()

    with open(infile, "w", newline="\n") as header:
        header.write("#include \"./imageformat.h\"\n")
        header.write(hdr)

def fix_shaderstage_define(infile):
    hdr = []
    with open(infile, "r", newline="\n") as header:
        for line in header:
            # Make sure ShaderStage always uses non-metal define
            if '} SampleCount;' in line:
                hdr.append(line)
                hdr.append('#undef METAL')
            elif 'MAKE_ENUM_FLAG(uint32_t, ShaderStage)' in line:
                hdr.append(line)
                hdr.append('#define METAL')
            else:
                hdr.append(line)

    with open(infile, "w", newline="\n") as header:
        for line in hdr:
            header.write(line)

def main():
    # Location of TheForge
    tf_path = "../external/the-forge/Common_3/"
    
    # Copy and simplify important headers
    shutil.copy(tf_path+"Renderer/IRay.h", "./IRay.h")
    shutil.copy(tf_path+"Renderer/IRenderer.h", "./IRenderer.h")
    shutil.copy(tf_path+"Renderer/IResourceLoader.h", "./IResourceLoader.h")
    shutil.copy(tf_path+"Renderer/IShaderReflection.h", "./IShaderReflection.h")
    shutil.copy(tf_path+"ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h", "./imageformat.h")

    # Remove includes so clang json doesn't generate huge amounts of redundant data
    clean_header("./IRay.h")
    clean_header("./IRenderer.h")
    clean_header("./IResourceLoader.h")
    clean_header("./IShaderReflection.h")
    clean_header("./imageformat.h")

    # TinyFmageFormat include
    apply_tinyimage_include("./IRenderer.h")
    
    # Bring back hidden symbols to bind
    apply_hidden_funcs("./IRenderer.h")
    # Fix ShaderStage define so the metal one isn't used
    fix_shaderstage_define("./IRenderer.h")
    # ResourceLoader needs typedefs
    fix_resourceloader_includes("./IResourceLoader.h")

    apply_renderer_include("./IRay.h")
    apply_renderer_include("./IShaderReflection.h")

    with open("./IRay.ast.json", "w") as outfile:
        subprocess.run(["clang", "-Xclang", "-ast-dump=json", "IRay.h"], stdout=outfile)
    with open("./IRenderer.ast.json", "w") as outfile:
        subprocess.run(["clang", "-Xclang", "-ast-dump=json", "IRenderer.h"], stdout=outfile)
    with open("./IResourceLoader.ast.json", "w") as outfile:
        subprocess.run(["clang", "-Xclang", "-ast-dump=json", "IResourceLoader.h"], stdout=outfile)
    with open("./IShaderReflection.ast.json", "w") as outfile:
        subprocess.run(["clang", "-Xclang", "-ast-dump=json", "IShaderReflection.h"], stdout=outfile)
    with open("./imageformat.ast.json", "w") as outfile:
        subprocess.run(["clang", "-Xclang", "-ast-dump=json", "imageformat.h"], stdout=outfile)
    
    gen_json('IRay')
    gen_json('IRenderer')
    gen_json('IResourceLoader')
    gen_json('IShaderReflection')
    gen_json('imageformat')
    
    if not os.path.isdir('../src/rhi/'):
        os.mkdir('../src/rhi')
    if not os.path.isdir('../src/rhi/private'):
        os.mkdir('../src/rhi/private')
    gen_c("./", "../src/rhi/", "RHI")

    os.remove("./IRay.ast.json")
    os.remove("./IRenderer.ast.json")
    os.remove("./IResourceLoader.ast.json")
    os.remove("./IShaderReflection.ast.json")
    os.remove("./imageformat.ast.json")
    os.remove("./IRay.json")
    os.remove("./IRenderer.json")
    os.remove("./IResourceLoader.json")
    os.remove("./IShaderReflection.json")
    os.remove("./imageformat.json")
    os.remove("./IRay.h")
    os.remove("./IRenderer.h")
    os.remove("./IResourceLoader.h")
    os.remove("./IShaderReflection.h")
    os.remove("./imageformat.h")

if __name__ == '__main__':
    main()

