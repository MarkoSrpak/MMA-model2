idf_component_register(SRCS "main.c"
                            "src/bme_task.c"
                            "src/sdcard_task.c"
                            "src/accelGY_task.c"
                            "src/accel_task.c"
                            "src/gps_task.c"
                            "src/ble_task.c"
                            "src/sweat_task.c"
                    INCLUDE_DIRS "." "inc"
                    REQUIRES accel
                             accelGY
                             adc_drv
                             ble_com
                             bme68x
                             gps
                             i2c
                             led_rgb
                             led_strip_encoder
                             microphone
                             pwm
                             sdspi
                             sweat_sensor
                             data_queues
                             )
