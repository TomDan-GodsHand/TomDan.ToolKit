add_rules("mode.debug", "mode.release")

target("TomDan.ToolKit.HttpServer")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_includedirs("include")
    add_deps("TomDan.ToolKit.Interface")
    add_files("src/*.cpp")

    add_packages("cpp-httplib")

    set_targetdir("../build/linux/x86_64/debug/plugins")

