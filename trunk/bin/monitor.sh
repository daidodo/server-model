#!/bin/bash

cd /usr/local/sandai/newEmuleTracker/bin

BIN=Server.out
PWD=`pwd`

#email
SEND_MAIL="/usr/local/bin/sendEmail"
MAIL_SITE="mail.sandai.net"
MAIL_FROM="monitor@xunlei.com"
MAIL_TO="daizhao@xunlei.com"
MAIL_TITLE="Server Model restart"
MAIL_MSG="Server Model restart"

THREAD_NUM=`ps axm | grep ${BIN} | grep -v grep | grep -v monitor.sh | wc -l`
echo `date +"%Y-%m-%d %H:%M:%S"` "thread_num:" ${THREAD_NUM}
if test "${THREAD_NUM}" -eq 0
then
    sleep 2
    ${PWD}/start.sh
    echo `date +"%Y-%m-%d %H:%M:%S"` restarted >> ${PWD}/restart.txt
DD=`date +"%Y-%m-%d %H:%M:%S"`
${SEND_MAIL} -s ${MAIL_SITE}  -f ${MAIL_FROM} -t ${MAIL_TO} -xu monitor@xunlei.com -xp 121212 -u "${MAIL_TITLE}" -m "${MAIL_MSG} at ${DD}"
fi


