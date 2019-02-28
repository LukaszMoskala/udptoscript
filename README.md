# udptoscript
This program receives UDP packets, checks them, and executes script

# Dependencies
Program has no external dependencies

# License
Program is distributed under GPL license, see `LICENSE` file for more details

# Status
Software is in early alpha, might have security issues. If you need high security, go with SSH instead.

# Reloading config
To reload config file on-the-go, send `SIGUSR1`.

You can use `killall -USR1 udptoscript`

Or, if you know PID, `kill -USR1 $PID`

# PID file
Program can save it's PID to file. This file will be removed if program is
able to exit gracefully (that is, the kernel will not kill it).

To enable, set `pidfile /path/to/pid/file` in config. Path can be relative.

If this file exist when program is started, you'll see error message and program
will refuse to work. If that is problem for you, don't specify this setting in
your config file ( or modify source code ;) )

If you reload config with `SIGUSR1`, new pidfile will NOT be generated, and
program will continue to use the old one
