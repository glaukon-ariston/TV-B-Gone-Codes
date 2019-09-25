# TV-B-Gone-Codes
Converts [TV-B-Gone](https://github.com/shirriff/Arduino-TV-B-Gone) infrared codes for switching TVs off into a format suitable for the [TVKILL](https://github.com/42SK/TVKILL) Android app.

[TV-B-Gone](https://github.com/shirriff/Arduino-TV-B-Gone) infrared code format is explained on the Adafruit's learning [website](https://learn.adafruit.com/tv-b-gone-kit/design-notes). It gives you a frequency and on-off (mark-silence) pairs expressed in (micro-)seconds. TVKILL needs the pairs in the multiple of the period `(1/freq)`.

 
