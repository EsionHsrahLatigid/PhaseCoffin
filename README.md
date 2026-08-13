# PhaseCoffin

PhaseCoffin is an EHL Digital Harsh Noise phaser. It uses two six-stage first-order allpass banks, sub-unity feedback, stereo phase offset, and an optional barberpole-style crossfade between offset banks to carve moving hollow notches without using flanger delay lines.

## Identity

- Product: `PhaseCoffin`
- Repository slug: `phasecoffin`
- Bundle ID: `jp.ehl.phasecoffin`
- Manufacturer: `EsionHsrahLatigid`
- Manufacturer code: `EHL_`
- Plugin code: `PhCf`

## Parameters

- `rate`: LFO speed, 0.02 Hz to 20 Hz.
- `depth`: sweep width around the center frequency.
- `center`: middle frequency for the allpass notch field.
- `spread`: octave spacing across the active stages.
- `stages`: active first-order allpass stages per bank, 1 to 6.
- `feedback`: bounded wet feedback, clamped below unity.
- `barberDirection`: Off, Up, or Down crossfade direction.
- `coffinSkew`: asymmetric sweep bend for harsher clustered notches.
- `stereoPhase`: right-channel LFO offset.
- `mix`: dry to notched wet balance.
- `trim`: output gain after the mix.

## Build

```sh
cmake --preset engine-debug
cmake --build --preset engine-debug
ctest --preset engine-debug --output-on-failure

cmake --preset plugin-release -DEHL_JUCE_SOURCE_DIR=/path/to/JUCE
cmake --build --preset plugin-release --target ehl_stage_products
ctest --preset plugin-release --output-on-failure
```

The project pins JUCE to `91ad83ae34a81e0833b1a2b0866f54846370ae53` when network FetchContent is used. Set `EHL_JUCE_SOURCE_DIR` for offline builds.

On local macOS builds, VST3 and AU products are copied after build to the current user's standard Audio Plug-Ins folders. CI and non-macOS builds leave this off by default. Override with `-DEHL_COPY_PLUGIN_AFTER_BUILD=ON|OFF`; Standalone products are only staged under the artifact tree.

Stable artifacts:

```text
artifacts/plugin-release/macos-arm64/standalone/phasecoffin_standalone_plugin.app
artifacts/plugin-release/macos-arm64/vst3/phasecoffin_vst3_plugin.vst3
artifacts/plugin-release/macos-arm64/au/phasecoffin_au_plugin.component
artifacts/plugin-release/macos-arm64/ARTIFACTS.txt

artifacts/plugin-release/windows-x64/standalone/phasecoffin_standalone_plugin.exe
artifacts/plugin-release/windows-x64/vst3/phasecoffin_vst3_plugin.vst3
artifacts/plugin-release/windows-x64/ARTIFACTS.txt
```

## Verification Targets

- `phasecoffin_dsp_tests`
- `phasecoffin_plugin_tests`
- `phasecoffin_editor_tests`
- `ehl_stage_products`

The DSP tests cover allpass energy, moving notch response, stereo divergence, feedback bounds, bypass mix, deterministic reset, silence, non-finite input, and mono/stereo operation. This plugin enforces digital finite bounds; it does not claim SPL or hearing-safety protection.
