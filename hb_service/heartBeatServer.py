# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com

import socket
import threading
import traceback

# self module
import processData
import dbManager
import myLog

# Global Variables
HOST = socket.gethostbyname(socket.gethostname()) #socket.INADDR_ANY
SERVER_PORT = 44444

client_list = list()
list_lock = threading.Lock()

def insert_list(id):
    list_lock.acquire()
    client_list.append(id)
    list_lock.release()
    return

def remove_list(id):
    list_lock.acquire()
    client_list.remove(id)
    list_lock.release()
    return

def total_in_list():
    list_lock.acquire()
    total = len(client_list)
    list_lock.release()
    return total

def do_init():
    dbconn = None
    cur = None
    
    dbconn = dbManager.get_one_db_connect()

    if dbconn:
        cur = dbconn.cursor()
    
    insert_list(threading.get_ident())
    
    return dbconn, cur

def do_finish(dbconn, cur, sock):
    
    if dbconn:
        cur.close()
    
    dbManager.close_one_db_connect(dbconn)
    
    remove_list(threading.get_ident())
    
    # close socket
    sock.close()
    
    return

def handleConnect(sock, addr):
    
    dbconn, cur = do_init()
    
    
    while True:
        try:
            b_data = sock.recv(1024)
            if b_data == b'':
                print('reomte closed!')
                sock.close()
                break
            
            #print(len(b_data), b_data)
            myLog.mylogger.debug('recv %s %s %s',addr[0], addr[1], str(b_data))
            
            #��process data
            r_data  = processData.process_data(b_data, dbconn, cur)
            
            sock.send(r_data)
        except:
            print(traceback.format_exc())
            myLog.mylogger.debug(traceback.format_exc())
            break
    
    do_finish(dbconn, cur, sock)
    print(total_in_list())
    return

def Server():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(HOST)
        sock.bind((HOST, SERVER_PORT))
        sock.listen(5)
        print('main server ready to accept...')
        while True:
            conn, addr = sock.accept()
            print('connected by ', addr)
            try:
                t = threading.Thread(target=handleConnect, args=(conn, addr))
                t.start()
            except RuntimeError as e:
                print(e)
                myLog.mylogger.debug(e)
    except:
        print(traceback.format_exc())
        myLog.mylogger.debug(traceback.format_exc())


if __name__=='__main__':
    import os
    print(__file__, 'test')
    os.system('pause')
    