// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Runtime/Launch/Resources/Version.h>
#include <string>

/**
 *
 */

namespace HttpGPT
{
    namespace Internal
    {
        template<typename Ty>
        constexpr const bool HasEmptyParam(const Ty& Arg1)
        {
            if constexpr (std::is_base_of<FString, Ty>())
            {
                return Arg1.IsEmpty();
            }
            else if constexpr (std::is_base_of<FName, Ty>())
            {
                return Arg1.IsNone();
            }
            else if constexpr (std::is_base_of<FText, Ty>())
            {
                return Arg1.IsEmptyOrWhitespace();
            }
            else if constexpr (std::is_base_of<std::string, Ty>())
            {
                return Arg1.empty();
            }
            else
            {
#if ENGINE_MAJOR_VERSION >= 5
                return Arg1.IsEmpty();
#else
                return Arg1.Num() == 0;
#endif
            }
        }

        template<typename Ty, typename ...Args>
        constexpr const bool HasEmptyParam(const Ty& Arg1, Args&& ...args)
        {
            return HasEmptyParam(Arg1) || HasEmptyParam(std::forward<Args>(args)...);
        }
    }
}