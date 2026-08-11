# Build Notes

Requirements: CMake 3.22+, C++17 compiler, and JUCE `91ad83ae34a81e0833b1a2b0866f54846370ae53`.

`engine-debug` builds only the JUCE-independent DSP tests. `plugin-release` builds VST3 and Standalone on every platform and AU on Apple, then stages products through `ehl_stage_products`.

For local offline verification on this workspace, pass the existing Plitch JUCE checkout:

```sh
cmake --preset plugin-release -DEHL_JUCE_SOURCE_DIR=/Users/2bit/prog/juce/Plitch/build/release/_deps/juce-src
```
