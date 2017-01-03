/* sig2str.h -- convert between signal names and numbers

   Copyright (C) 2002, 2005, 2009-2012 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert.  */

#include <signal.h>

/* Don't override system declarations of SIG2STR_MAX, sig2str, str2sig.  */
#ifndef SIG2STR_MAX

#define INT_BITS_STRLEN_BOUND(b) (((b) * 146 + 484) / 485)
#define INT_STRLEN_BOUND(t) (INT_BITS_STRLEN_BOUND (sizeof (t) * 8))

/* Size of a buffer needed to hold a signal name like "HUP".  */
# define SIG2STR_MAX (sizeof "SIGRTMAX" + INT_STRLEN_BOUND (int) - 1)

int sig2str (int, char *);
int str2sig (char const *, int *);

#endif

/* An upper bound on signal numbers allowed by the system.  */

#if defined _sys_nsig
# define SIGNUM_BOUND (_sys_nsig - 1)
#elif defined NSIG
# define SIGNUM_BOUND (NSIG - 1)
#elif __APPLE__
# define SIGNUM_BOUND 32
#else
# define SIGNUM_BOUND 64
#endif

#ifndef SIGRTMIN
# define SIGRTMIN 0
# undef SIGRTMAX
#endif
#ifndef SIGRTMAX
# define SIGRTMAX (SIGRTMIN - 1)
#endif

#define NUMNAME(name) { SIG##name, #name }

/* Signal names and numbers.  Put the preferred name first.  */
static struct numname { int num; char const name[8]; } numname_table[] =
  {
    /* Signals required by POSIX 1003.1-2001 base, listed in
       traditional numeric order where possible.  */
#ifdef SIGHUP
    NUMNAME (HUP),
#endif
#ifdef SIGINT
    NUMNAME (INT),
#endif
#ifdef SIGQUIT
    NUMNAME (QUIT),
#endif
#ifdef SIGILL
    NUMNAME (ILL),
#endif
#ifdef SIGTRAP
    NUMNAME (TRAP),
#endif
#ifdef SIGABRT
    NUMNAME (ABRT),
#endif
#ifdef SIGFPE
    NUMNAME (FPE),
#endif
#ifdef SIGKILL
    NUMNAME (KILL),
#endif
#ifdef SIGSEGV
    NUMNAME (SEGV),
#endif
    /* On Haiku, SIGSEGV == SIGBUS, but we prefer SIGSEGV to match
       strsignal.c output, so SIGBUS must be listed second.  */
#ifdef SIGBUS
    NUMNAME (BUS),
#endif
#ifdef SIGPIPE
    NUMNAME (PIPE),
#endif
#ifdef SIGALRM
    NUMNAME (ALRM),
#endif
#ifdef SIGTERM
    NUMNAME (TERM),
#endif
#ifdef SIGUSR1
    NUMNAME (USR1),
#endif
#ifdef SIGUSR2
    NUMNAME (USR2),
#endif
#ifdef SIGCHLD
    NUMNAME (CHLD),
#endif
#ifdef SIGURG
    NUMNAME (URG),
#endif
#ifdef SIGSTOP
    NUMNAME (STOP),
#endif
#ifdef SIGTSTP
    NUMNAME (TSTP),
#endif
#ifdef SIGCONT
    NUMNAME (CONT),
#endif
#ifdef SIGTTIN
    NUMNAME (TTIN),
#endif
#ifdef SIGTTOU
    NUMNAME (TTOU),
#endif

    /* Signals required by POSIX 1003.1-2001 with the XSI extension.  */
#ifdef SIGSYS
    NUMNAME (SYS),
#endif
#ifdef SIGPOLL
    NUMNAME (POLL),
#endif
#ifdef SIGVTALRM
    NUMNAME (VTALRM),
#endif
#ifdef SIGPROF
    NUMNAME (PROF),
#endif
#ifdef SIGXCPU
    NUMNAME (XCPU),
#endif
#ifdef SIGXFSZ
    NUMNAME (XFSZ),
#endif

    /* Unix Version 7.  */
#ifdef SIGIOT
    NUMNAME (IOT),      /* Older name for ABRT.  */
#endif
#ifdef SIGEMT
    NUMNAME (EMT),
#endif

    /* USG Unix.  */
#ifdef SIGPHONE
    NUMNAME (PHONE),
#endif
#ifdef SIGWIND
    NUMNAME (WIND),
#endif

    /* Unix System V.  */
#ifdef SIGCLD
    NUMNAME (CLD),
#endif
#ifdef SIGPWR
    NUMNAME (PWR),
#endif

    /* GNU/Linux 2.2 and Solaris 8.  */
#ifdef SIGCANCEL
    NUMNAME (CANCEL),
#endif
#ifdef SIGLWP
    NUMNAME (LWP),
#endif
#ifdef SIGWAITING
    NUMNAME (WAITING),
#endif
#ifdef SIGFREEZE
    NUMNAME (FREEZE),
#endif
#ifdef SIGTHAW
    NUMNAME (THAW),
#endif
#ifdef SIGLOST
    NUMNAME (LOST),
#endif
#ifdef SIGWINCH
    NUMNAME (WINCH),
#endif

    /* GNU/Linux 2.2.  */
#ifdef SIGINFO
    NUMNAME (INFO),
#endif
#ifdef SIGIO
    NUMNAME (IO),
#endif
#ifdef SIGSTKFLT
    NUMNAME (STKFLT),
#endif

    /* AIX 5L.  */
#ifdef SIGDANGER
    NUMNAME (DANGER),
#endif
#ifdef SIGGRANT
    NUMNAME (GRANT),
#endif
#ifdef SIGMIGRATE
    NUMNAME (MIGRATE),
#endif
#ifdef SIGMSG
    NUMNAME (MSG),
#endif
#ifdef SIGPRE
    NUMNAME (PRE),
#endif
#ifdef SIGRETRACT
    NUMNAME (RETRACT),
#endif
#ifdef SIGSAK
    NUMNAME (SAK),
#endif
#ifdef SIGSOUND
    NUMNAME (SOUND),
#endif

    /* Older AIX versions.  */
#ifdef SIGALRM1
    NUMNAME (ALRM1),    /* unknown; taken from Bash 2.05 */
#endif
#ifdef SIGKAP
    NUMNAME (KAP),      /* Older name for SIGGRANT.  */
#endif
#ifdef SIGVIRT
    NUMNAME (VIRT),     /* unknown; taken from Bash 2.05 */
#endif
#ifdef SIGWINDOW
    NUMNAME (WINDOW),   /* Older name for SIGWINCH.  */
#endif

    /* BeOS */
#ifdef SIGKILLTHR
    NUMNAME (KILLTHR),
#endif

    /* Older HP-UX versions.  */
#ifdef SIGDIL
    NUMNAME (DIL),
#endif

    /* Korn shell and Bash, of uncertain vintage.  */
    { 0, "EXIT" }
  };

#define NUMNAME_ENTRIES (sizeof numname_table / sizeof numname_table[0])

