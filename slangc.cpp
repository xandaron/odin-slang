#include "slangc.h"
#include "include/slang.h"
#include "include/slang-com-ptr.h"
#include <string>
#include <vector>
#include <memory>

using namespace slang;

// Global error state for simple error handling
static thread_local std::string g_lastError;
static thread_local bool g_hasError = false;

// Helper to set error state
void setError(const std::string& message) {
    g_lastError = message;
    g_hasError = true;
}

void clearError() {
    g_lastError.clear();
    g_hasError = false;
}

// Internal wrapper structures
struct SlangcGlobalSession {
    Slang::ComPtr<IGlobalSession> session;
};

struct SlangcSession {
    Slang::ComPtr<ISession> session;
};

struct SlangcModule {
    Slang::ComPtr<IModule> module;
};

struct SlangcEntryPoint {
    Slang::ComPtr<IEntryPoint> entryPoint;
};

// Internal composite wrapper for C++ ComPtr management
struct SlangcComposite {
    Slang::ComPtr<IComponentType> composite;
};

struct SlangcBlob {
    Slang::ComPtr<ISlangBlob> blob;
};

// Helper function to get IComponentType from SlangcComponentType
IComponentType* getComponentType(const SlangcComponentType* componentType) {
    if (!componentType) return nullptr;
    
    switch (componentType->kind) {
        case SLANGC_COMPONENT_TYPE_MODULE:
            return componentType->module ? componentType->module->module.get() : nullptr;
        case SLANGC_COMPONENT_TYPE_ENTRY_POINT:
            return componentType->entryPoint ? componentType->entryPoint->entryPoint.get() : nullptr;
        case SLANGC_COMPONENT_TYPE_COMPOSITE:
            return componentType->composite ? 
                static_cast<SlangcComposite*>(componentType->composite)->composite.get() : nullptr;
        default:
            return nullptr;
    }
}

// Helper functions
namespace {
    SlangcResult convertResult(SlangResult result) {
        switch (result) {
            case SLANG_OK:
                return SLANGC_OK;
            case SLANG_FAIL:
                return SLANGC_FAIL;
            case SLANG_E_NOT_IMPLEMENTED:
                return SLANGC_E_NOT_IMPLEMENTED;
            case SLANG_E_INVALID_ARG:
                return SLANGC_E_INVALID_ARG;
            case SLANG_E_OUT_OF_MEMORY:
                return SLANGC_E_OUT_OF_MEMORY;
            case SLANG_E_BUFFER_TOO_SMALL:
                return SLANGC_E_BUFFER_TOO_SMALL;
            default:
                return SLANGC_FAIL;
        }
    }

    SlangSourceLanguage convertSourceLanguage(SlangcSourceLanguage lang) {
        switch (lang) {
            case SLANGC_SOURCE_LANGUAGE_SLANG:
                return SLANG_SOURCE_LANGUAGE_SLANG;
            case SLANGC_SOURCE_LANGUAGE_HLSL:
                return SLANG_SOURCE_LANGUAGE_HLSL;
            case SLANGC_SOURCE_LANGUAGE_GLSL:
                return SLANG_SOURCE_LANGUAGE_GLSL;
            case SLANGC_SOURCE_LANGUAGE_C:
                return SLANG_SOURCE_LANGUAGE_C;
            case SLANGC_SOURCE_LANGUAGE_CPP:
                return SLANG_SOURCE_LANGUAGE_CPP;
            default:
                return SLANG_SOURCE_LANGUAGE_UNKNOWN;
        }
    }

    SlangCompileTarget convertCompileTarget(SlangcCompileTarget target) {
        switch (target) {
            case SLANGC_TARGET_HLSL:
                return SLANG_HLSL;
            case SLANGC_TARGET_GLSL:
                return SLANG_GLSL;
            case SLANGC_TARGET_SPIRV:
                return SLANG_SPIRV;
            case SLANGC_TARGET_SPIRV_ASM:
                return SLANG_SPIRV_ASM;
            case SLANGC_TARGET_DXBC:
                return SLANG_DXBC;
            case SLANGC_TARGET_DXBC_ASM:
                return SLANG_DXBC_ASM;
            case SLANGC_TARGET_DXIL:
                return SLANG_DXIL;
            case SLANGC_TARGET_DXIL_ASM:
                return SLANG_DXIL_ASM;
            case SLANGC_TARGET_C_SOURCE:
                return SLANG_C_SOURCE;
            case SLANGC_TARGET_CPP_SOURCE:
                return SLANG_CPP_SOURCE;
            case SLANGC_TARGET_CUDA_SOURCE:
                return SLANG_CUDA_SOURCE;
            case SLANGC_TARGET_PTX:
                return SLANG_PTX;
            case SLANGC_TARGET_CUBIN:
                return SLANG_CUDA_OBJECT_CODE;
            case SLANGC_TARGET_METAL:
                return SLANG_METAL;
            case SLANGC_TARGET_METAL_LIB:
                return SLANG_METAL_LIB;
            case SLANGC_TARGET_HOST_CALLABLE:
                return SLANG_SHADER_HOST_CALLABLE;
            case SLANGC_TARGET_SHADER_SHARED_LIBRARY:
                return SLANG_SHADER_SHARED_LIBRARY;
            case SLANGC_TARGET_SHADER_HOST_CALLABLE:
                return SLANG_SHADER_HOST_CALLABLE;
            case SLANGC_TARGET_WGSL:
                return SLANG_WGSL;
            default:
                return SLANG_TARGET_UNKNOWN;
        }
    }

    SlangStage convertStage(SlangcStage stage) {
        switch (stage) {
            case SLANGC_STAGE_VERTEX:
                return SLANG_STAGE_VERTEX;
            case SLANGC_STAGE_HULL:
                return SLANG_STAGE_HULL;
            case SLANGC_STAGE_DOMAIN:
                return SLANG_STAGE_DOMAIN;
            case SLANGC_STAGE_GEOMETRY:
                return SLANG_STAGE_GEOMETRY;
            case SLANGC_STAGE_FRAGMENT:
                return SLANG_STAGE_FRAGMENT;
            case SLANGC_STAGE_COMPUTE:
                return SLANG_STAGE_COMPUTE;
            case SLANGC_STAGE_RAY_GENERATION:
                return SLANG_STAGE_RAY_GENERATION;
            case SLANGC_STAGE_INTERSECTION:
                return SLANG_STAGE_INTERSECTION;
            case SLANGC_STAGE_ANY_HIT:
                return SLANG_STAGE_ANY_HIT;
            case SLANGC_STAGE_CLOSEST_HIT:
                return SLANG_STAGE_CLOSEST_HIT;
            case SLANGC_STAGE_MISS:
                return SLANG_STAGE_MISS;
            case SLANGC_STAGE_CALLABLE:
                return SLANG_STAGE_CALLABLE;
            case SLANGC_STAGE_MESH:
                return SLANG_STAGE_MESH;
            case SLANGC_STAGE_AMPLIFICATION:
                return SLANG_STAGE_AMPLIFICATION;
            default:
                return SLANG_STAGE_NONE;
        }
    }
    
    // Helper to create a wrapper for diagnostics blob
    void setDiagnosticsOutput(SlangcBlob** outDiagnostics, Slang::ComPtr<ISlangBlob>& diagnostics) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
    }
}

//
// Global Session Management
//

SlangcGlobalSession* slangc_createGlobalSession(void) {
    clearError();
    auto wrapper = std::make_unique<SlangcGlobalSession>();
    
    SlangResult result = slang_createGlobalSession(SLANG_API_VERSION, wrapper->session.writeRef());
    if (SLANG_FAILED(result)) {
        setError("Failed to create global session");
        return nullptr;
    }

    return wrapper.release();
}

SlangcGlobalSession* slangc_createGlobalSessionWithDesc(
    const SlangcGlobalSessionDesc* desc) {
    
    clearError();
    if (!desc) {
        setError("Invalid session description");
        return nullptr;
    }

    auto wrapper = std::make_unique<SlangcGlobalSession>();
    
    // Fallback to basic global session creation since slang_createGlobalSession2 may not be available
    SlangResult result = slang_createGlobalSession(desc->apiVersion, wrapper->session.writeRef());
    if (SLANG_FAILED(result)) {
        setError("Failed to create global session with description");
        return nullptr;
    }

    return wrapper.release();
}

void slangc_releaseGlobalSession(SlangcGlobalSession* globalSession) {
    delete globalSession;
}

SlangcProfileID slangc_findProfile(SlangcGlobalSession* globalSession, const char* name) {
    if (!globalSession || !name) return 0;
    return globalSession->session->findProfile(name);
}

//
// Session Management
//

SlangcSession* slangc_createSession(
    SlangcGlobalSession* globalSession,
    const SlangcSessionDesc* desc) {
    
    clearError();
    if (!globalSession) {
        setError("Invalid global session");
        return nullptr;
    }

    auto wrapper = std::make_unique<SlangcSession>();
    
    SessionDesc sessionDesc = {};
    std::vector<TargetDesc> targets;
    
    if (desc && desc->targetCount > 0) {
        targets.resize(desc->targetCount);
        for (int32_t i = 0; i < desc->targetCount; i++) {
            targets[i].format = convertCompileTarget(desc->targets[i]);
        }
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = desc->targetCount;
    } else {
        // Default to SPIRV target if none specified
        targets.resize(1);
        targets[0].format = SLANG_SPIRV;
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = 1;
    }

    SlangResult result = globalSession->session->createSession(sessionDesc, wrapper->session.writeRef());
    if (SLANG_FAILED(result)) {
        setError("Failed to create session");
        return nullptr;
    }

    return wrapper.release();
}

void slangc_releaseSession(SlangcSession* session) {
    delete session;
}

//
// Module Loading
//

SlangcModule* slangc_loadModule(
    SlangcSession* session,
    const char* moduleName,
    SlangcBlob** outDiagnostics) {
    
    clearError();
    if (!session || !moduleName) {
        setError("Invalid session or module name");
        return nullptr;
    }

    auto wrapper = std::make_unique<SlangcModule>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    wrapper->module = session->session->loadModule(moduleName, diagnostics.writeRef());
    if (!wrapper->module) {
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        setError("Failed to load module");
        return nullptr;
    }

    setDiagnosticsOutput(outDiagnostics, diagnostics);
    return wrapper.release();
}

SlangcModule* slangc_loadModuleFromSource(
    SlangcSession* session,
    const char* moduleName,
    const char* path,
    const char* sourceText,
    size_t sourceSize,
    SlangcBlob** outDiagnostics) {
    
    if (!session || !moduleName || !sourceText) return nullptr;

    auto wrapper = std::make_unique<SlangcModule>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    // Create a blob from the source text
    std::string source(sourceText, sourceSize == 0 ? strlen(sourceText) : sourceSize);
    Slang::ComPtr<ISlangBlob> sourceBlob;
    // Note: We would need to create a blob implementation here
    // For now, we'll use the string directly
    
    wrapper->module = session->session->loadModuleFromSource(
        moduleName, path, nullptr, diagnostics.writeRef());
    
    if (!wrapper->module) {
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        return nullptr;
    }

    setDiagnosticsOutput(outDiagnostics, diagnostics);
    return wrapper.release();
}

void slangc_releaseModule(SlangcModule* module) {
    delete module;
}

//
// Component Type Management
//

SlangcComponentTypeKind slangc_getComponentTypeKind(SlangcComponentType* componentType) {
    if (!componentType) {
        return SLANGC_COMPONENT_TYPE_MODULE; // Default fallback
    }
    return componentType->kind;
}

SlangcModule* slangc_getComponentTypeModule(SlangcComponentType* componentType) {
    if (!componentType || componentType->kind != SLANGC_COMPONENT_TYPE_MODULE) {
        return nullptr;
    }
    return componentType->module;
}

SlangcEntryPoint* slangc_getComponentTypeEntryPoint(SlangcComponentType* componentType) {
    if (!componentType || componentType->kind != SLANGC_COMPONENT_TYPE_ENTRY_POINT) {
        return nullptr;
    }
    return componentType->entryPoint;
}

SlangcEntryPoint* slangc_findEntryPoint(
    SlangcModule* module,
    const char* entryPointName,
    SlangcStage stage,
    SlangcBlob** outDiagnostics) {
    
    clearError();
    if (!module || !entryPointName) {
        setError("Invalid module or entry point name");
        return nullptr;
    }

    auto wrapper = std::make_unique<SlangcEntryPoint>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    SlangResult result = module->module->findAndCheckEntryPoint(
        entryPointName,
        convertStage(stage),
        wrapper->entryPoint.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        setError("Failed to find entry point");
        return nullptr;
    }

    setDiagnosticsOutput(outDiagnostics, diagnostics);
    return wrapper.release();
}

void slangc_releaseEntryPoint(SlangcEntryPoint* entryPoint) {
    delete entryPoint;
}

SlangcComponentType* slangc_createModuleComponentType(SlangcModule* module) {
    clearError();
    if (!module) {
        setError("Invalid module");
        return nullptr;
    }
    
    auto wrapper = new SlangcComponentType();
    wrapper->kind = SLANGC_COMPONENT_TYPE_MODULE;
    wrapper->module = module;
    return wrapper;
}

SlangcComponentType* slangc_createEntryPointComponentType(SlangcEntryPoint* entryPoint) {
    clearError();
    if (!entryPoint) {
        setError("Invalid entry point");
        return nullptr;
    }
    
    auto wrapper = new SlangcComponentType();
    wrapper->kind = SLANGC_COMPONENT_TYPE_ENTRY_POINT;
    wrapper->entryPoint = entryPoint;
    return wrapper;
}

SlangcComponentType* slangc_createCompositeComponentType(
    SlangcSession* session,
    const SlangcComponentType* componentTypes,
    int32_t componentTypeCount,
    SlangcBlob** outDiagnostics) {
    
    clearError();
    if (!session || !componentTypes || componentTypeCount <= 0) {
        setError("Invalid parameters for composite component type");
        return nullptr;
    }

    auto compositeWrapper = new SlangcComposite();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    std::vector<IComponentType*> slangComponents(componentTypeCount);
    for (int32_t i = 0; i < componentTypeCount; i++) {
        slangComponents[i] = getComponentType(&componentTypes[i]);
        if (!slangComponents[i]) {
            delete compositeWrapper;
            setError("Invalid component type in array");
            return nullptr;
        }
    }

    SlangResult result = session->session->createCompositeComponentType(
        slangComponents.data(),
        componentTypeCount,
        compositeWrapper->composite.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        delete compositeWrapper;
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        setError("Failed to create composite component type");
        return nullptr;
    }

    auto wrapper = new SlangcComponentType();
    wrapper->kind = SLANGC_COMPONENT_TYPE_COMPOSITE;
    wrapper->composite = compositeWrapper;
    
    setDiagnosticsOutput(outDiagnostics, diagnostics);
    return wrapper;
}

SlangcComponentType* slangc_linkComponentType(
    SlangcComponentType* componentType,
    SlangcBlob** outDiagnostics) {
    
    clearError();
    if (!componentType) {
        setError("Invalid component type");
        return nullptr;
    }

    IComponentType* slangComponentType = getComponentType(componentType);
    if (!slangComponentType) {
        setError("Invalid component type");
        return nullptr;
    }

    auto linkedCompositeWrapper = new SlangcComposite();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    SlangResult result = slangComponentType->link(
        linkedCompositeWrapper->composite.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        delete linkedCompositeWrapper;
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        setError("Failed to link component type");
        return nullptr;
    }

    auto wrapper = new SlangcComponentType();
    wrapper->kind = SLANGC_COMPONENT_TYPE_COMPOSITE;
    wrapper->composite = linkedCompositeWrapper;
    
    setDiagnosticsOutput(outDiagnostics, diagnostics);
    return wrapper;
}

void slangc_releaseComponentType(SlangcComponentType* componentType) {
    if (!componentType) return;
    
    // Clean up composite wrapper if it exists
    if (componentType->kind == SLANGC_COMPONENT_TYPE_COMPOSITE && componentType->composite) {
        delete static_cast<SlangcComposite*>(componentType->composite);
    }
    
    delete componentType;
}

SlangcBlob* slangc_getEntryPointCode(
    SlangcComponentType* componentType,
    int32_t entryPointIndex,
    int32_t targetIndex,
    SlangcBlob** outDiagnostics) {
    
    clearError();
    if (!componentType) {
        setError("Invalid component type");
        return nullptr;
    }

    Slang::ComPtr<ISlangBlob> codeBlob;
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    IComponentType* slangComponentType = getComponentType(componentType);
    if (!slangComponentType) {
        setError("Invalid component type");
        return nullptr;
    }
    
    SlangResult result = slangComponentType->getEntryPointCode(
        entryPointIndex,
        targetIndex,
        codeBlob.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        setDiagnosticsOutput(outDiagnostics, diagnostics);
        setError("Failed to get entry point code");
        return nullptr;
    }

    setDiagnosticsOutput(outDiagnostics, diagnostics);
    
    if (codeBlob) {
        auto codeWrapper = std::make_unique<SlangcBlob>();
        codeWrapper->blob = codeBlob;
        return codeWrapper.release();
    }

    return nullptr;
}

//
// Blob Management
//

const void* slangc_getBlobData(SlangcBlob* blob) {
    if (!blob) return nullptr;
    return blob->blob->getBufferPointer();
}

size_t slangc_getBlobSize(SlangcBlob* blob) {
    if (!blob) return 0;
    return blob->blob->getBufferSize();
}

void slangc_releaseBlob(SlangcBlob* blob) {
    delete blob;
}

//
// Utilities
//

const char* slangc_getVersionString(void) {
    // Return a static version string
    return "Slang C Wrapper 1.0";
}

void slangc_shutdown(void) {
    slang_shutdown();
}

const char* slangc_getLastErrorMessage(void) {
    return slang_getLastInternalErrorMessage();
}

//
// Error Handling
//

bool slangc_hasError(void) {
    return g_hasError;
}

const char* slangc_getLastError(void) {
    return g_hasError ? g_lastError.c_str() : nullptr;
}

void slangc_clearError(void) {
    clearError();
}
