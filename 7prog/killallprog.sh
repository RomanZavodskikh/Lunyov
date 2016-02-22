#!/bin/bash
kill -9 `ps -ax | grep a.out | grep -v grep`
