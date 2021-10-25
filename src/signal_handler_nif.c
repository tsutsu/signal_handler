#include <signal.h>
#include <stdio.h>
#include <erl_nif.h>
#include "sig2str.h"

ErlNifPid controller_pid;

void
notify_controller(int signum) {
  ErlNifEnv *msg_env = enif_alloc_env();
  ERL_NIF_TERM term_signum = enif_make_uint(msg_env, signum);
  ERL_NIF_TERM atom_caught = enif_make_atom(msg_env, "caught");
  ERL_NIF_TERM msg = enif_make_tuple2(msg_env, atom_caught, term_signum);

  enif_send(NULL, &controller_pid, msg_env, msg);
}

const char* handler_state_descriptions[] = {
  "default",
  "ignore",
  "erts",
  "registered"
};

int
handler_state(int signum) {
    struct sigaction old_action;

    sigaction(signum, NULL, &old_action);
    if (old_action.sa_handler == SIG_DFL) return 0;
    if (old_action.sa_handler == SIG_IGN) return 1;
    if (old_action.sa_handler == notify_controller) return 3;
    return 2;
}

void
install_handler(int signum) {
    struct sigaction new_action;

    new_action.sa_handler = notify_controller;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(signum, &new_action, NULL);
}

void
remove_handler(int signum) {
    struct sigaction new_action;

    new_action.sa_handler = SIG_DFL;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(signum, &new_action, NULL);
}

static ERL_NIF_TERM
bind_controller_pid(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 1)
    return enif_make_badarg(env);

  if(!enif_get_local_pid(env, argv[0], &controller_pid))
    return enif_make_badarg(env);

  return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM
register_signal(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 1)
    return enif_make_badarg(env);

  unsigned int signum;
  if(!enif_get_uint(env, argv[0], &signum))
    return enif_make_badarg(env);

  install_handler(signum);

  return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM
unregister_signal(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 1)
    return enif_make_badarg(env);

  unsigned int signum;
  if(!enif_get_uint(env, argv[0], &signum))
    return enif_make_badarg(env);

  remove_handler(signum);

  return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM
get_signal_table(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 0)
    return enif_make_badarg(env);

  ERL_NIF_TERM list = enif_make_list(env, 0);
  struct numname* entry;

  for(int i = 0; i < NUMNAME_ENTRIES; i++) {
    entry = (numname_table + i);

    list = enif_make_list_cell(env,
      enif_make_tuple2(env,
        enif_make_int(env, entry->num),
        enif_make_atom(env, entry->name)
      ),
      list
    );
  }

  enif_make_reverse_list(env, list, &list);
  return list;
}


static ERL_NIF_TERM
get_signal_states(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 0)
    return enif_make_badarg(env);

  ERL_NIF_TERM list = enif_make_list(env, 0);

  for(int i = 0; i < SIGNUM_BOUND; i++) {
    list = enif_make_list_cell(env,
      enif_make_tuple2(env,
        enif_make_int(env, i),
        enif_make_atom(env, handler_state_descriptions[handler_state(i)])
      ),
      list
    );
  }

  enif_make_reverse_list(env, list, &list);
  return list;
}

static ERL_NIF_TERM
get_signal_code_limits(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 0)
    return enif_make_badarg(env);

  return enif_make_tuple3(env,
    enif_make_int(env, SIGNUM_BOUND),
    enif_make_int(env, SIGRTMIN),
    enif_make_int(env, SIGRTMAX)
  );
}

static ErlNifFunc nif_funcs[] = {
  {"bind", 1, bind_controller_pid},
  {"signal_code_limits", 0, get_signal_code_limits},
  {"signal_table", 0, get_signal_table},
  {"signal_states", 0, get_signal_states},
  {"register", 1, register_signal},
  {"unregister", 1, unregister_signal}
};

ERL_NIF_INIT(Elixir.SignalHandler.Listener, nif_funcs, NULL, NULL, NULL, NULL)
