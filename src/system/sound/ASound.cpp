// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "ASound.h"

#include "common/Event.h"
#include "common/Utils.h"

namespace mod {

ASound::ASound() : Logger("ASound") {

    mVoiceDb = {0.0, -50.0};

    connect(&Event::getInstance(), &Event::uiCompleted, this, &ASound::onUiCompleted);
}

void ASound::onUiCompleted() { setDb(mVoiceDb); }

bool ASound::setDb(VoiceDb val) {
    mVoiceDb = val;
    return _resetConfig();
}

ASound::VoiceDb ASound::getDb() { return mVoiceDb; }

bool ASound::_resetConfig() {
    auto cfg     = _getConfig();
    auto content = QString::fromStdString(cfg.mContent)
                       .replace("{mindb}", QString::number(mVoiceDb.min, 'f', 1))
                       .replace("{maxdb}", QString::number(mVoiceDb.max, 'f', 1))
                       .toStdString();
    std::ofstream ofile(cfg.mPath);
    if (ofile.good()) {
        ofile << content;
    } else {
        return false;
    }
    ofile.close();
    ofile.open("/etc/asound.conf");
    if (ofile.good()) {
        ofile << content;
    } else {
        return false;
    }
    ofile.close();
    return true;
}

std::string ASound::_getRawConfigure(const char* model) {
    switch (H(model)) {
    case H("V4"):
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "plug_ply" # no use ladspa path for eq_drc_process
    capture.pcm "plug:dsnooper"
}

# start for soft playback >>>
pcm.plug_ply {
    type plug
    slave.pcm "softvol_ply"
}

pcm.dsnooper {
    type dsnoop
    ipc_key 12342 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid true
    slave {
        pcm "hw:0,0"
        channels 2
        rate 48000
    }
    bindings {
        0 0
        1 1
    }
}
pcm.dmixer {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:7,0,0"
        channels 2
        period_size 1024
        buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.softvol_ply {
    type softvol
    # slave.pcm "hooks_ply" # no use eq_drc_process
    # slave.pcm "hw:7,0,0" # using eq_drc_process for loopback
    # slave.pcm "rk_eqdrc" # using eq_drc_process for loopback
    slave.pcm "dmixer" # using eq_drc_process for loopback
    control {
        name "MasterP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}
# end for soft playback <<<

# start for spk playpath >>>
pcm.playback {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "SPK"
            lock false
        }]
    }
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "plug:ladspa_play"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
# end for spk playpath <<<

# start fot soft capture >>>
pcm.softvol_cap {
    type softvol
    slave.pcm "hw:0,1"
}
# end for soft capture <<<

# start for fake record >>>
pcm.fake_record {
    type plug
    slave.pcm "hw:7,1,0"
}
# end for fake record <<<

# start for fake playpath >>>
pcm.fake_play {
    type plug
    slave.pcm "rk_eqdrc" # with ladspa path for eq_drc_process
}
# end for fake playpath <<<

# start for digital headset:fake_jack >>>
pcm.fake_jack {
    type plug
    slave.pcm "dig_hp"
}
pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}
# end for digital headset <<<

# for ana headset:fake_jack2 >>>
pcm.fake_jack2 {
    type plug
    slave.pcm "ana_phone"
}

pcm.hooks_ana_phone {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "HP"
            lock false
        }]
    }
}

pcm.softvol_ana_phone {
    type softvol
    slave.pcm "hooks_ana_phone"
    control {
        name "MasterAHP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}

pcm.ana_phone
{
    type plug
    slave {
        pcm "softvol_ana_phone"
        rate 48000
    }
}
# end for ana headset <<<

pcm.multi_2 {
    type multi
    slaves.a.pcm "hw:0,1"
    slaves.a.channels 4
    bindings.0.slave a
    bindings.0.channel 2
    bindings.1.slave a
    bindings.1.channel 3
}

pcm.2mic
{
    type plug
    slave.pcm "multi_2"
}
)";
    case H("Mango6L"):
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "plug_ply" # no use ladspa path for eq_drc_process
    capture.pcm "hw:0,0"
}

# start for soft playback >>>
pcm.plug_ply {
    type plug
    slave.pcm "softvol_ply"
}

pcm.dmixer {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:7,0,0"
        channels 2
        period_size 1024
        buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.softvol_ply {
    type softvol
    # slave.pcm "hooks_ply" # no use eq_drc_process
    # slave.pcm "hw:7,0,0" # using eq_drc_process for loopback
    # slave.pcm "rk_eqdrc" # using eq_drc_process for loopback
    slave.pcm "dmixer" # using eq_drc_process for loopback
    control {
        name "MasterP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}
# end for soft playback <<<

# start for spk playpath >>>
pcm.playback {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "SPK"
            lock false
        }]
    }
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "plug:ladspa_play"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
# end for spk playpath <<<

# start fot soft capture >>>
pcm.softvol_cap {
    type softvol
    slave.pcm "hw:0,1"
}
# end for soft capture <<<

# start for fake record >>>
pcm.fake_record {
    type plug
    slave.pcm "hw:7,1,0"
}
# end for fake record <<<

# start for fake playpath >>>
pcm.fake_play {
    type plug
    slave.pcm "rk_eqdrc" # with ladspa path for eq_drc_process
}
# end for fake playpath <<<

# start for digital headset:fake_jack >>>
pcm.fake_jack {
    type plug
    slave.pcm "dig_hp"
}
pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}
# end for digital headset <<<

# for ana headset:fake_jack2 >>>
pcm.fake_jack2 {
    type plug
    slave.pcm "ana_phone"
}

pcm.hooks_ana_phone {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "HP"
            lock false
        }]
    }
}

pcm.softvol_ana_phone {
    type softvol
    slave.pcm "hooks_ana_phone"
    control {
        name "MasterAHP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}

pcm.ana_phone
{
    type plug
    slave {
        pcm "softvol_ana_phone"
        rate 48000
    }
}
# end for ana headset <<<

pcm.multi_2 {
    type multi
    slaves.a.pcm "hw:0,1"
    slaves.a.channels 4
    bindings.0.slave a
    bindings.0.channel 2
    bindings.1.slave a
    bindings.1.channel 3
}

pcm.2mic
{
    type plug
    slave.pcm "multi_2"
}
)";

    case H("Exam"):
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "plug_ply" # no use ladspa path for eq_drc_process
    capture.pcm "plug:dsnooper"
}

# start for soft playback >>>
pcm.plug_ply {
    type plug
    slave.pcm "softvol_ply"
}

pcm.dsnooper {
    type dsnoop
    ipc_key 12342 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid true
    slave {
        pcm "hw:0,0"
        channels 2
        rate 48000
    }
    bindings {
        0 0
        1 1
    }
}
pcm.dmixer {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:7,0,0"
        channels 2
        period_size 1024
        buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.softvol_ply {
    type softvol
    # slave.pcm "hooks_ply" # no use eq_drc_process
    # slave.pcm "hw:7,0,0" # using eq_drc_process for loopback
    # slave.pcm "rk_eqdrc" # using eq_drc_process for loopback
    slave.pcm "dmixer" # using eq_drc_process for loopback
    control {
        name "MasterP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}
# end for soft playback <<<

# start for spk playpath >>>
pcm.playback {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "SPK"
            lock false
        }]
    }
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "plug:ladspa_play"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
# end for spk playpath <<<

# start fot soft capture >>>
pcm.softvol_cap {
    type softvol
    slave.pcm "hw:0,1"
}
# end for soft capture <<<

# start for fake record >>>
pcm.fake_record {
    type plug
    slave.pcm "hw:7,1,0"
}
# end for fake record <<<

# start for fake playpath >>>
pcm.fake_play {
    type plug
    slave.pcm "rk_eqdrc" # with ladspa path for eq_drc_process
}
# end for fake playpath <<<

# start for digital headset:fake_jack >>>
pcm.fake_jack {
    type plug
    slave.pcm "dig_hp"
}
pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}
# end for digital headset <<<

# for ana headset:fake_jack2 >>>
pcm.fake_jack2 {
    type plug
    slave.pcm "ana_phone"
}

pcm.hooks_ana_phone {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "HP"
            lock false
        }]
    }
}

pcm.softvol_ana_phone {
    type softvol
    slave.pcm "hooks_ana_phone"
    control {
        name "MasterAHP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}

pcm.ana_phone
{
    type plug
    slave {
        pcm "softvol_ana_phone"
        rate 48000
    }
}
# end for ana headset <<<

pcm.multi_2 {
    type multi
    slaves.a.pcm "hw:0,1"
    slaves.a.channels 4
    bindings.0.slave a
    bindings.0.channel 2
    bindings.1.slave a
    bindings.1.channel 3
}

pcm.2mic
{
    type plug
    slave.pcm "multi_2"
}
)";

    case H("Cherry"):
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "plug_ply" # no use ladspa path for eq_drc_process
    capture.pcm "hw:0,1"
}

# start for soft playback >>>
pcm.plug_ply {
    type plug
    slave.pcm "softvol_ply"
}

pcm.dmixer {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:7,0,0"
        channels 2
        period_size 1024
        buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.softvol_ply {
    type softvol
    # slave.pcm "hooks_ply" # no use eq_drc_process
    # slave.pcm "hw:7,0,0" # using eq_drc_process for loopback
    # slave.pcm "rk_eqdrc" # using eq_drc_process for loopback
    slave.pcm "dmixer" # using eq_drc_process for loopback
    control {
        name "MasterP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}
# end for soft playback <<<

# start for spk playpath >>>
pcm.playback {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "SPK"
            lock false
        }]
    }
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "plug:ladspa_play"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
# end for spk playpath <<<

# start fot soft capture >>>
pcm.softvol_cap {
    type softvol
    slave.pcm "hw:0,1"
}
# end for soft capture <<<

# start for fake record >>>
pcm.fake_record {
    type plug
    slave.pcm "hw:7,1,0"
}
# end for fake record <<<

# start for fake playpath >>>
pcm.fake_play {
    type plug
    slave.pcm "rk_eqdrc" # with ladspa path for eq_drc_process
}
# end for fake playpath <<<

# start for digital headset:fake_jack >>>
pcm.fake_jack {
    type plug
    slave.pcm "dig_hp"
}
pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}
# end for digital headset <<<

# for ana headset:fake_jack2 >>>
pcm.fake_jack2 {
    type plug
    slave.pcm "ana_phone"
}

pcm.hooks_ana_phone {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "HP"
            lock false
        }]
    }
}

pcm.softvol_ana_phone {
    type softvol
    slave.pcm "hooks_ana_phone"
    control {
        name "MasterAHP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}

pcm.ana_phone
{
    type plug
    slave {
        pcm "softvol_ana_phone"
        rate 48000
    }
}
# end for ana headset <<<

pcm.multi_2 {
    type multi
    slaves.a.pcm "hw:0,1"
    slaves.a.channels 4
    bindings.0.slave a
    bindings.0.channel 2
    bindings.1.slave a
    bindings.1.channel 3
}

pcm.2mic
{
    type plug
    slave.pcm "multi_2"
}
)";

    case H("Cherry_3566"):
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "plug_ply" # no use ladspa path for eq_drc_process
    capture.pcm "hw:0,1"
}

# start for soft playback >>>
pcm.plug_ply {
    type plug
    slave.pcm "softvol_ply"
}

pcm.dmixer {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:7,0,0"
        channels 2
        period_size 1024
        buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.softvol_ply {
    type softvol
    # slave.pcm "hooks_ply" # no use eq_drc_process
    # slave.pcm "hw:7,0,0" # using eq_drc_process for loopback
    # slave.pcm "rk_eqdrc" # using eq_drc_process for loopback
    slave.pcm "dmixer" # using eq_drc_process for loopback
    control {
        name "MasterP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}
# end for soft playback <<<

# start for spk playpath >>>
pcm.playback {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "SPK"
            lock false
        }]
    }
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "plug:ladspa_play"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
# end for spk playpath <<<

# start fot soft capture >>>
pcm.softvol_cap {
    type softvol
    slave.pcm "hw:0,1"
}
# end for soft capture <<<

# start for fake record >>>
pcm.fake_record {
    type plug
    slave.pcm "hw:7,1,0"
}
# end for fake record <<<

# start for fake playpath >>>
pcm.fake_play {
    type plug
    slave.pcm "rk_eqdrc" # with ladspa path for eq_drc_process
}
# end for fake playpath <<<

# start for digital headset:fake_jack >>>
pcm.fake_jack {
    type plug
    slave.pcm "dig_hp"
}
pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}
# end for digital headset <<<

# for ana headset:fake_jack2 >>>
pcm.fake_jack2 {
    type plug
    slave.pcm "ana_phone"
}

pcm.hooks_ana_phone {
    type hooks
    slave.pcm "hw:0,0"
    hooks.0 {
        type ctl_elems
        hook_args [{
            name "Playback Path"
            preserve true
            value "HP"
            lock false
        }]
    }
}

pcm.softvol_ana_phone {
    type softvol
    slave.pcm "hooks_ana_phone"
    control {
        name "MasterAHP Volume"
        card 0
        device 0
    }
    min_dB {mindb}
    max_dB {maxdb}
    resolution 100
}

pcm.ana_phone
{
    type plug
    slave {
        pcm "softvol_ana_phone"
        rate 48000
    }
}
# end for ana headset <<<

pcm.multi_2 {
    type multi
    slaves.a.pcm "hw:0,1"
    slaves.a.channels 4
    bindings.0.slave a
    bindings.0.channel 2
    bindings.1.slave a
    bindings.1.channel 3
}

pcm.2mic
{
    type plug
    slave.pcm "multi_2"
})";
    case H("default"):
    default:
        return R"(defaults.pcm.rate_converter "speexrate_medium"
pcm.!default
{
    type asym
    playback.pcm "rk_eqdrc"
#    playback.pcm {
#        type plug
#        slave.pcm "rk_eqdrc"
#        #slave.pcm "hw:0,0"
#	#slave.pcm "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=C9:50:76:23:68:BB"
#    }
    capture.pcm {
        type plug
        slave.pcm "hw:0,0"
    #slave.pcm "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=C9:50:76:23:68:BB"
    }
}

pcm.playback {
    type dmix
    ipc_key 5978293 # must be unique for all dmix plugins!!!!
    ipc_key_add_uid yes
    slave {
        pcm "hw:0,0"
        channels 2
        format S16_LE
        rate 48000
        # period_size 1024
        # buffer_size 4096
    }
    bindings {
        0 0
        1 1
    }
}

pcm.dig_hp {
    type plug
    slave.pcm "hw:1,0"
}

pcm.rk_eqdrc {
    type plug
    slave {
        pcm {
            type softvol
            slave.pcm "ladspa_plug"
            control {
                name "Master Playback Volume"
                card 0
            }
            min_dB {mindb}
            max_dB {maxdb}
            resolution 256
        }
        channels 2
        format S16_LE
        rate 48000
    }
}

pcm.ladspa_play {
    type ladspa
    # slave.pcm "hw:0,0"
    slave.pcm "plug:playback"
    path "/usr/share/alsa/"
    playback_plugins [{
        label eq_drc_stereo
            input {
                controls [0]
            }
    }]
}

pcm.ladspa_plug {
    type plug
    slave {
        pcm "ladspa_play"
    }
}
)";
    }
}

ASound::Config ASound::_getConfig() {
    auto pcba = exec("get_pcba_version");
    if (pcba == "Dictpen2.0_V4") {
        return {"/etc/asound.conf.V4", _getRawConfigure("V4")};
    }
    if (pcba == "Cherry_V0" || pcba == "Mango_V0" || pcba == "Kiwi-3326_V0") {
        return {"/etc/asound.conf.VCherry", _getRawConfigure("Cherry")};
    }
    if (pcba == "Mango_V1") {
        return {"/etc/asound.conf.VMango6L", _getRawConfigure("Mango6L")};
    }
    if (pcba == "Cherry-3566_V0" || pcba == "Kiwi-3566_V0") {
        return {"/etc/asound.conf.VCherry_3566", _getRawConfigure("Cherry_3566")};
        //} else if (pcba == "Apollo_V0") {
        //    return {"/etc/asound.conf.VApollo",asound_Apollo};
    }
    if (pcba == "Exam_V0") {
        return {"/etc/asound.conf.VExam", _getRawConfigure("Exam")};
    }
    warn("Unable to find a matching asound configuration file for this pcba({}).", pcba);
    return {"/etc/asound.conf", _getRawConfigure("default")};
}

} // namespace mod
