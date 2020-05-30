#!/bin/sh
echo ${PATH}
ls -al
curl http://taruo.net/e/ | nkf -w
autoreconf -vfi
