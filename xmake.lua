add_rules('mode.release', 'mode.debug')

add_requires('spdlog        1.15.3')
add_requires('elfio         3.12')
add_requires('nlohmann_json 3.12.0')
add_requires('boost         1.88.0')
add_requires('dobby         2023.4.14')
add_requires('lame          3.100', {
    -- DictPen's buildroot exists lame v3.100,
    -- so we use it as a shared library.
    configs = {shared = true}
})

--- options

option('qemu')
    set_default(false)
    set_showmenu(true)
    set_description('Enable build for QEMU.')
    add_defines('PL_QEMU')
option_end()

option('build-platform')
    set_default('YDP02X')
    set_showmenu(true)
    set_description('Enable build for specific devices.')
    set_values('YDP02X', 'YDPG3', 'YDP03X')
option_end()

option('target-channel')
    set_default('dev')
    set_showmenu(true)
    set_description('Tweak the compilation results in release.')
    set_values('dev', 'canary', 'beta', 'stable')
option_end()

--- global configs

set_version('2.0.0')

set_allowedarchs('linux|arm64-v8a')

-- The libstdc++ that shipped with DictPen only supports c++14, 
-- and we need more new language features.
set_runtimes('c++_static')
set_languages('cxx23', 'c99')

set_warnings('all')
set_exceptions('cxx')

set_configdir('$(builddir)/config')
add_configfiles('src/mod/Version.h.in')

if is_mode('debug') then
    add_defines('PL_DEBUG')
end

if is_config('build-platform', 'YDP02X') then
    add_defines('DICTPEN_YDP02X')
end

--- targets

target('PenMods')
    add_rules('qt.shared')
    add_files('src/**.cpp')
    add_files('src/**.h')
    add_frameworks(
        'QtNetwork',
        'QtQuick',
        'QtQml',
        'QtGui',
        'QtMultimedia',
        'QtWebSockets',
        'QtSql')
    add_packages(
        'spdlog',
        'elfio',
        'nlohmann_json',
        'boost',
        'dobby',
        'lame')
    set_pcxxheader('src/base/Base.h')
    add_includedirs(
        'src',
        'src/base',
        '$(builddir)/config')
    add_links(
        -- crypt, src/helper/ServiceManager.cpp
        -- 'crypt', 
        -- dladdr, src/common/util/System.cpp
        'dl')
    
    on_config(function (target) 
        target:add('defines', 'PL_BUILD_' .. get_config('build-platform'))
        target:add('defines', 'PL_' .. get_config('target-channel'):upper() .. '_CHANNEL')
    end)

    on_run(function(target)
        os.exec(('$(projectdir)/scripts/install.sh %s %s'):format(
            get_config('mode'),
            get_config('build-platform'))
        )
    end)
    
target('QrcExporter')
    add_rules('qt.shared')
    add_files('resource/exporter/**.cpp')
    add_packages(
        'spdlog',
        'dobby')
