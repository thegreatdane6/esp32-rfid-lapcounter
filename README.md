# RFID based DIY lap counter for ESP32

Small cars need smaller transponders.

This project is made to count laps and run races with the e.g. 1:76 scale rc cars from aliexpress. As RC522 or PN532 modules are highly available and cheap enough to fool around with they were a good choice. Tiny transponders are available to stick onto the cars all of whats missing is a small customization regarding the RFID modules antenna and a little software to make it happen.

- The [transponders](https://www.amazon.de/gp/product/B01CJYUSG0) which were chosen in this project's setup (13,56 MHz).
- RFID read module
- A strip addressable LED's (e.g. WS2812b)
- A buzzer / speaker
- A button

You will now need to choose a module to read the transponder. This project is using a RC522 for now because it was available right away. As the tracks width is about 15cm and the chips antenna is only about ~4cm, multiple chips will need to be mounted next to each other. This is still an ongoing process as there are still experiments taking place on how to mount them best to get most reliable readings. A .stl file to print a the bridge covering the chips, the LED's and some more stuff will get published so you can build one yourself.

<img alt="racetrack" src="documentation/media/track_to_race.jpg" width="400" />
<img alt="racetrack_gif" src="documentation/media/track_to_race.gif" width="400" />
