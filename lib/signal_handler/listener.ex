defmodule SignalHandler.Listener do
  require Logger
  use Bitwise

  use GenServer

  @on_load :load_nif
  def load_nif do
    nif_path = :filename.join(:code.priv_dir(:signal_handler), 'signal_handler')
    :ok = :erlang.load_nif(nif_path, 0)
  end

  def start_link do
    GenServer.start_link(__MODULE__, [])
  end

  defstruct by_name: nil, by_code: nil, handlers: %{}

  def init(_args) do
    Process.register(self(), :signal_handler)
    __MODULE__.bind(self())

    sig_names_by_num = build_signals_map()
    sig_nums_by_name = sig_names_by_num |> Enum.map(fn {k, v} -> {v, k} end) |> Enum.into(%{})

    Task.start_link(fn ->
      configured_handler_modules =
        Application.get_env(:signal_handler, :modules, [
          SignalHandler.GracefulShutdown
        ])

      configured_handler_modules
      |> Enum.each(fn mod ->
        :signal_handler |> GenServer.call({:install, mod})
      end)
    end)

    {:ok, %__MODULE__{by_code: sig_names_by_num, by_name: sig_nums_by_name}}
  end

  def handle_call(:get_signals, _from, state) do
    erts_signals =
      signal_states()
      |> Enum.map(fn {code, in_state} -> {get_name(state, code), in_state} end)
      |> Enum.into(MapSet.new())

    {:reply, erts_signals, state}
  end

  def handle_call({:get_signals_in_state, in_state}, _from, state) do
    erts_signals =
      signal_states()
      |> Enum.filter(fn {_, state} -> state == in_state end)
      |> Enum.map(&elem(&1, 0))
      |> Enum.map(&get_name(state, &1))
      |> Enum.into(MapSet.new())

    {:reply, erts_signals, state}
  end

  def handle_call({:install, mod}, _from, %__MODULE__{handlers: handlers} = state) do
    signals =
      mod.__info__(:functions)
      |> Enum.filter(fn {_, v} -> v == 0 end)
      |> Enum.map(&elem(&1, 0))
      |> Enum.map(&Atom.to_string/1)
      |> Enum.map(fn
        <<"catch_signal_", sig_name::binary>> -> sig_name
        _ -> nil
      end)
      |> Enum.filter(& &1)
      |> Enum.map(&String.to_atom/1)

    new_registrations =
      signals
      |> Enum.map(fn signal_name ->
        signal_code = state |> get_code(signal_name)
        __MODULE__.register(signal_code)
        {signal_code, normalize_handler(mod, signal_name)}
      end)
      |> Enum.into(%{})

    handlers = Map.merge(handlers, new_registrations)

    {:reply, {:registered, signals}, %{state | handlers: handlers}}
  end

  def handle_call(
        {:register, signal_name, new_handler},
        _from,
        %__MODULE__{handlers: handlers} = state
      ) do
    signal_code = state |> get_code(signal_name)
    handlers = Map.put(handlers, signal_code, normalize_handler(new_handler, signal_name))
    __MODULE__.register(signal_code)

    {:reply, {:registered, signal_name}, %{state | handlers: handlers}}
  end

  def handle_call({:unregister, signal_name}, _from, %__MODULE__{handlers: handlers} = state) do
    signal_code = state |> get_code(signal_name)
    handlers = Map.delete(handlers, signal_code)
    __MODULE__.unregister(signal_code)

    {:reply, {:unregistered, signal_name}, %{state | handlers: handlers}}
  end

  def handle_call({:send_own_node_signal, signal_name}, _from, state) do
    signal_code = state |> get_code(signal_name)
    System.cmd("kill", ["-#{signal_code}", to_string(:os.getpid())])
    {:reply, :ok, state}
  end

  def handle_call(_req, _from, state), do: {:noreply, state}

  def handle_info({:caught, signal_code}, %__MODULE__{handlers: handlers} = state) do
    handler = Map.fetch!(handlers, signal_code)
    handler.()

    {:noreply, state}
  end

  def handle_info(_msg, state), do: {:noreply, state}

  # overridden by NIF
  def bind(_pid), do: exit("not implemented")
  def register(_signal_code), do: exit("not implemented")
  def unregister(_signal_code), do: exit("not implemented")
  def signal_table, do: exit("not implemented")
  def signal_states, do: exit("not implemented")
  def signal_code_limits, do: exit("not implemented")

  defp build_signals_map do
    {signum_bound, sigrtmin, sigrtmax} = signal_code_limits()

    named_signals =
      signal_table()
      |> Enum.map(fn {signum, name} ->
        normalized_name = name |> Atom.to_string() |> String.downcase() |> String.to_atom()
        {signum, normalized_name}
      end)
      |> Enum.into(%{})

    rt_signals =
      case sigrtmax do
        -1 ->
          []

        _ ->
          sigrtmin..sigrtmax
          |> Enum.map(fn signum ->
            {signum, realtime_signal_name(signum, sigrtmin, sigrtmax)}
          end)
      end
      |> Enum.into(%{})

    unknown_signals =
      0..signum_bound
      |> Enum.map(fn i ->
        {i, i}
      end)
      |> Enum.into(%{})

    unknown_signals |> Map.merge(rt_signals) |> Map.merge(named_signals)
  end

  defp normalize_handler(fun, _) when is_function(fun), do: fun

  defp normalize_handler(mod, sig_name) when is_atom(mod),
    do: fn ->
      fn_name = String.to_atom("catch_signal_#{sig_name}")
      :erlang.apply(mod, fn_name, [])
    end

  defp realtime_signal_name(signum, sigrtmin, sigrtmax) do
    {d_up, d_down} = {signum - sigrtmin, sigrtmax - signum}

    case {d_up, d_down, d_up - d_down} do
      {0, _, _} -> :rtmin
      {_, 0, _} -> :rtmax
      {d_up, _, sign} when sign <= 0 -> String.to_atom("rtmin+" <> to_string(d_up))
      {_, d_down, sign} when sign > 0 -> String.to_atom("rtmax-" <> to_string(d_down))
    end
  end

  defp get_code(_, code) when is_integer(code), do: code

  defp get_code(%__MODULE__{by_name: by_name}, name) when is_atom(name),
    do: Map.fetch!(by_name, name)

  defp get_name(%__MODULE__{by_code: by_code}, code) when is_integer(code),
    do: Map.fetch!(by_code, code)
end
