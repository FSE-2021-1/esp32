set(COMPONENT_SRCS "http_client.c" "wifi.c" "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(REQUIRES "esp32-DHT11")

register_component()