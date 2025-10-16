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

set_allowedarchs('linux|arm64-v8a')

option('qemu')
    set_default(false)
    set_showmenu(true)
    set_description('Enable build for QEMU.')

option('build-platform')
    set_default('YDP02X')
    set_showmenu(true)
    set_description('Enable build for specific devices.')
    set_values('YDP02X', 'YDPG3', 'YDP03X')

option('target-channel')
    set_default('priv')
    set_showmenu(true)
    set_description('Tweak the compilation results in release.')
    set_values('priv', 'canary', 'beta', 'stable')

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
        'lame',
        'elfio',
        'nlohmann_json',
        'boost',
        'dobby')
    add_includedirs(
        'src',
        'src/base',
        '$(buildir)/config')
    add_links(
        -- crypt, src/helper/ServiceManager.cpp
        -- 'crypt', 
        -- dladdr, src/common/util/System.cpp
        'dl')
    set_warnings('all')
    set_languages('cxx14', 'c99')
    set_exceptions('cxx')
    set_pcxxheader('src/base/Base.h')
    set_configdir('$(buildir)/config')
    add_configfiles('src/mod/Version.h.in')
    set_configvar('TARGET_CHANNEL', get_config('target-channel'))
    
    if is_mode('release') then
        if is_config('target-channel', 'priv') then
            set_symbols('debug')
        else
            set_symbols('hidden')
            set_optimize('smallest')
            set_strip('all')
        end
    end
    
    if is_mode('debug') then
        add_defines('DEBUG')
        set_symbols('debug')
        set_optimize('none')
    end

    if has_config('qemu') then
        add_defines('QEMU')
    end

    if is_config('build-platform', 'YDP02X') then
        add_defines('DICTPEN_YDP02X')
    end

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
    set_warnings('all')
    set_languages('cxx14', 'c99')
    set_exceptions('cxx')
