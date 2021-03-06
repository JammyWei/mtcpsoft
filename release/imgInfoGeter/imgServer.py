# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com
import os
import sys
import configparser

import fileWatcher
import InfoProcess
import dbManager

DIRECTORY_PATH = ''

def do_process_file(fn):
    infos = do_get_file_infos(fn)
    do_store_db(infos)
    return

def do_get_file_infos(fn):
    infos = InfoProcess.do_get_file_infos(fn)
    return infos

def do_store_db(infos):
    dbManager.store_pic_infos(infos)
    return


def handle_change(changes):
    for action, file in changes:
        parent_path = os.path.dirname(os.path.abspath(file))
        if action == 1 and 'jpg' in file and parent_path == DIRECTORY_PATH:
            print(file)
            do_process_file(file)

def readConfig():
    global DIRECTORY_PATH
    
    cf = configparser.ConfigParser()
    cf.read('config.ini')
    cf.sections()
    
    
    dbManager.DB_HOST          = cf.get('db', 'DB_HOST')
    dbManager.USER             = cf.get('db', 'USER')
    dbManager.PWD              = cf.get('db', 'PWD')
    dbManager.DATABASE         = cf.get('db', 'DATABASE')
    DIRECTORY_PATH             = cf.get('parameter', 'DIRECTORY_PATH')
    InfoProcess.MOVE_FILE      = cf.getboolean('rename', 'MOVE_FILE')
    InfoProcess.RENAME_BY_EQUIP = cf.getboolean('rename', 'RENAME_BY_EQUIP')
    InfoProcess.MOVE_FLODER    = cf.get('rename', 'MOVE_FLODER')
    InfoProcess.CAMERA_EQUIP_FILE  = cf.get('rename', 'CAMERA_EQUIP_FILE')
    InfoProcess.USING_IMG_COMPLETE = cf.getboolean('parameter', 'USING_IMG_COMPLETE')
    
    # Standardization the user input
    DIRECTORY_PATH = os.path.abspath(DIRECTORY_PATH)
    InfoProcess.MOVE_FLODER = os.path.abspath(InfoProcess.MOVE_FLODER) + os.path.sep
    
    
    # print configration
    print('watch floder :', DIRECTORY_PATH)
    print('is rename    :', InfoProcess.MOVE_FILE)
    print('rename floder:', InfoProcess.MOVE_FLODER)

def read_camera_equipment():
    if InfoProcess.RENAME_BY_EQUIP and InfoProcess.CAMERA_EQUIP_FILE != '':
        f = open(InfoProcess.CAMERA_EQUIP_FILE, 'rt')
        while True:
            ss = f.readline()
            ss = ss.strip('\n')
            if ss == '':
                break
            arr = ss.split(',')
            InfoProcess.CAMERAID_EQUIPMENTID[arr[0]] = arr[1]
        f.close()
    #print(InfoProcess.CAMERAID_EQUIPMENTID)

def main_server():
    
    # init global variables from file config.ini 
    readConfig()
    
    # init camera_equipment_table
    read_camera_equipment()
    
    fileWatcher.watchFileChange(DIRECTORY_PATH, handle_change)
    return

if __name__=='__main__':
    print('imgInfoGeter imgServer start')
    
    if len(sys.argv) > 1:
        DIRECTORY_PATH = sys.argv[1]
    else:
        DIRECTORY_PATH = '.'
    main_server()
    
    ''' # test do process
    fl = ['../res/5.3/20130503170514-db98-0002-1.jpg', 
          '../res/5.3/20130503170514-db98-0002-2.jpg', 
          '../res/5.3/20130503170515-db98-0002-3.jpg']
    for i in fl:
        print(i)
        print(do_process_file(i))
    '''
