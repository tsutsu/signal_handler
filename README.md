# signal_handler

`SignalHandler` is an Elixir application containing a NIF module. This NIF can register itself as the recipient of arbitrary POSIX signals directed at the Erlang node, and will then hand them off as messages to Erlang-land.

**Important note:** `SignalHandler` runs in the context of an ERTS scheduler thread, so unexpected behavior could occur if you were able to override signal registrations used by ERTS. You can see which signals these are using `SignalHandler.erts_reserved_signals/0`. It is not recommended to override these signals, but `SignalHandler` does nothing to stop you from doing so.

## Configuration

In your `config.exs`:

```elixir
config :signal_handler, :modules, [YourHandler]
```

Then, in `your_handler.ex`:

```elixir
defmodule YourHandler do
  use SignalHandler

  handle :winch do
    # oh hey the terminal size changed
  end
end
```

By default, `modules` is `[SignalHandler.GracefulShutdown]`, which simply maps `SIGTERM` to `:init.stop()`.

## Usage

Installing a predefined signal-handler module at runtime:

```elixir
SignalHandler.install(YourHandler)
```

Registering a one-off signal handler at runtime:

```elixir
SignalHandler.register(:term, &:init.stop/0)
```

Unregistering a signal:

```elixir
SignalHandler.unregister(:term)
```

Getting the state of all known signals:

```elixir
SignalHandler.signals()
```
