#pragma once

#include <memory>
#include <utility>

// =============================================================================
// Platform-specific definitions
// =============================================================================
#ifdef CH_PLATFORM_WINDOWS
#ifdef CH_DYNAMIC_LINKING
#ifdef CH_BUILD_DLL
#define CHERRY_API __declspec(dllexport)
#else
#define CHERRY_API __declspec(dllimport)
#endif // CH_BUILD_DLL
#else
#define CHERRY_API
#endif // CH_DYNAMIC_LINKING
#else
#error CHERRY ONLY SUPPORTS WINDOWS
#endif // CH_PLATFORM_WINDOWS

// =============================================================================
// Debug configuration
// =============================================================================
#ifdef CH_DEBUG
#define CH_ENABLE_ASSERTS
//#define CH_PROFILE          // Enable profiling in debug builds
#endif

// =============================================================================
// Assertion macros
// =============================================================================
#ifdef CH_ENABLE_ASSERTS
#define CH_CLIENT_ASSERT(x, ...) { if(!(x)) { CH_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define CH_CORE_ASSERT(x, ...) { if(!(x)) { CH_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define CH_CLIENT_ASSERT(x, ...)
#define CH_CORE_ASSERT(x, ...)
#endif

// =============================================================================
// Utility macros
// =============================================================================
#define BIT(X) (1 << X)
#define CH_BIND_EVENT_FN(fn) (std::bind(&fn, this, std::placeholders::_1))
/*
// =============================================================================
// Profiling macros - Conditional compilation for zero overhead in release
// =============================================================================
#ifdef CH_PROFILE
    // Resolve which function signature macro to use based on compiler
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define CH_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define CH_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define CH_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define CH_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define CH_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define CH_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define CH_FUNC_SIG __func__
#else
#define CH_FUNC_SIG "CH_FUNC_SIG unknown!"
#endif

// ImGui-based real-time profiling macros
#define PROFILE_SCOPE(name) InstrumentationTimer timer_##__LINE__(name)
#define CH_PROFILE_FUNCTION() CH_PROFILE_SCOPE(CH_FUNC_SIG)
#define CH_PROFILE_FRAME_MARK() static ::Cherry::FrameTimer s_frameTimer##__LINE__; s_frameTimer##__LINE__.NewFrame()
#else
#define CH_PROFILE_SCOPE(name)
#define CH_PROFILE_FUNCTION()
#define CH_PROFILE_FRAME_MARK()
#endif
*/
namespace Cherry {

    // =============================================================================
    // Smart Pointer Utility Class
    // Provides convenient factory functions and utilities for std::unique_ptr
    // and std::shared_ptr to improve code readability and consistency
    // =============================================================================
    class SmartPointer {
    public:
        //-------------------------------------------------------------------------
        // Type aliases for smart pointers
        //-------------------------------------------------------------------------
        template<typename T>
        using Scope = std::unique_ptr<T>;

        template<typename T>
        using Ref = std::shared_ptr<T>;

        //-------------------------------------------------------------------------
        // Factory functions for creating scoped objects (unique_ptr)
        // Perfect forwarding ensures optimal performance for constructor arguments
        //-------------------------------------------------------------------------
        template<typename T, typename... Args>
        static Scope<T> CreateScope(Args&&... args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        //-------------------------------------------------------------------------
        // Factory functions for creating referenced objects (shared_ptr)
        // Perfect forwarding ensures optimal performance for constructor arguments
        //-------------------------------------------------------------------------
        template<typename T, typename... Args>
        static Ref<T> CreateRef(Args&&... args) {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

        //-------------------------------------------------------------------------
        // Utility function to convert unique_ptr to shared_ptr
        // Note: This transfers ownership from unique_ptr to shared_ptr
        //-------------------------------------------------------------------------
        template<typename T>
        static Ref<T> ScopeToRef(Scope<T>&& scopePtr) {
            return Ref<T>(scopePtr.release());
        }

        //-------------------------------------------------------------------------
        // Utility functions to check if a smart pointer is valid (non-null)
        //-------------------------------------------------------------------------
        template<typename T>
        static bool IsValid(const Scope<T>& ptr) {
            return ptr != nullptr;
        }

        template<typename T>
        static bool IsValid(const Ref<T>& ptr) {
            return ptr != nullptr;
        }

        //-------------------------------------------------------------------------
        // Get reference count for shared_ptr
        // Useful for debugging and understanding object lifetime
        //-------------------------------------------------------------------------
        template<typename T>
        static long GetRefCount(const Ref<T>& ptr) {
            return ptr.use_count();
        }

        //-------------------------------------------------------------------------
        // Reset smart pointers - releases current object and optionally assigns new one
        //-------------------------------------------------------------------------
        template<typename T>
        static void Reset(Scope<T>& ptr) {
            ptr.reset();
        }

        template<typename T>
        static void Reset(Ref<T>& ptr) {
            ptr.reset();
        }

        template<typename T>
        static void Reset(Scope<T>& ptr, T* newPtr) {
            ptr.reset(newPtr);
        }

        template<typename T>
        static void Reset(Ref<T>& ptr, T* newPtr) {
            ptr.reset(newPtr);
        }

    private:
        // Prevent instantiation - this is a utility class with static methods only
        SmartPointer() = delete;
        ~SmartPointer() = delete;
        SmartPointer(const SmartPointer&) = delete;
        SmartPointer& operator=(const SmartPointer&) = delete;
    };

    // =============================================================================
    // Convenience macros for shorter syntax (optional but recommended)
    // These provide a more concise way to work with smart pointers
    // =============================================================================
#define SCOPE(type) Cherry::SmartPointer::Scope<type>
#define REF(type) Cherry::SmartPointer::Ref<type>
#define CREATE_SCOPE(type, ...) Cherry::SmartPointer::CreateScope<type>(__VA_ARGS__)
#define CREATE_REF(type, ...) Cherry::SmartPointer::CreateRef<type>(__VA_ARGS__)

} // namespace Cherry