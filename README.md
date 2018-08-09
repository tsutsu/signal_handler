# signal_handler

`System.SignalHandler` is an Elixir application containing a NIF module. This NIF can register itself as the recipient of arbitrary POSIX signals directed at the Erlang node, and will then hand them off as messages to Erlang-land.

**Important note:** `System.SignalHandler` runs in the context of an ERTS scheduler thread, so unexpected behavior could occur if you were able to override signal registrations used by ERTS. You can see which signals these are using `System.SignalHandler.erts_reserved_signals/0`. It is not recommended to override these signals, but `System.SignalHandler` does nothing to stop you from doing so.

## Configuration

In your `config.exs`:

```elixir
config :signal_handler, :modules, [YourHandler]
```

Then, in `your_handler.ex`:

```elixir
defmodule YourHandler do
  use System.SignalHandler

  handle :winch do
    # oh hey the terminal size changed
  end
end
```

By default, `modules` is `[System.SignalHandler.GracefulShutdown]`, which simply maps `SIGTERM` to `:init.stop()`.

## Usage

Installing a predefined signal-handler module at runtime:

```elixir
System.SignalHandler.install(YourHandler)
```

Registering a one-off signal handler at runtime:

```elixir
System.SignalHandler.register(:term, &:init.stop/0)
```

Unregistering a signal:

```elixir
System.SignalHandler.unregister(:term)
```

Getting the state of all known signals:

```elixir
System.SignalHandler.signals()
```

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed as:

  1. Add `signal_handler` to your list of dependencies in `mix.exs`:

  ```elixir
def deps do
  [{:signal_handler, "~> 0.1.0"}]
end
  ```

  2. Ensure `signal_handler` is started before your application:

  ```elixir
def application do
  [applications: [:signal_handler]]
end
  ```

