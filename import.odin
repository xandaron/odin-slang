when ODIN_OS == .Windows {
    foreign import lib {
        "gfx.lib",
        "slang-rt.lib",
        "slang.lib",
    }
}
else {
    foreign import lib {
        "gfx.a",
        "slang-rt.a",
        "slang.a",
    }
}