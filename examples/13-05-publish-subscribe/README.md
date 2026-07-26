# Publish/Subscribe

A publisher allocates one dynamic telemetry event and broadcasts it to two
subscribers. haievent retains one reference per successful subscriber and
returns the block to the pool only after both Active Objects finish dispatch.

```bash
make EXAMPLE=13-05-publish-subscribe run
```
