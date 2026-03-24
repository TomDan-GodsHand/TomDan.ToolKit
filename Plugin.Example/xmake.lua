add_rules("mode.debug", "mode.release")

target("Plugin.Example")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_deps("Interface")
    add_files("src/*.cpp")
    -- 根据平台和架构设置目标目录
    set_targetdir("../build/$(plat)/$(arch)/$(mode)/plugins")
