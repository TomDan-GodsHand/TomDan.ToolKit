set_config("compile_commands",true)

set_languages("cxx26")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

add_requires("cpp-httplib")

-- 包含子项目
includes("Interface", "Core", "Plugin.Example" )
