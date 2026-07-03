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
