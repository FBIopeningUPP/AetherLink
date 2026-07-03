---
title: AetherLink
author: Milind Singh
description: A compact battery monitoring system designed for UAV, Drone and autonomus robots batteries. It provides realtime battery insights 
created_at: 2026-07-03
---
 ## Day 1
- July 3rd

i have decided to make a battery monitoring system with esp32 c3, it will work as a montoring system mounted on ur jst connectors of your lipo and lion batteries it will track the cycles and everything that is required for optimal battery usage

i will be doing a optimal folder strcuture for all the files and folders for further ease in development

created and updated the config.h with all the required options for free rtos and the main ble and partition and every configuration 

checked all the things using pio run -t compiledb

moved towards a new file that is types h for all the data types used in code
made the whole types.h files with every type of thing there and verified it with command pio run -t compiledb and it works

now we move to the next file which is main.cpp the main file that will initilize the system and run the event loop
completed the main.cpp for proper usage with proper message and everything

and lets move to battery_manager.h for the battery managing logic

i will write the battery moniring interface so, i will declare everything like the reading calclulating and managing battery's data
finished the battery manager h files with all the functions and it compiles smoothely

we will move over to battery manager .cpp so we can impleent the reading calculating and monitroing battery data

took guide of ai for mentorship reagarding this and i was told to not make fake values and do real stuff

soo i am restarting and changing a lot of the stuff in this as we will not measure the amp ratings as they take time and efforts do it so we will remove the amp features and only keep the voltage features, measuring amp required shunt resisotrs and all and they occupy spaces

choose the components for the build we will go with difference ic as it can monitor many cells and voltage and current and all that monitoring so we will go with that we will start working on the hardware definition and the pinout and everything before hand

so what i planned is to start working on the hardware first cause it is easy to work on so i will be making a pinout.md file for better management

i will be wriitng pinout.h so it can help platform io and all the whoel code

now i will start making the pcb design after these updates

woill be shifting to BQ7695202PFBR for better support of 3s-12s batteries and will be re writing the code and all 

## DAY 2 July 4th

will be updating ble_manager.cpp for the new code and structure of the code
eveyrthing is done and firmware is done

will be working on website
writing the whole code  finished the js code 
i wrote the code for css like the basic code what ever will be required, and i will try to take help form ai for furhter html optimization