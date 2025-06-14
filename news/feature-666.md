`clickhouse()` destination: Added `server-side-schema()` option.

Sending data to ClickHouse in Protobuf format has some limitations.
The Protobuf formatted data does not carry type information,
so there are two ways of determining the type mapping in ClickHouse:

1. Using autogenerated schema: https://clickhouse.com/docs/interfaces/formats/Protobuf#using-autogenerated-protobuf-schema

   This method looks at the table that the data is getting inserted
   into and generates the default protobuf mapping for it.

   The problem with this solution is that some more complex column
   types can receive data from different input types, for example
   DateTime64 can recieve from uint64, string, float, but with the
   autogenerated schema you can utilize only one of them.

2. Using format schema: https://clickhouse.com/docs/interfaces/formats#formatschema

   This method solves the issue of the autogenerated schema, but
   needs a more complex setup. First you need to place a .proto
   file along your server in a specific directory, then you can
   reference that schema during insert, which is done by the
   `server-side-schema()` option.

Example: `server-side-schema("my_proto_file_on_server:my_message_schema_name")`
