# AMTraC - Ableton Midi fooT Controller
MIDI foot controller for Ableton

Use in combination with [AMTraC-Info](https://github.com/LeovR/amtrac-info)

## Introduction
For a band project of mine we wanted to extend our sound with samples. Therefore, I needed the possibility to start songs by foot. In addition, I wanted to have the opportunity to stop songs, repeat a part of a song, check the tempo, and see which part the song is currently in.

To add samples to your sound, in a live setup, the software which usually is used is [Ableton Live](http://www.ableton.com).
Ableton Live let's you trigger a lot of things via MIDI, which covers a lot of things I wanted to have.
There are a lot of MIDI foot controllers out there which could be used for such a setup. But none of these covered all my needs for such a controller. Thus, I decided to build my own MIDI controller.

## Hardware

### Controller
For the controller I decided to use the [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) controller which is Arduino compatible, has 34 Digital I/O pins and natively supports MIDI. Thus, it is quite easy to use MIDI.

### Case
For the controller I wanted to have 2 have an LCD, to display the current song / part, 1 stop-Button, 1 play/repeat-Button, 2 bank-switch-buttons and 6 buttons to select a song. Each button should have a corresponding LED and I wanted to have an additional LED which corresponds to the click of the song.
Thus, in total I needed a quite large case to fit everything on it.

After some googling I decided to use the **Hammond 1441-14** case.

I used simple LEDs and foot buttons.

## LCD
As the LCD I used a **Blue IIC 16x2 LCD** which should have enough space to display the required information.
