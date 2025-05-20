idf_component_register(SRCS "main.c"
                            "src/bme_task.c"
                            "src/sdcard_task.c"
                            "src/accelGY_task.c"
                            "src/accel_task.c"
                            "src/gps_task.c"
                            "src/ble_task.c"
                    INCLUDE_DIRS "." "inc")
