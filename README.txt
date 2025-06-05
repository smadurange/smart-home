SMART HOME

This is a collection of projects I'm currently working on to make our home
smarter. The project is in its early stages. This project considers efficiency
and environmental friendliness as part of the problem constraints. That means
'working' solutions aren't enough. They must be efficient.

THE DOOR LOCK

The door lock project aims to drive my door lock using a fingerprint sensor
module (FPM). In its current version, the front of the door has FPM connected
to an ATmega328P microcontroller. The back of the door has a servo connected to
another ATmega328P. Both modules have 2.4GHz NRF24L01+ RFMs connected to them.

When the FPM authenticates a user, the MCU establishes a secure connection with
the back of the door, exchanges a session key, and transmits a lock/unlock 
command to the servo. The wireless protocol allowed me to get it working with
modifying our old Japanese door lock. I wrote all the firmware and drivers of
this project from scratch.

Although I designed this around RFM to learn more about RFM, the architecture
can be made more power-efficient if I toss the RFMs. I'm in the midst of making
these changes to the system.

To build and upload the client and server components, use the following
commands. Change the KEY, SYN, LOCK and UNLOCK definitions for your system.

  make -f Client.Makefile FPM_PWD={your_pwd_for_FPM} upload
  make -f Server.Makefile upload


