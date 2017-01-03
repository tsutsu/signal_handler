defmodule System.SignalHandler.GracefulShutdown do
  use System.SignalHandler

  handle :term do
    :init.stop
  end
end
