set(COMPONENT_SRCS "gps.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES driver esp_event data_queues)

register_component()
