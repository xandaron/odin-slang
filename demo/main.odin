package test

import slang "../odin-slang"
import "core:os/os2"
import "core:fmt"

main :: proc() {
	searchPaths := []cstring{"./shaders"}

	desc := slang.Global_Session_Desc {
		searchPaths     = raw_data(searchPaths),
		searchPathCount = i32(len(searchPaths)),
	}
	globalSession := slang.createGlobalSessionWithDesc(&desc)
	if globalSession == nil {
		panic("Failed to create global session\n")
	}

	compileTargets := []slang.Compile_Target{.GLSL}
	sessionDesc := slang.Session_Desc {
		targets                = raw_data(compileTargets),
		targetCount            = i32(len(compileTargets)),
		searchPaths            = raw_data(searchPaths),
		searchPathCount        = i32(len(searchPaths)),
		preprocessorMacros     = nil,
		preprocessorMacroCount = 0,
	}
	session := slang.createSession(globalSession, &sessionDesc)
	if session == nil {
		panic("Failed to create session\n")
	}

	blob: ^slang.Blob
	module := slang.loadModule(session, "./shaders/VertexDemo.slang", &blob)
	if module == nil {
		fmt.println(slang.getLastError())

		fmt.println(cstring(slang.getBlobData(blob)))

		panic("Failed to load module")
	}

	vertEntryPoint := slang.findEntryPoint(module, "vertexMain", .VERTEX, nil)
	if vertEntryPoint == nil {
		panic("Failed to find entry point\n")
	}

	fragEntryPoint := slang.findEntryPoint(module, "fragmentMain", .FRAGMENT, nil)
	if fragEntryPoint == nil {
		panic("Failed to find entry point\n")
	}

	components := []slang.Component_Type {
		{kind = .MODULE, module = module},
		{kind = .ENTRY_POINT, entryPoint = vertEntryPoint},
		{kind = .ENTRY_POINT, entryPoint = fragEntryPoint},
	}
	program := slang.createCompositeComponentType(
		session,
		raw_data(components),
		i32(len(components)),
		nil,
	)
	if program == nil {
		panic("Failed to create program\n")
	}

	linkedProgram := slang.linkComponentType(program, nil)
	if linkedProgram == nil {
		panic("Failed to link program\n")
	}

	codeBlob := slang.getEntryPointCode(linkedProgram, 0, 0, nil)
	if codeBlob == nil {
		panic("Failed to get entry point code\n")
	}

	codeString := (cstring)(slang.getBlobData(codeBlob))

	file, _ := os2.open("vert.glsl", os2.O_WRONLY | os2.O_CREATE | os2.O_TRUNC)
	os2.write(file, transmute([]u8)(string(codeString)))
	os2.close(file)

	slang.releaseBlob(codeBlob)

	codeBlob = slang.getEntryPointCode(linkedProgram, 0, 0, nil)
	if codeBlob == nil {
		panic("Failed to get entry point code\n")
	}

	codeString = (cstring)(slang.getBlobData(codeBlob))

	file, _ = os2.open("frag.glsl", os2.O_WRONLY | os2.O_CREATE | os2.O_TRUNC)
	os2.write(file, transmute([]u8)(string(codeString)))
	os2.close(file)

	slang.releaseBlob(codeBlob)

	slang.releaseComponentType(linkedProgram)
	slang.releaseComponentType(program)
	slang.releaseEntryPoint(vertEntryPoint)
	slang.releaseEntryPoint(fragEntryPoint)
	slang.releaseModule(module)

	module = slang.loadModule(session, "./shaders/ComputeDemo.slang", nil)
	if module == nil {
		panic("Failed to load module\n")
	}

	compEntryPoint := slang.findEntryPoint(module, "computeMain", .COMPUTE, nil)
	if compEntryPoint == nil {
		panic("Failed to find entry point\n")
	}

	components = []slang.Component_Type {
		{kind = .MODULE, module = module},
		{kind = .ENTRY_POINT, entryPoint = compEntryPoint},
	}
	program = slang.createCompositeComponentType(
		session,
		raw_data(components),
		i32(len(components)),
		nil,
	)
	if program == nil {
		panic("Failed to create program\n")
	}

	linkedProgram = slang.linkComponentType(program, nil)
	if linkedProgram == nil {
		panic("Failed to link program\n")
	}

	codeBlob = slang.getEntryPointCode(linkedProgram, 0, 0, nil)
	if codeBlob == nil {
		panic("Failed to get entry point code\n")
	}

	codeString = (cstring)(slang.getBlobData(codeBlob))

	file, _ = os2.open("comp.glsl", os2.O_WRONLY | os2.O_CREATE | os2.O_TRUNC)
	os2.write(file, transmute([]u8)(string(codeString)))
	os2.close(file)

	slang.releaseBlob(codeBlob)

	slang.releaseComponentType(linkedProgram)
	slang.releaseComponentType(program)
	slang.releaseEntryPoint(compEntryPoint)
	slang.releaseModule(module)

	slang.releaseSession(session)
	slang.releaseGlobalSession(globalSession)
	slang.shutdown()
}
