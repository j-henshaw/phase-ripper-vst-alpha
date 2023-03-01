# phase-ripper-vst-alpha
This is the alpha version of a VST audio plugin named the Shearing Force Phase Ripper.
This plugin was developed with Joshua Sterner.

The function of the Shearing Force Phase Ripper is to take an incoming audio signal, and induce phase-flips in the signal at a rate that is harmonically related to the sound itself. In its current state, the Phase Ripper accepts incoming MIDI signals, and flips the audio at the frequency of the MIDI notes received. This makes it easy to send the same MIDI information to a synthesizer which is running into the Phase Ripper—and to the Phase Ripper itself. A future version with monophonic pitch detection to automatically flip incoming signals that may or may not come from a MIDI-powered synthesizer. There is also a button matrix, where extra harmonics of the fundamental frequency of the MIDI notes can be added into the control signal for flipping the audio signal's phase, creating an even richer sound. The sound of the Shearing Force Phase Ripper is quite unique, though it has a distinctly digital sound, similar to bitcrushing. The effect can create massive, noisy, washy sounds that take up little headroom.

No proper GUI has been developed, but it has been tested to run in Reaper with a placeholder GUI. The effect may or may not work in other DAWs.
