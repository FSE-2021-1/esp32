set(COMPONENT_SRCS "mqtt_router.c" "register.c" "mqtt.c" "http_client.c" "wifi.c" "main.c" "global.c" "data.c" "pwm.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(REQUIRES "esp32-DHT11")

register_component()