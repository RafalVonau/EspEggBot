#!/bin/bash

scp ./*.py pi3:/var/www/html/eggbot/
scp ./index.html pi3:/var/www/html/eggbot/
scp ./test.svg pi3:/var/www/html/eggbot/