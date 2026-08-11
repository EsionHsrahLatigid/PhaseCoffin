# Research And Decision Map

## Evidence

- Julius O. Smith's PASP phasing material supports the established phaser identity used here: cascaded allpass sections mixed with dry signal to create moving notches.
- Hartmann's flanging/phasing context supports keeping this product distinct from JetScab: PhaseCoffin uses allpass phase rotation, not a short modulated delay line.
- JUCE APVTS and the local Plitch/DHN9 scaffold pattern support host-visible parameters, state round-trip tests, and custom editor verification.
- G001 selected PhaseCoffin as `jp.ehl.phasecoffin` / `PhCf` with dual six-stage allpass banks, smoothed coefficients, feedback, stereo phase, and optional barberpole-style motion.

## Decisions

- The core is two six-stage first-order allpass banks. Each stage uses the stable first-order form `y[n] = -a x[n] + z[n-1]`, `z[n] = x[n] + a y[n]`, with `a` clamped to `[-0.985, 0.985]`.
- Dry and allpass output are differenced for aggressive notches. This is intentionally harsher than a polite vintage phaser but remains bounded and deterministic.
- Feedback is applied from bounded wet allpass output and capped to `[-0.88, 0.88]`.
- Stereo width comes from a right-channel LFO phase offset rather than hidden delay.
- The barberpole mode is characterized honestly as barberpole-style: it crossfades between two offset allpass banks in Up or Down direction. It is not a mathematically continuous Shepard/Risset phaser and does not claim infinite perceptual rise/fall.
- Parameter and coefficient changes are smoothed inside the DSP core. Memory is fixed at prepare/reset time; the sample path has no allocation, locks, I/O, or logging.

## Rejected Alternatives

- A short delay-line flanger core was rejected because it belongs to JetScab and would blur the effect-class boundary.
- Static allpass/EQ notches were rejected because G001 requires performative sweep motion.
- Feedback at or above unity was rejected because the DHN9 safety invariant requires finite deterministic long renders.

## Test Oracles

- A standalone allpass bank preserves impulse energy within tolerance.
- A moving sweep changes deterministic sine-mixture response over time.
- Stereo phase offset creates repeatable left/right divergence.
- Maximum feedback renders remain finite and decay under silence.
- Mix zero is dry bypass.
- Reset, silence, non-finite input, mono prepare, and stereo prepare are deterministic and finite.
