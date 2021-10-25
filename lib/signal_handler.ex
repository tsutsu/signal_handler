defmodule SignalHandler do
  require Logger

  use Application

  def start(_type, _args) do
    import Supervisor.Spec, warn: false

    children = [
      SignalHandler.Listener
    ]

    opts = [strategy: :one_for_one, name: SignalHandler.Supervisor]
    Supervisor.start_link(children, opts)
  end

  def register(signal_name, fun_spec) do
    :signal_handler |> GenServer.call({:register, signal_name, fun_spec})
  end

  def install(mod) when is_atom(mod) do
    :signal_handler |> GenServer.call({:install, mod})
  end

  def unregister(signal_name) do
    :signal_handler |> GenServer.call({:unregister, signal_name})
  end

  def erts_reserved_signals do
    :signal_handler |> GenServer.call({:get_signals_in_state, :erts})
  end

  def registered_signals do
    :signal_handler |> GenServer.call({:get_signals_in_state, :registered})
  end

  def signals do
    :signal_handler |> GenServer.call(:get_signals)
  end

  def send_self(signame) do
    :signal_handler |> GenServer.call({:send_own_node_signal, signame})
  end

  defmacro __using__(_opts) do
    quote do
      import SignalHandler, only: [handle: 2]
    end
  end

  defmacro handle(signal_name, do: block) do
    handler_name = String.to_atom("catch_signal_#{signal_name}")

    quote do
      def unquote(handler_name)(), do: unquote(block)
    end
  end
end
