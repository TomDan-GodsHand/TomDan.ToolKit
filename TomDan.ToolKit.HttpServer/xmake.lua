add_rules("mode.debug", "mode.release")

target("TomDan.ToolKit.HttpServer")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_includedirs("include")
    add_deps("TomDan.ToolKit.Interface")
    add_files("src/*.cpp")

    add_packages("cpp-httplib")

    -- 平台特定的系统链接
    if is_plat("windows", "mingw") then
        add_syslinks("ws2_32")
    end

    -- 根据平台和架构设置目标目录
    set_targetdir("../build/$(plat)/$(arch)/$(mode)/plugins")

