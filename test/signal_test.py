# -*- coding:gbk -*-
# author : pdm
# email : ppppdm@gmail.com
#

import os  
import signal  
from time import sleep  
   
def onsignal_term(a,b):  
    print('�յ�SIGTERM�ź�')
   
#�����ǰ��źŴ���������SIGTERM���ں���onsignal_term����  
signal.signal(signal.SIGTERM,onsignal_term)  
   
def onsignal_usr1(a,b):  
    print('�յ�SIGUSR1�ź�')  
#�����ǰ��źŴ���������SIGUSR1���ں���onsignal_term����  
signal.signal(signal.SIGINT,onsignal_usr1)  
   
while 1:  
    print('�ҵĽ���id��',os.getpid())  
    sleep(10)  
