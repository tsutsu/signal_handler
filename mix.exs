defmodule Mix.Tasks.Compile.SignalHandlerMakefile do
  @shortdoc "Compiles the SignalHandler NIF"
  use Mix.Task

  @include_path Path.join([:code.root_dir(), ["erts-", :erlang.system_info(:version)], "include"])

  def run(_) do
    if Mix.env() == :test, do: File.rm_rf("priv")

    unless File.regular?("priv/signal_handler.so") do
      File.mkdir("priv")

      {result, error_code} =
        System.cmd("make", ["priv/signal_handler.so"],
          stderr_to_stdout: true,
          env: [{"ERTS_INCLUDE_PATH", @include_path}]
        )

      IO.binwrite(result)

      if error_code != 0 do
        raise Mix.Error,
          message: """
            Could not run `make`.
            Please check if `make` and either `clang` or `gcc` are installed
          """
      end
    end

    Mix.Project.build_structure()
    :ok
  end
end

defmodule System.SignalHandler.Mixfile do
  use Mix.Project

  def project do
    [
      app: :signal_handler,
      version: "0.1.1",
      elixir: "~> 1.12",
      build_embedded: Mix.env() == :prod,
      start_permanent: Mix.env() == :prod,
      compilers: [:signal_handler_makefile, :elixir, :app],
      deps: deps()
    ]
  end

  def application do
    [
      mod: {System.SignalHandler, []},
      applications: [:logger]
    ]
  end

  defp deps, do: []
end
