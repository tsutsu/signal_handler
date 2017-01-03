# signal_handler

`System.SignalHandler` is an Elixir application containing a NIF module. This NIF can register itself as the recipient of arbitrary POSIX signals directed at the Erlang node, and will then hand them off as messages to Erlang-land.

By default, no signals are listened for. For now, you must register any signals you want to respond to at runtime.

**Important note 1:** `System.SignalHandler` runs in the context of an ERTS scheduler thread, so unexpected behavior could occur if you were able to override signal registrations used by ERTS. To avoid this, currently `System.SignalHandler` only allows registration within a specific set of well-known signals, all of which are left to their default implementations by ERTS-8.2.

**Important note 2:** While POSIX signal *names* are standardized, signal *numbers* are OS-dependent. `System.SignalHandler` currently maps the POSIX signal names you provide to Linux signal numbers, and as such, will only work if used on Linux. This will be fixed!

## Usage

Registering a "graceful shutdown" signal:

```elixir
System.SignalHandler.register :term, &:init.stop/0
```

Registering an "async info" ^T signal, as seen in `dd(1)`:

```elixir
System.SignalHandler.register :info, fn -> Logger.debug("some info") end
```

Unregistering a signal:

```elixir
System.SignalHandler.unregister :term
```

## Signals

Signals that can currently be used:

* `:hup`
* `:term`
* `:winch`
* `:info`
* `:usr`
* `:usr2`

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

