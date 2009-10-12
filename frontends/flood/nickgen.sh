#!/bin/bash
#cat nickname
echo -n `head -c 8 /dev/urandom|base64|sed 's/[//+0-9]//g; s/=//;'|tr A-Z a-z`
