function krutolib(arch)
  project(string.format("krutoc%s", arch))
  kind "SharedLib"
  files { "src/lib/**.hpp", "src/lib/**.cpp" }
  links { "dl" }
  buildoptions { string.format("-m%s", arch) }
  linkoptions { string.format("-m%s", arch) }
end

solution "krutoc"
  objdir "build"
  targetdir "build"
  language "C++"

  if _ACTION == "clean" then
    os.rmdir("build")
  end

  configurations { "debug", "release" }
  includedirs { "src/lib" }
  libdirs { "build" }

  configuration "debug"
    defines { "KR_DEBUG" }

  configuration "release"
    defines { "KR_NDEBUG" }

  configuration { "release", "gmake" }
    buildoptions { "-O3" }

  configuration { "gmake" }
    buildoptions { "-fdiagnostics-show-option", "-Wall", "-Wextra", "-Werror" }
    postbuildcommands { "cp src/krutoc build/krutoc" }

  krutolib("32")

  if os.is64bit() then
    krutolib("64")
  end

newoption 
{
  trigger     = "prefix",
  value       = "path",
  description = "Install directory (default: /opt/krutoc)"
}

newaction
{
  trigger = "install",
  description = "Install into a specified directory",
  execute = function ()
    local files = { "krutoc", "libkrutoc32.so" }
    if os.is64bit() then
      table.insert(files, "libkrutoc64.so")
    end

    local prefix = "/opt/krutoc"

    if _OPTIONS["prefix"] then
      prefix = _OPTIONS["prefix"]
    end

    if not os.mkdir(prefix) then
      print(string.format("Failed to create %s directory. Aborting ...", prefix))
      return
    end

    for i, file in pairs(files) do
      local src = string.format("build/%s", file)
      local dst = string.format("%s/%s", prefix, file)

      if os.isfile(src) then
        os.copyfile(src, dst)
      else
        print(string.format("Failed to copy %s to %s. Aborting ...", src, dst))
        break
      end
    end
  end
}
