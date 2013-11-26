#!/bin/bash

for f in *.htm; do rm -rf $f && wget http://wwitv.com/television/$f; done
