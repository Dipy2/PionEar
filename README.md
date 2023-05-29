# PionEar
PionEar smart sesnor for emergency vehicle alarm detection.

More about the project can be found here:
https://hackaday.io/project/191087-pionear-making-roads-safer-for-deaf-drivers


This code is used to be uploaded into TinyML host MCU (SAMD21) via Arduino IDE
use this manual to read about how to upload custom code:
https://github.com/rdpoor/tinyml_low_power

More general description about other possible ways to write own firmware:
https://discourse.syntiant.com/t/writing-firmware-from-scratch-using-arduino-ide/66

Also use edge impulse manuals about uploading ML models into TinyML:
https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets/syntiant-tinyml-board


General description:

Edge Impulse is a platform that allows to create your own ML models and deploy into supported HW.
For current PionEar project the Edge Impulse model is created by me and available here:
https://studio.edgeimpulse.com/public/145554/latest
This in only part of the code that runs on the neural decision processor NDP100 and is responsible for the emergency vehicle siren recognition.
Follow the edge impulse manual about the way how to upload the model into TinyML

The another part of the firmware is control firmware that is running on the host MCU (SAMD21) that is also present on TinyML board.
Basically it communicaes with NDP100 and read the results from the ML model. Based on this results it can do other opreations like controlling the LED's, GPIO's
etc. The code can be prepared in Arduino IDE as the TinyML is compatible with Arduino MKR zero.
After you upload the ML model you can use arduino IDE to upload FW for the host MCU.

