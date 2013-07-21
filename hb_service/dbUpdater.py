# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com

# ר�Ŵ��� update ���ݿ��Ĺ����߳�/����

import queue
import dbManager


# ���ڻ������������߳�sql���Ķ���
q = queue.Queue()

def update_db_server():
    conn = dbManager.get_db_connect()
    cur = conn.cursor()
    while True:
        sql, gpx, gpy, gpstime, roadname, mph, createtime, camera_id = q.get()
        q.task_done()
        print(gpx, gpy, gpstime, roadname, mph, createtime, camera_id)
        try:
            cur.execute(sql, gpx, gpy, gpstime, roadname, mph, createtime, camera_id)
        except:
             print('update db excute error!\n')
    
    dbManager.close_db_connect(conn)
    return



