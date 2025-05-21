idf_component_register(SRCS "gap.c"
                            "gatt_svc.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash data_queues)
