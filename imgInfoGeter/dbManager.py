# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com
import logging


try:
    import pyodbc
except:
    print('no module pyodbc, should init first!')

logging.config.fileConfig("logging.conf")

#create logger
logger = logging.getLogger("example")

DB_HOST = '10.20.1.200' # '210.73.152.201'
USER = 'sa'
PWD = 'sa'
DATABASE = 'CDMTCP'

def get_db_connect():
    try:
        db_conn = pyodbc.connect('DRIVER={SQL Server}', host = DB_HOST, user = USER, password = PWD, database = DATABASE)
    except: # not print db execption yet
        logger.debug('init db got an error!')
        print('init db got an error!')
    return db_conn

def close_db_connect(db_conn):
    if db_conn:
        db_conn.close()

'''

DB_CONNECT_LOCK = threading.Lock()

MAX_DB_CONNECT = 10
DB_CONNECT_NOT_USE_LIST = list()
DB_CONNECT_USED_LIST = list()

def init_db_connect_list():
    for i in range(MAX_DB_CONNECT):
        try:
            db_conn = pyodbc.connect('DRIVER={SQL Server}', host = DB_HOST, user = USER, password = PWD, database = DATABASE)
            # lock
            DB_CONNECT_LOCK.acquire()
            DB_CONNECT_NOT_USE_LIST.append(db_conn)
            # release lock
            DB_CONNECT_LOCK.release()
        except: # not print db execption yet
            logger.debug('init db got an error!')
            print('init db got an error!')
            break
    print('init db conn done!')
    logger.debug('init db conn done!')
    return

def get_one_db_connect():
    db_conn = None
    try:
        # lock
        DB_CONNECT_LOCK.acquire()
        db_conn = DB_CONNECT_NOT_USE_LIST.pop()
        DB_CONNECT_USED_LIST.append(db_conn)
        # release lock
        DB_CONNECT_LOCK.release()
        
    except ValueError:
        print('not enough db_conn!')
        logger.debug('not enough db_conn!')
    except Exception as e:
        print(e)
        logger.debug(e)
    return db_conn

def close_one_db_connect(conn):
    if conn:
        # if the conn is usable
        try:
            conn.commit()
        except : # not print db except  yet
            print('connect can not used')
            # lock
            DB_CONNECT_LOCK.acquire()
            DB_CONNECT_USED_LIST.remove(conn)
            # release lock
            DB_CONNECT_LOCK.release()
            return
        
        try:
            # lock
            DB_CONNECT_LOCK.acquire()
            DB_CONNECT_USED_LIST.remove(conn)
            DB_CONNECT_NOT_USE_LIST.append(conn)
            # release lock
            DB_CONNECT_LOCK.release()
        except Exception as e:
            print(e)
            logger.debug(e)
    return
'''