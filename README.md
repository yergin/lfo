MIDI CC messages
================

| CC # | CC Name | Range | Function |
| --- | --- | --- | --- |
| 1 | Modulation Wheel | 0-127 | Rate  |
| 5 | Portamento | 0-127 | Ramp Time |
| 7 | Volume | 0-127 | Master Volume |
| 11 | Expression | 0-127 | Expression |
| 70 | Sound Controller 1 | 0-1 | Mode: Vibrato or Chorus |
| 91 | Reverb | 0-127 | Autopan Width |
| 92 | Tremolo | 0-127 | Tremolo / Autopan Depth |
| 93 | Chorus | 0-127 | Vibrato / Chorus Depth |
| 94 | Detune | 0-127 | Rotary Phase |
| 95 | Phaser | 0-127 | Phaser |

Compressor control
------------------

| CC # | CC Name | Range | Function |
| --- | --- | --- | --- |
| 72 | Sound Controller 3 | 0-127 | Compressor release |
| 73 | Sound Controller 4 | 0-127 | Compressor attack |
| 74 | Sound Controller 5 | 0-127 | Compressor threshold |
| 75 | Sound Controller 6 | 0-127 | Compressor amount |

Parameters
==========

Rate (CC1)
----------

| Value | Hz | RPM |
| --- | --- | --- |
| 0 | 0 | 0 |
| 12 | 0.3 | 18 |
| 24* | 0.75 | 48 (Slow Leslie) |
| 29 | 1 | 60 |
| 47 | 2.5 | 150 |
| 72 | 6.6 | 400 (Fast Leslie) |
| 99 | 15 | 900 |
| 127 | 30 | 1800 |

Ramp Time (CC5)
---------------

| Value | Time |
| --- | --- |
| 0 | - |
| 10 | 100 ms |
| 32 | 0.5 sec |
| 50 | 1 sec |
| 75* | 2 sec |
| 104 | 3.5 sec |
| 127 | 5 sec |

Master Volume, Expression (CC7, CC11)
-------------------------------------

| Value | Level |
| --- | --- |
| 0 | -inf dB |
| 14 | -24 dB |
| 46 | -12 dB |
| 60 | -9 dB |
| 78 | -6 dB |
| 100* | -3 dB |
| 127 | 0 dB |

Mode (CC70)
-----------

| Value | Mode |
| --- | --- |
| 0* | Vibrato |
| 1 | Chorus |

Autopan Width (CC91)
--------------------

| Value | Width |
| --- | --- |
| 0 | 0% |
| 32* | 50% |
| 64 | 100% |
| 65 | -99% |
| 96 | -50% |
| 127 | -2% |

Tremolo / Autopan Depth (CC92)
------------------------------

| Value | Depth |
| --- | --- |
| 0 | 0% |
| 32 | 25% |
| 64 | 50% |
| 95 | 75% |
| 127* | 100% |

Vibrato / Chorus Depth (CC93)
-----------------------------

| Value | Depth |
| --- | --- |
| 0 | 0% |
| 32 | 25% |
| 64 | 50% |
| 95 | 75% |
| 127* | 100% |

Rotary Phase (CC94)
-------------------

| Value | Width |
| --- | --- |
| 0* | 0% |
| 32 | 50% |
| 64 | 100% |
| 65 | -99% |
| 96 | -50% |
| 127 | -2% |

Phaser (CC95)
-------------

| Value | Amount |
| --- | --- |
| 0* | 0% |
| 32 | 25% |
| 64 | 50% |
| 95 | 75% |
| 127 | 100% |
