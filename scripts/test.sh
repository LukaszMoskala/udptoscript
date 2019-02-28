#!/bin/sh

echo I output to STDOUT so that ill be sent back to client

>&2 echo I output to STDERR so that ill be logged into console
