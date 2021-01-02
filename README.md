# RFID based DIY lap counter for ESP32

Small cars need smaller transponders.

This project is made to count laps and run races with the e.g. 1:76 scale rc cars from aliexpress. As RC522 or PN532 modules are highly available and cheap enough to fool around with they were a good choice. Tiny transponders are available to stick onto the cars all of whats missing is a small customization regarding the RFID modules antenna and a little software to make it happen.

The [transponders](https://www.amazon.de/gp/product/B01CJYUSG0) which i chose (13,56 MHz).

You will now need to choose a module to read the transponder. I've chosen the RC522 just because i had it laying around.

<img alt="racetrack" src="documentation/media/track_to_race.jpg" width="400" />

<video controls="controls" width="400">
  <source type="video/mp4" src="/documentation/media/video_driving_moms_car.mp4"></source>
</video>
