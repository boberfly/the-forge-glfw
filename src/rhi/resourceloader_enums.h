#pragma once
#ifndef RHI_RESOURCELOADER_ENUMS_H_
#define RHI_RESOURCELOADER_ENUMS_H_

typedef enum RHI_ResourceDirectory
{
    /// The main application's shader binaries folder
    RHI_RD_SHADER_BINARIES = 0,
    /// The main application's shader source directory
    RHI_RD_SHADER_SOURCES,
    /// The main application's texture source directory (TODO processed texture folder)
    RHI_RD_TEXTURES,
    RHI_RD_MESHES,
    RHI_RD_BUILTIN_FONTS,
    RHI_RD_GPU_CONFIG,
    RHI_RD_ANIMATIONS,
    RHI_RD_AUDIO,
    RHI_RD_OTHER_FILES,

    // Libraries can have their own directories.
    // Up to 100 libraries are supported.
    ____rhi_rd_lib_counter_begin = RHI_RD_OTHER_FILES,

    // Add libraries here
    RHI_RD_MIDDLEWARE_0,
    RHI_RD_MIDDLEWARE_1,
    RHI_RD_MIDDLEWARE_2,
    RHI_RD_MIDDLEWARE_3,

    ____rhi_rd_lib_counter_end = ____rhi_rd_lib_counter_begin + 99 * 3,
    RHI_RD_ROOT,
    RHI_RD_COUNT
} RHI_ResourceDirectory;

typedef enum RHI_LoadPriority
{
	// This load priority is only used for updates that
	// have their data stored in GPU memory (e.g. from an
	// updateResource call).
	RHI_LOAD_PRIORITY_UPDATE = 0,
	
	// LoadPriorities High, Normal, and Low are for loads
	// where the data is not already stored in GPU memory.
	RHI_LOAD_PRIORITY_HIGH,
	RHI_LOAD_PRIORITY_NORMAL,
	RHI_LOAD_PRIORITY_LOW,
	
	RHI_LOAD_PRIORITY_COUNT
} RHI_LoadPriority;

typedef enum RHI_GeometryLoadFlags
{
	/// Keep shadow copy of indices and vertices for CPU
	RHI_GLF_SHADOWED = 0x1,
	/// Use structured buffers instead of raw buffers
	RHI_GLF_STRUCTURED_BUFFERS = 0x2,
} RHI_GeometryLoadFlags;

#endif
