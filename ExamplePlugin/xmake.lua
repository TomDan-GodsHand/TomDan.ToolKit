add_rules("mode.debug", "mode.release")

target("ExamplePlugin")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_includedirs("../TomDan.ToolKit.Interface/include")
    add_deps("TomDan.ToolKit.Interface")
    add_files("src/*.cpp")
    set_targetdir("../build/linux/x86_64/debug/plugins")
