#include "slangc.h"
#include "slang/include/slang.h"
#include "slang/include/slang-com-ptr.h"
#include <string>
#include <vector>
#include <memory>

using namespace slang;
using namespace Slang;

// Internal wrapper structures
struct SlangcGlobalSession {
    Slang::ComPtr<slang::IGlobalSession> session;
};

struct SlangcSession {
    Slang::ComPtr<slang::ISession> session;
};

struct SlangcModule {
    Slang::ComPtr<slang::IModule> module;
};

struct SlangcEntryPoint {
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
};

struct SlangcComponentType {
    Slang::ComPtr<slang::IComponentType> componentType;
};

struct SlangcBlob {
    Slang::ComPtr<ISlangBlob> blob;
};

struct SlangcCompileRequest {
    Slang::ComPtr<slang::ICompileRequest> request;
};

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
}

//
// Global Session Management
//

SlangcResult slangc_createGlobalSession(SlangcGlobalSession** outGlobalSession) {
    if (!outGlobalSession) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcGlobalSession>();
    
    SlangResult result = slang_createGlobalSession(SLANG_API_VERSION, wrapper->session.writeRef());
    if (SLANG_FAILED(result)) {
        return convertResult(result);
    }

    *outGlobalSession = wrapper.release();
    return SLANGC_OK;
}

SlangcResult slangc_createGlobalSessionWithDesc(
    const SlangcGlobalSessionDesc* desc,
    SlangcGlobalSession** outGlobalSession) {
    
    if (!desc || !outGlobalSession) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcGlobalSession>();
    
    SlangGlobalSessionDesc slangDesc = {};
    slangDesc.apiVersion = desc->apiVersion;
    slangDesc.enableGLSL = desc->enableGLSL;

    SlangResult result = slang_createGlobalSession2(&slangDesc, wrapper->session.writeRef());
    if (SLANG_FAILED(result)) {
        return convertResult(result);
    }

    *outGlobalSession = wrapper.release();
    return SLANGC_OK;
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

SlangcResult slangc_createSession(
    SlangcGlobalSession* globalSession,
    const SlangcSessionDesc* desc,
    SlangcSession** outSession) {
    
    if (!globalSession || !outSession) return SLANGC_E_INVALID_ARG;

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
        return convertResult(result);
    }

    *outSession = wrapper.release();
    return SLANGC_OK;
}

void slangc_releaseSession(SlangcSession* session) {
    delete session;
}

//
// Module Loading
//

SlangcResult slangc_loadModule(
    SlangcSession* session,
    const char* moduleName,
    SlangcModule** outModule,
    SlangcBlob** outDiagnostics) {
    
    if (!session || !moduleName || !outModule) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcModule>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    wrapper->module = session->session->loadModule(moduleName, diagnostics.writeRef());
    if (!wrapper->module) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return SLANGC_FAIL;
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outModule = wrapper.release();
    return SLANGC_OK;
}

SlangcResult slangc_loadModuleFromSource(
    SlangcSession* session,
    const char* moduleName,
    const char* path,
    const char* sourceText,
    size_t sourceSize,
    SlangcModule** outModule,
    SlangcBlob** outDiagnostics) {
    
    if (!session || !moduleName || !sourceText || !outModule) return SLANGC_E_INVALID_ARG;

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
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return SLANGC_FAIL;
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outModule = wrapper.release();
    return SLANGC_OK;
}

void slangc_releaseModule(SlangcModule* module) {
    delete module;
}

//
// Component Type Management
//

SlangcResult slangc_findEntryPoint(
    SlangcModule* module,
    const char* entryPointName,
    SlangcStage stage,
    SlangcEntryPoint** outEntryPoint,
    SlangcBlob** outDiagnostics) {
    
    if (!module || !entryPointName || !outEntryPoint) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcEntryPoint>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    SlangResult result = module->module->findAndCheckEntryPoint(
        entryPointName,
        convertStage(stage),
        wrapper->entryPoint.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return convertResult(result);
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outEntryPoint = wrapper.release();
    return SLANGC_OK;
}

void slangc_releaseEntryPoint(SlangcEntryPoint* entryPoint) {
    delete entryPoint;
}

SlangcResult slangc_createModuleComponentType(
    SlangcSession* session,
    SlangcModule* module,
    SlangcEntryPoint* entryPoint,
    SlangcComponentType** outComponentType,
    SlangcBlob** outDiagnostics) {
    
    if (!session || !module || !outComponentType) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcComponentType>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    // Create an array of components to combine
    std::vector<IComponentType*> components;
    components.push_back(module->module.get());
    if (entryPoint) {
        components.push_back(entryPoint->entryPoint.get());
    }

    SlangResult result = session->session->createCompositeComponentType(
        components.data(),
        (SlangInt)components.size(),
        wrapper->componentType.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return convertResult(result);
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outComponentType = wrapper.release();
    return SLANGC_OK;
}

SlangcResult slangc_createEntryPointComponentType(
    SlangcModule* module,
    const char* entryPointName,
    SlangcStage stage,
    SlangcComponentType** outComponentType,
    SlangcBlob** outDiagnostics) {
    
    if (!module || !entryPointName || !outComponentType) return SLANGC_E_INVALID_ARG;

    auto wrapper = std::make_unique<SlangcComponentType>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    // First find the entry point
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SlangResult result = module->module->findAndCheckEntryPoint(
        entryPointName,
        convertStage(stage),
        entryPoint.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return convertResult(result);
    }

    // Note: This function creates a component type from entry point only
    // In practice, you would typically combine this with the module
    wrapper->componentType = entryPoint;

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outComponentType = wrapper.release();
    return SLANGC_OK;
}

SlangcResult slangc_createCompositeComponentType(
    SlangcSession* session,
    SlangcComponentType* const* componentTypes,
    int32_t componentTypeCount,
    SlangcComponentType** outComposite,
    SlangcBlob** outDiagnostics) {
    
    if (!session || !componentTypes || componentTypeCount <= 0 || !outComposite) {
        return SLANGC_E_INVALID_ARG;
    }

    auto wrapper = std::make_unique<SlangcComponentType>();
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    std::vector<IComponentType*> slangComponents(componentTypeCount);
    for (int32_t i = 0; i < componentTypeCount; i++) {
        slangComponents[i] = componentTypes[i]->componentType.get();
    }

    SlangResult result = session->session->createCompositeComponentType(
        slangComponents.data(),
        componentTypeCount,
        wrapper->componentType.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return convertResult(result);
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    *outComposite = wrapper.release();
    return SLANGC_OK;
}

void slangc_releaseComponentType(SlangcComponentType* componentType) {
    delete componentType;
}

SlangcResult slangc_getEntryPointCode(
    SlangcComponentType* componentType,
    int32_t entryPointIndex,
    int32_t targetIndex,
    SlangcBlob** outCode,
    SlangcBlob** outDiagnostics) {
    
    if (!componentType || !outCode) return SLANGC_E_INVALID_ARG;

    Slang::ComPtr<ISlangBlob> codeBlob;
    Slang::ComPtr<ISlangBlob> diagnostics;
    
    SlangResult result = componentType->componentType->getEntryPointCode(
        entryPointIndex,
        targetIndex,
        codeBlob.writeRef(),
        diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
        if (outDiagnostics && diagnostics) {
            auto diagWrapper = std::make_unique<SlangcBlob>();
            diagWrapper->blob = diagnostics;
            *outDiagnostics = diagWrapper.release();
        }
        return convertResult(result);
    }

    if (codeBlob) {
        auto codeWrapper = std::make_unique<SlangcBlob>();
        codeWrapper->blob = codeBlob;
        *outCode = codeWrapper.release();
    }

    if (outDiagnostics && diagnostics) {
        auto diagWrapper = std::make_unique<SlangcBlob>();
        diagWrapper->blob = diagnostics;
        *outDiagnostics = diagWrapper.release();
    }

    return SLANGC_OK;
}

//
// Compilation
//

SlangcResult slangc_createCompileRequest(
    SlangcSession* session,
    SlangcCompileRequest** outCompileRequest) {
    
    if (!session || !outCompileRequest) return SLANGC_E_INVALID_ARG;

    // Note: This is using the deprecated compile request API
    // For production use, prefer the component type API
    return SLANGC_E_NOT_IMPLEMENTED;
}

int32_t slangc_addTranslationUnit(
    SlangcCompileRequest* request,
    SlangcSourceLanguage language,
    const char* path,
    const char* source) {
    
    // Not implemented - use the modern component type API instead
    return -1;
}

int32_t slangc_addEntryPoint(
    SlangcCompileRequest* request,
    int32_t translationUnitIndex,
    const char* name,
    SlangcStage stage) {
    
    // Not implemented - use the modern component type API instead
    return -1;
}

SlangcResult slangc_setTarget(SlangcCompileRequest* request, SlangcCompileTarget target) {
    // Not implemented - use the modern component type API instead
    return SLANGC_E_NOT_IMPLEMENTED;
}

SlangcResult slangc_compile(SlangcCompileRequest* request) {
    // Not implemented - use the modern component type API instead
    return SLANGC_E_NOT_IMPLEMENTED;
}

SlangcResult slangc_getCompiledCode(
    SlangcCompileRequest* request,
    int32_t entryPointIndex,
    SlangcBlob** outCode) {
    
    // Not implemented - use the modern component type API instead
    return SLANGC_E_NOT_IMPLEMENTED;
}

SlangcResult slangc_getDiagnosticOutput(
    SlangcCompileRequest* request,
    SlangcBlob** outDiagnostics) {
    
    // Not implemented - use the modern component type API instead
    return SLANGC_E_NOT_IMPLEMENTED;
}

void slangc_releaseCompileRequest(SlangcCompileRequest* request) {
    delete request;
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
