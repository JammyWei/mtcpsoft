# -*- coding:gbk -*-
# auther : pdm
# email : ppppdm@gmail.com

import os

# ���ڸ�ʽ��
def formate_date(date_str):
    return date_str[:4] + '-' + date_str[4:6]+ '-' + date_str[6:]

# ͼƬ���麯��
def classify_img_by_date(src_floder, dst_floder):
    img_files = os.listdir(src_floder)
    print(img_files)
    
    for file in img_files:
        if '.jpg' in file:
            img_date = file[:8]
            sec_dir = formate_date(img_date)
            sub_path = os.path.join(dst_floder, sec_dir)
            if not os.path.exists(sub_path):
                os.mkdir(sub_path)
            os.rename(os.path.join(src_floder, file), os.path.join(sub_path, file))
    return

# ͼƬ����ʽ�� "����ʱ��-�豸��-���-���ں�.jpg"
# ��ȡͼƬ���������ڶ�ͼƬ���й���

IMG_FLODER = "..\\test\\classify_imgs" #Ҫ���й����ͼƬ���ڵ��ļ���
DES_FLODER = "..\\test\\classify_imgs" #�����ĸ��ļ���

if __name__=="__main__":
    # check
    if not os.path.exists(IMG_FLODER):
        print(IMG_FLODER, 'not exists!')
        exit()
    
    if not os.path.exists(DES_FLODER):
        print(DES_FLODER, 'not exists!')
        exit()
    
    classify_img_by_date(IMG_FLODER, DES_FLODER)

