#pragma once
#ifndef RHI_EXPORT_H_
#define RHI_EXPORT_H_

#if !defined(RHI_API)
#    if defined(_WIN32)
#        if defined(RHI_IMPLEMENTATION)
#            define RHI_API __declspec(dllexport)
#        else
#            define RHI_API __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(RHI_IMPLEMENTATION)
#            define RHI_API __attribute__((visibility("default")))
#        else
#            define RHI_API
#        endif
#    endif  // defined(_WIN32)
#endif  // defined(RHI_API)

#endif // RHI_EXPORT_H_
