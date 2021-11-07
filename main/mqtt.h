#ifndef MQTT_H
#define MQTT_H

void mqtt_start();

void mqtt_envia_mensagem(char * topico, char * mensagem);

void mqtt_topic_subscribe(char * topico);

void mqtt_topic_unsubscribe(char * topico);

#endif