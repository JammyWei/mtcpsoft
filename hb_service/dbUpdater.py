# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com

# ר�Ŵ��� update ���ݿ��Ĺ����߳�/����

import queue
import dbManager
import time


# ���ڻ������������߳�sql���Ķ���
q = queue.Queue()

def update_db_server():
    sql = "update tbl_heartbeatinfo_new set gpx = ?, gpy = ?, gpstime = ?, roadname = ?, mph = ?, createtime = ? where (camera_id = ?)"
    conn = dbManager.get_db_connect()
    cur = conn.cursor()
    arg_list = list()
    print('update_db_server running')
    time_wait = 1
    get_num =10
    while True:
        #print('in while update db')
        for i in range(get_num):
            try:
                arg_list.append(q.get(False))
                q.task_done()
            except queue.Empty:
                print('empty')#
                time_wait = (time_wait + 1) * 2
                get_num = 10
                break
            
            if time_wait > 0:
                time_wait -= 1
                get_num += 10
            
        #print(gpx, gpy, gpstime, roadname, mph, createtime, camera_id)
        #print(arg_list)
        if len(arg_list) != 0:
            #print(arg_list)
            try:
                cur.executemany(sql, arg_list)
                conn.commit()
                print('execute success')
            except:
                print('update db excute error!\n')
            arg_list.clear()
        print('time_wait', time_wait, 'get_num', get_num)
        time.sleep(time_wait)
    
    dbManager.close_db_connect(conn)
    return



