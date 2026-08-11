# Design

## DSP

PhaseCoffin is a harsh allpass phaser. The processor owns two fixed six-stage first-order allpass banks per channel. The LFO maps rate, depth, center, spread, coffin skew, and stereo phase into smoothed allpass coefficients; stages are clamped inside the stable unit circle. The wet signal is a dry/allpass difference for deep notches, followed by sub-unity feedback, wet/dry mix, and output trim.

The optional barber mode crossfades between two offset allpass banks. The UI labels it as `BARBER`; documentation calls it barberpole-style because it is a bounded crossfade illusion, not an unbounded infinite-rise implementation.

The audio callback and direct DSP callees do not allocate heap memory, acquire locks, log, touch the filesystem/network, throw exceptions, or perform unbounded loops. Inputs, parameters, coefficients, feedback samples, and outputs are sanitized and clamped to finite ranges.

## UI

The editor uses the DHN9 monochrome 8-bit system: 8 px grid, grayscale palette, fixed procedural phase-coffin/notch motif, no external images, and no external fonts. The default size is 960 x 544 and the minimum is 720 x 432.

Every parameter has an attached visible control with a stable component ID (`phasecoffin-<parameter-id>`), accessible name/title/description, tooltip, and keyboard focus. `GenericAudioProcessorEditor` is banned and covered by tests.
