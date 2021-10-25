defmodule SignalHandler.GracefulShutdown do
  require SignalHandler

  SignalHandler.handle :term do
    :init.stop()
  end
end
