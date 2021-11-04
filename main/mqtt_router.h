#ifndef __MQTT_ROUTER_H__
#define __MQTT_ROUTER_H__

void mqtt_router_route(int topic_len, char *topic, int payload_len, char *payload);

#endif