add_rules("mode.debug", "mode.release")

target("ExamplePlugin")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_includedirs("../TomDan.ToolKit.Interface/include")
    add_deps("TomDan.ToolKit.Interface")
    add_files("src/*.cpp")
    -- 根据平台和架构设置目标目录
    set_targetdir("../build/$(plat)/$(arch)/$(mode)/plugins")
