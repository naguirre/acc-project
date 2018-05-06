Contains code for the main board of the Robot
Robot is based on esp32 microcontroller

To build robot software you need ESP_IDF sdk. you can find informations about how to install it here :
http://esp-idf.readthedocs.io/en/latest/

After installing ESP_IDF and gcc crosscompiler for xtensa you can export ESP_IDF environnement variable and add xtensa gcc into your PATH.
For example :

```
export ESP_IDF=${HOME}/Dev/esp32/esp-idf
export PATH=$PATH:${HOME}/Dev/esp32/xtensa-esp32-elf/bin
```

Robot software depends on [odometry](https://github.com/naguirre/robot_sw) project, and uses it as a submodule. You need to update submodule of the project :
```
git submodule init
git submodule update
```

You can now compile the code
```
make
```
And flash the code on the ESP32 module :
```
make flash
```
