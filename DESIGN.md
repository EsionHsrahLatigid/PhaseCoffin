# Design

## DSP

PhaseCoffin is a harsh allpass phaser. The processor owns two fixed six-stage first-order allpass banks per channel. The LFO maps rate, depth, center, spread, coffin skew, and stereo phase into smoothed allpass coefficients; stages are clamped inside the stable unit circle. The wet signal is a dry/allpass difference for deep notches, followed by sub-unity feedback, wet/dry mix, and output trim.

The optional barber mode crossfades between two offset allpass banks. The UI labels it as `BARBER`; documentation calls it barberpole-style because it is a bounded crossfade illusion, not an unbounded infinite-rise implementation.

The audio callback and direct DSP callees do not allocate heap memory, acquire locks, log, touch the filesystem/network, throw exceptions, or perform unbounded loops. Inputs, parameters, coefficients, feedback samples, and outputs are sanitized and clamped to finite ranges.

## UI

The editor uses the strict DHN9 simple monochrome 8-bit contract: flat four-level palette only (`#050505`, `#2A2A2A`, `#8A8A86`, `#F2F2F0`), 4 px base grid / 8 px major spacing, product name at `y=16`, compact function label at `y=48`, one 1 px divider at `y=72`, and controls starting at absolute `y=80`. There is no full-canvas grid, tagline, package ID, decorative motif, fake visualizer, meter, panel, outer frame, or parameter-driven atmospheric paint. The default size remains 960 x 544 and the minimum remains 720 x 432.

Every parameter has an attached visible control with a stable component ID (`phasecoffin-<parameter-id>`), accessible name/title/description, tooltip, and keyboard focus. `GenericAudioProcessorEditor` is banned and covered by tests. DSP, parameter IDs, bundle ID, and accessibility contracts are unchanged.
