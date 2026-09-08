`kafka()` source: fixed a crash on an empty `topic()`, an inverted `kafka-logging()` value check, leaks of the
`rd_kafka_conf_t` and of the topic-partition list on error paths, a silently ignored persist-state init failure and
a NULL dereference when the state is updated before the client exists.
