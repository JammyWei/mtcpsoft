# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com

import logging
import logging.config

LOG_FILE   = 'log.txt'
LOG_LEVEL  = logging.DEBUG
LOG_FORMAT = "%(asctime)s %(levelname)s %(thread)d %(message)s"
'''
logging.basicConfig(filename = LOG_FILE, 
                    level    = LOG_LEVEL, 
                    format   = LOG_FORMAT)

mylogger = logging.getLogger(__name__)
'''
logging.config.fileConfig("logging.conf")

#create logger
mylogger = logging.getLogger("example")
