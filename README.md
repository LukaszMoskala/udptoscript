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
