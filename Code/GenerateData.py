from __future__ import division

import os
import sys
import numpy as np
from unrealcv import Client

import matplotlib.pyplot as plt 
import scipy.io as sio
import os.path as osp
import random, os
import cv2
import pickle as cp
import scipy.signal as ssig
import scipy.stats as sstat
import pygame, pygame.locals
from pygame import freetype
from PIL import Image
import math
import random
import time
import tqdm

import text_utils as tu
from common import *

import sys

client = Client(('localhost', 9999))
time.sleep(10)
client.connect()
wait_time = 1.
while not client.isconnected():
    print('UnrealCV server is not running.')
    time.sleep(wait_time)
    client.connect()
    wait_time *= 2
    if wait_time > 8: 
        print('Failed! Kill process ..')
        os.system('ps aux|grep UnrealText >> ./KillList.txt')
        for line in open('./KillList.txt', 'r').readlines():
            if "Linux/Unreal" in line:
                ID = line.split()[1]
                os.system(f'kill {ID}')
        exit(0)
client.request('vrun r.setRes 1080x720w')
print('Set as Window Mode')

MAXPUGTEXT=8
MAXANGLE=1

root = '../SceneData'
scene = 'RealisticRendering' if len(sys.argv) < 4 else sys.argv[1]
env = 'gen' if len(sys.argv) < 4 else sys.argv[2]
GenerateNumber = 100 if len(sys.argv) < 4 else int(sys.argv[3])
print(" ===== ===== ===== generate new boxes ===== ===== =====")
os.system(f'python3 ./GenerateBox.py {scene}')
print(" ===== ===== ===== generate new colors ===== ===== =====")
os.system(f'python3 ./SampleColor.py {scene}')


TRAJ_PATH = f'{root}/trajectory/{scene}.txt'
BBOX_PATH = f'{root}/scene_info/{scene}/BoxProposals/'
OUTPUT_NAME = f'{root}/scene_info/{scene}/Finaldata_{env}/'
TEXTURE_PATH = "../WordCrops/"
TEXTCOLOR_PATH = f'{root}/scene_info/{scene}/Color/'
BBOX_OUTPUT_PATH = f'{root}/scene_info/{scene}/Finaldata_{env}/BBOX/'
TEXT_OUTPUT_PATH = f'{root}/scene_info/{scene}/Finaldata_{env}/TEXT/'
CHARBBOX_OUTPUT_PATH = f'{root}/scene_info/{scene}/Finaldata_{env}/CBOX/'
CHAR_OUTPUT_PATH = f'{root}/scene_info/{scene}/Finaldata_{env}/CHAR/'

output_dirs = [OUTPUT_NAME, BBOX_OUTPUT_PATH, TEXT_OUTPUT_PATH, CHARBBOX_OUTPUT_PATH, CHAR_OUTPUT_PATH]

for output_dir in output_dirs:
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)

def load_loc_rot(traj_file):
    print('Reading trajectory:')
    shuqian = 0
    location=[]
    rotation=[]
    currentLocation = np.zeros(3,dtype=np.float32)
    currentRotation = np.zeros(3,dtype=np.float32)
    for lineNum in tqdm.tqdm(traj_file):
        if shuqian==0:
            currentLocation[0]=float(lineNum)
            shuqian+=1
        elif shuqian==1:
            currentLocation[1]=float(lineNum)
            shuqian+=1
        elif shuqian==2:
            currentLocation[2]=float(lineNum)
            shuqian+=1
        elif shuqian==3:
            currentRotation[0]=float(lineNum)
            shuqian+=1
        elif shuqian==4:
            currentRotation[1]=float(lineNum)
            shuqian+=1
        elif shuqian==5:
            currentRotation[2]=float(lineNum)
            shuqian=0

            storeLoc = currentLocation.copy()
            storeRot = currentRotation.copy()

            location.append(storeLoc)
            rotation.append(storeRot)
    return location, rotation

def get_bboxes_text_colors(bbox_path, text_color_path):
    bboxs_of_images = []
    textcolors_of_images = []
    fileCnt=1
    print('Reading boxes:')
    for filename in tqdm.tqdm(os.listdir(text_color_path)):
        bboxs_of_curimg = []
        textcolors_of_curimg = []
        real_box_filename = os.path.join(bbox_path,str(fileCnt)+'_.txt')
        real_color_filename = os.path.join(text_color_path,str(fileCnt)+'.txt')
        if os.path.isfile(real_box_filename) and os.path.isfile(real_color_filename):
            file_object = open(real_box_filename,'r')
            tc_file = open(real_color_filename,'r')
            for line in file_object:
                linArr = line.strip().split(",")
                bboxs_of_curimg.append([float(linArr[0]), float(linArr[1]), float(linArr[2]), float(linArr[3])])
                if bboxs_of_curimg[-1][2]-bboxs_of_curimg[-1][0] > 400 and bboxs_of_curimg[-1][3]-bboxs_of_curimg[-1][1] > 400:
                    UL_X = random.randint(bboxs_of_curimg[-1][0], bboxs_of_curimg[-1][2] - 400)
                    UL_Y = random.randint(bboxs_of_curimg[-1][1], bboxs_of_curimg[-1][3] - 400)
                    bboxs_of_curimg[-1][0] = UL_X
                    bboxs_of_curimg[-1][1] = UL_Y
                    bboxs_of_curimg[-1][2] = UL_X + 400
                    bboxs_of_curimg[-1][3] = UL_Y + 400

                if bboxs_of_curimg[-1][3]-bboxs_of_curimg[-1][1] > 200 and random.random() < 0.8:
                    length = bboxs_of_curimg[-1][3]-bboxs_of_curimg[-1][1] - 200
                    bboxs_of_curimg[-1][3] = 200 + random.randint(0, length) + bboxs_of_curimg[-1][1]
                    bboxs_of_curimg[-1][1] = bboxs_of_curimg[-1][3] - 200

                if bboxs_of_curimg[-1][3]-bboxs_of_curimg[-1][1] > 100 and random.random() < 0.5:
                    length = bboxs_of_curimg[-1][3]-bboxs_of_curimg[-1][1] - 100
                    bboxs_of_curimg[-1][3] = 100 + length + bboxs_of_curimg[-1][1]
                    bboxs_of_curimg[-1][1] = bboxs_of_curimg[-1][3] - 100

                if bboxs_of_curimg[-1][2]-bboxs_of_curimg[-1][0] > 600 and random.random() < 0.9:
                    length = bboxs_of_curimg[-1][2]-bboxs_of_curimg[-1][0] - 600
                    bboxs_of_curimg[-1][2] = 600 + length + bboxs_of_curimg[-1][0]
                    bboxs_of_curimg[-1][0] = bboxs_of_curimg[-1][2] - 600
                    if bboxs_of_curimg[-1][2]-bboxs_of_curimg[-1][0] > 400 and random.random() < 0.5:
                        length = bboxs_of_curimg[-1][2]-bboxs_of_curimg[-1][0] - 400
                        bboxs_of_curimg[-1][2] = 400 + length + bboxs_of_curimg[-1][0]
                        bboxs_of_curimg[-1][0] = bboxs_of_curimg[-1][2] - 400
            file_object.close()
            tc_R=0
            tc_G=0
            tc_B=0
            for line in tc_file:
                lineArr = line.strip().split(",")
                tc_R=float(lineArr[0])
                tc_G=float(lineArr[1])
                tc_B=float(lineArr[2])
                randScale=-1.0+2.0*random.random()
                tc_R=max(min(tc_R+randScale*40, 254), 5)
                randScale=-1.0+2.0*random.random()
                tc_G=max(min(tc_G+randScale*40, 254), 5)
                randScale=-1.0+2.0*random.random()
                tc_B=max(min(tc_B+randScale*40, 254), 5)
                textcolors_of_curimg.append([tc_R,tc_G,tc_B])
            tc_file.close()
        area = [(box[2]-box[0])*(box[3]-box[1]) for box in bboxs_of_curimg]
        aspect_ratio = [(box[2]-box[0])/(box[3]-box[1]) for box in bboxs_of_curimg]

        bboxs_of_images.append(bboxs_of_curimg)
        textcolors_of_images.append(textcolors_of_curimg)
        fileCnt+=1
    return bboxs_of_images, textcolors_of_images, fileCnt


WINDOW_H=720.0
WINDOW_W=1080.0
REPEAT = 1

DATA_PATH = '../fonts&corpus'
traj_file = open(TRAJ_PATH)

location, rotation = load_loc_rot(traj_file)
bboxs_of_images, textcolors_of_images, fileCnt = get_bboxes_text_colors(BBOX_PATH, TEXTCOLOR_PATH)
text_renderer = tu.RenderFont(DATA_PATH)
text_renderer.setStoredToPath(TEXTURE_PATH);
print('trajectory, boxes, text renderer loaded')

allIndex = [i for i in range(fileCnt-1)]
curCnt=random.choice(allIndex)
assert len(location) == len(rotation) and len(rotation) == fileCnt-1
print(len(location),len(rotation),fileCnt)
outputCnt=0
for ffi in range((fileCnt-1)*REPEAT):
    curCnt = ffi
    curLoc = location[curCnt]
    curRot = rotation[curCnt]
    client.request('vset /camera/0/location '+str(curLoc[0])+' '+str(curLoc[1])+' '+str(curLoc[2]))
    client.request('vset /camera/0/rotation '+str(curRot[0])+' '+str(curRot[1])+' '+str(curRot[2]))
    time1=time.clock()
    imagebboxsNum = min(len(bboxs_of_images[curCnt]), MAXPUGTEXT)
    mixed_id = [i for i in range(len(bboxs_of_images[curCnt]))]
    random.shuffle(mixed_id)
    bboxs_of_images[curCnt] = [bboxs_of_images[curCnt][i] for i in mixed_id]
    textcolors_of_images[curCnt] = [textcolors_of_images[curCnt][i] for i in mixed_id]
    for i in range(imagebboxsNum):
        result = client.request('vget /object/PugTextPawn/location '+str(i))
        if "args1 out of range" in result:
            print('\n===== ===== ===== Not enough PugText!. Skip. ==== ==== ==== \n')
            imagebboxsNum = i
            break
        client.request("vset /object/PugTextPawn/3DGeomFlag "+str(i)+" 0");
        client.request("vset /object/PugTextPawn/texthide "+str(i)+" 0");

        bboxs_of_images[curCnt][i][0]+=2.0
        bboxs_of_images[curCnt][i][1]+=2.0
        bboxs_of_images[curCnt][i][2]-=2.0
        bboxs_of_images[curCnt][i][3]-=2.0

        hor_angle0 = bboxs_of_images[curCnt][i][0]
        ver_angle0 = bboxs_of_images[curCnt][i][1]
        hor_angle1 = bboxs_of_images[curCnt][i][2]
        ver_angle1 = bboxs_of_images[curCnt][i][3]
        client.request("vset /object/PugTextPawn/angle "+str(i)+" "+str(hor_angle0)+" "+str(ver_angle0)+" "+str(hor_angle1)+" "+str(ver_angle1) );

    for i in range(imagebboxsNum,MAXPUGTEXT):
        client.request("vset /object/PugTextPawn/texthide "+str(i)+" 1");

    bboxs_num_of_cluster=[]
    chars_num_of_cluster=[]
    texts_of_cluster=[]
    chars_of_cluster=[]
    enters_of_string_of_cluster=[]
    enters_of_bbox_of_cluster=[]

    for i in range(imagebboxsNum):
        hor_angle0 = bboxs_of_images[curCnt][i][0]
        ver_angle0 = bboxs_of_images[curCnt][i][1]
        hor_angle1 = bboxs_of_images[curCnt][i][2]
        ver_angle1 = bboxs_of_images[curCnt][i][3]

        lightColorR=textcolors_of_images[curCnt][i][2]
        lightColorG=textcolors_of_images[curCnt][i][1]
        lightColorB=textcolors_of_images[curCnt][i][0]

        if(lightColorR>250):
            lightColorR=250
        if(lightColorG>250):
            lightColorG=250
        if(lightColorB>250):
            lightColorB=250

        text_renderer.setCurCnt(i)
        text_renderer.setColor(lightColorR, lightColorG, lightColorB)
        bbox_h = bboxs_of_images[curCnt][i][3]-bboxs_of_images[curCnt][i][1]
        bbox_w = bboxs_of_images[curCnt][i][2]-bboxs_of_images[curCnt][i][0]
        ratio = 1
        failed_flag = False
        loop_count = 0
        while True:
            loop_count += 1
            if loop_count > 10:
                failed_flag = True
                break
            try:
                if bbox_h > 100 and bbox_h < 300 and bbox_w > 120 and bbox_w<360 and random.random() < 0.2:
                     ratio = 2
                text, bbs = text_renderer.getRandomTextImg(bbox_h * ratio, bbox_w * ratio)
                if len(text)==0 or np.any([len(line)==0 for line in text]):
                    text, bbs = text_renderer.getRandomTextImg(bbox_h * ratio, bbox_w * ratio)
                    if len(text)==0 or np.any([len(line)==0 for line in text]):
                        text, bbs = text_renderer.getRandomTextImg(bbox_h * ratio, bbox_w * ratio)
                        if len(text)==0 or np.any([len(line)==0 for line in text]):
                            print('render failed')
                            failed_flag = True
                break
            except:
                pass
        if failed_flag: continue
        textCnt=0
        inword=False
        bbox2D=[]
        text2D=[]
        bbox2D_char=[]
        text2D_char=[]
        enter_index_string=[]
        enter_index_bbox=[]
        bb_start_x=bb_start_y=bb_end_x=bb_end_y=0
        tbi=0
        wordStart=0
        charCnt=0
        enter_index_bbox.append(0)
        enter_index_string.append(0)
        for bi in range(len(text)):
            if (text[bi]!='\n' and text[bi]!='\0' and text[bi]!=' '):
                bbox2D_char.append([bbs[charCnt][0],bbs[charCnt][1],bbs[charCnt][0]+bbs[charCnt][2],bbs[charCnt][1]+bbs[charCnt][3]])
                charCnt+=1
                text2D_char.append(text[bi])
            if (text[bi]=='\n'):
                enter_index_string.append(bi)
                enter_index_bbox.append(charCnt)
            if(inword and bi==len(text)-1):
                bb_start_y=min(bbs[tbi][1],bb_start_y)
                bb_end_x=bbs[tbi][0]+bbs[tbi][2]
                bb_end_y=max(bbs[tbi][1]+bbs[tbi][3],bb_end_y)
                inword=False
                bbox2D.append([bb_start_x,bb_start_y,bb_end_x,bb_end_y])
                text2D.append(text[wordStart:bi+1])
                bb_start_x=bb_start_y=bb_end_x=bb_end_y=0
                tbi+=1
            elif(inword and (text[bi+1]=='\n' or text[bi+1]=='\0' or text[bi+1]==' ')):
                bb_start_y=min(bbs[tbi][1],bb_start_y)
                bb_end_x=bbs[tbi][0]+bbs[tbi][2]
                bb_end_y=max(bbs[tbi][1]+bbs[tbi][3],bb_end_y)
                inword=False
                bbox2D.append([bb_start_x,bb_start_y,bb_end_x,bb_end_y])
                text2D.append(text[wordStart:bi+1])
                bb_start_x=bb_start_y=bb_end_x=bb_end_y=0
                tbi+=1
            elif (inword==False and text[bi]!='\n' and text[bi]!='\0' and text[bi]!=' ' and bi!=len(text)-1):
                bb_start_x=bbs[tbi][0]
                bb_start_y=bbs[tbi][1]
                bb_end_y=max(bb_end_y,bbs[tbi][1]+bbs[tbi][3])
                inword=True
                tbi+=1
                wordStart=bi
            elif (inword==True and text[bi]!='\n' and text[bi]!='\0' and text[bi]!=' '):
                bb_start_y=min(bbs[tbi][1],bb_start_y)
                bb_end_y=max(bb_end_y,bbs[tbi][1]+bbs[tbi][3])
                tbi+=1
            elif (inword==False and text[bi]!='\n' and text[bi]!='\0' and text[bi]!=' ' and bi==len(text)-1):
                bb_start_x=bbs[tbi][0]
                bb_start_y=bbs[tbi][1]
                bb_end_x=bbs[tbi][0]+bbs[tbi][2]
                bb_end_y=bbs[tbi][1]+bbs[tbi][3]
                bbox2D.append([bb_start_x,bb_start_y,bb_end_x,bb_end_y])
                text2D.append(text[wordStart:bi+1])
                bb_start_x=bb_start_y=bb_end_x=bb_end_y=0
                tbi+=1
        texts_of_cluster.append(text2D)
        chars_of_cluster.append(text2D_char)
        bboxs_num_of_cluster.append(len(bbox2D))
        chars_num_of_cluster.append(len(text2D_char))
        enters_of_string_of_cluster.append(enter_index_string)
        enters_of_bbox_of_cluster.append(enter_index_bbox)
        ss=0
        client.request("vset /object/PugTextPawn/texture1 "+str(i)+" "+"fore_text"+str(i))
        client.request("vset /object/PugTextPawn/texture2 "+str(i)+" "+"back_text"+str(i))
        emission = 0.0
        moveFlag=1
        seperateheight=0.1
        geomFlag=0
        curveGapPercent_f=0.0

        geomSeed = random.randint(0,9)
        if (geomSeed<10):
            lightSeed = random.random()
            if (lightSeed<0.05):
                emission = float(random.randint(10,30)/10.0)
                lightColorR/=255.0
                lightColorG/=255.0
                lightColorB/=255.0
                if(lightColorR<0.2):
                    lightColorR=0.2
                if(lightColorG<0.2):
                    lightColorG=0.2
                if(lightColorB<0.2):
                    lightColorB=0.2

                if(lightColorR>0.9):
                    lightColorR=0.9
                if(lightColorG>0.9):
                    lightColorG=0.9
                if(lightColorB>0.9):
                    lightColorB=0.9
                shrikIndex=random.randint(0,7)
                if shrikIndex==0:
                    lightColorR*=float(random.randint(3,8)/10.0)
                elif shrikIndex==1:
                    lightColorG*=float(random.randint(3,8)/10.0)
                elif shrikIndex==2:
                    lightColorB*=float(random.randint(3,8)/10.0)
                elif shrikIndex==3:
                    lightColorR*=float(random.randint(3,8)/10.0)
                    lightColorG*=float(random.randint(3,8)/10.0)
                elif shrikIndex==4:
                    lightColorG*=float(random.randint(3,8)/10.0)
                    lightColorB*=float(random.randint(3,8)/10.0)
                elif shrikIndex==5:
                    lightColorR*=float(random.randint(3,8)/10.0)
                    lightColorB*=float(random.randint(3,8)/10.0)
                moveFlag=1
                seperateheight=0.1
                geomFlag=0
                curveGapPercent_f=0.0
                client.request("vset /object/PugTextPawn/param "+str(i)+" "+str(hor_angle0)+" "+str(ver_angle0)+" "+str(hor_angle1)+" "+str(ver_angle1)+" fore_text"+str(i)+" back_text"+str(i)+" "+str(moveFlag)+" "+str(emission)+" "+str(lightColorR)+" "+str(lightColorG)+" "+str(lightColorB)+" "+str(seperateheight)+" "+str(geomFlag)+" "+str(curveGapPercent_f) )
            else:
                emission=0.0
                moveFlag=1
                seperateheight=0.1
                geomFlag=0
                curveGapPercent_f=0.0
                client.request("vset /object/PugTextPawn/param "+str(i)+" "+str(hor_angle0)+" "+str(ver_angle0)+" "+str(hor_angle1)+" "+str(ver_angle1)+" fore_text"+str(i)+" back_text"+str(i)+" "+str(moveFlag)+" "+str(emission)+" "+str(lightColorR)+" "+str(lightColorG)+" "+str(lightColorB)+" "+str(seperateheight)+" "+str(geomFlag)+" "+str(curveGapPercent_f) )
        else:
            geomFlag=0
            curveSeed = random.randint(0,99)
            curveGapPercent = random.randint(0,20)
            curveGapPercent_f = float(curveGapPercent)/100.0
            emission=0.0
            moveFlag=1
            seperateheight=0.1
            client.request("vset /object/PugTextPawn/param "+str(i)+" "+str(hor_angle0)+" "+str(ver_angle0)+" "+str(hor_angle1)+" "+str(ver_angle1)+" fore_text"+str(i)+" back_text"+str(i)+" "+str(moveFlag)+" "+str(emission)+" "+str(lightColorR)+" "+str(lightColorG)+" "+str(lightColorB)+" "+str(seperateheight)+" "+str(geomFlag)+" "+str(curveGapPercent_f) )
            client.request("vset /object/PugTextPawn/3DMat "+str(i)+" "+str(1));
            client.request("vset /object/PugTextPawn/gapLength "+str(i)+" "+str(10.0));
            client.request("vset /object/PugTextPawn/3DGeomFlag "+str(i)+" 1")
        for bi in range(len(bbox2D)):
            client.request("vset /object/PugTextPawn/bbox2D "+str(i)+" "+str(bi)+" "+str(len(bbox2D))+" "+str(bbox2D[bi][0])+" "+str(bbox2D[bi][1])+" "+str(bbox2D[bi][2])+" "+str(bbox2D[bi][3]))

        client.request("vset /object/PugTextPawn/needMove "+str(i)+" "+"0");
    wordBboxLocation = []
    for i in range(imagebboxsNum):
        cluster_wordBboxLocation=[]
        for bi in range(bboxs_num_of_cluster[i]):
            bbox3DString = client.request("vget /object/PugTextPawn/bbox3D "+str(i)+" "+str(bi))
            linArr = bbox3DString.strip().split(",")
            cluster_wordBboxLocation.append([float(linArr[0]), float(linArr[1]), float(linArr[2])])
            cluster_wordBboxLocation.append([float(linArr[3]), float(linArr[4]), float(linArr[5])])
            cluster_wordBboxLocation.append([float(linArr[6]), float(linArr[7]), float(linArr[8])])
            cluster_wordBboxLocation.append([float(linArr[9]), float(linArr[10]), float(linArr[11])])
        wordBboxLocation.append(cluster_wordBboxLocation)

    time2=time.clock()
    cameraLocString = client.request('vget /camera/0/location');
    cameraRotString = client.request('vget /camera/0/rotation_withDirection');
    clocArr = cameraLocString.strip().split(" ")
    crotArr = cameraRotString.strip().split(" ")
    clocArr[0]=float(clocArr[0])
    clocArr[1]=float(clocArr[1])
    clocArr[2]=float(clocArr[2])
    crotArr[0]=float(crotArr[0])
    crotArr[1]=float(crotArr[1])
    crotArr[2]=float(crotArr[2])
    crotArr[3]=float(crotArr[3])
    crotArr[4]=float(crotArr[4])
    crotArr[5]=float(crotArr[5])
    crotArr[0] /= 180.0
    crotArr[1] /= 180.0
    crotArr[2] /= 180.0
    crotArr[0] *= 3.1415926
    crotArr[1] *= 3.1415926
    crotArr[2] *= 3.1415926

    crotArr_backup=np.zeros(3,dtype=np.float32)
    crotArr_backup[0]=crotArr[0]
    crotArr_backup[1]=crotArr[1]
    crotArr_backup[2]=crotArr[2]

    movedLocation = np.zeros(3)
    movedRotation = np.zeros(3)

    if imagebboxsNum==0:
        curCnt+=1
        continue
    
    word0Location = np.zeros(3)
    word0Location[0] = wordBboxLocation[0][0][0]
    word0Location[1] = wordBboxLocation[0][0][1]
    word0Location[2] = wordBboxLocation[0][0][2]
    for i in range(1,4):
        word0Location[0]+=wordBboxLocation[0][i][0]
        word0Location[1]+=wordBboxLocation[0][i][1]
        word0Location[2]+=wordBboxLocation[0][i][2]
    word0Location[0]/=4.0
    word0Location[1]/=4.0
    word0Location[2]/=4.0
    
    
    for viewpoint_i in range(0,MAXANGLE):
        xShift=40.0*(2.0*random.random()-1.0)
        yShift=40.0*(2.0*random.random()-1.0)
        zShift=40.0*(2.0*random.random()-1.0)
        movedLocation[0]=curLoc[0] + xShift
        movedLocation[1]=curLoc[1] + yShift
        movedLocation[2]=curLoc[2] + zShift
        client.request('vset /camera/0/location '+str(movedLocation[0])+' '+str(movedLocation[1])+' '+str(movedLocation[2]))
        pShift=random.random()*0.4-0.2
        yShift=random.random()*0.4-0.2
        crotArr[0]=crotArr_backup[0]+pShift
        crotArr[1]=crotArr_backup[1]+yShift
        crotArr[2]=0.0
        client.request('vset /camera/0/rotation '+str((crotArr[0])*180.0/3.1415926)+' '+str(crotArr[1]*180.0/3.1415926)+' '+str(crotArr[2]*180.0/3.1415926))
        
        time.sleep(0.5)
        client.request('vget /camera/0/screenshot')

        crotArr[1]+=3.1415926*2.0
        crotArr[1]%=3.1415926*2.0
        crotArr[0]+=3.1415926*2.0
        crotArr[0]%=3.1415926*2.0
        if(crotArr[0]>(3.1415926) ):
            crotArr[0]=crotArr[0]-(3.1415926*2.0)
        
        real_filename = str(outputCnt+1)+'.txt'
        bbox_output_file_object = open(os.path.join(BBOX_OUTPUT_PATH ,real_filename),'w')
        text_output_file_object = open(os.path.join(TEXT_OUTPUT_PATH ,real_filename),'w')
        for i in range(imagebboxsNum):
            for bi in range(bboxs_num_of_cluster[i]):
                text_output_file_object.write(str(texts_of_cluster[i][bi]+'\n'))                
                bboxProjectedString = client.request("vget /object/PugTextPawn/bboxProjected "+str(i)+" "+str(bi))
                if bboxProjectedString is None: continue
                linArr = bboxProjectedString.strip().split(",")
                for wi in range(4):
                    bbox_output_file_object.write(str(int(float(linArr[wi*2])))+","+str(int(float(linArr[wi*2+1])))+"\n")
        bbox_output_file_object.close()
        text_output_file_object.close()

        charbbox=[]
        charbbox_output_file_object = open(os.path.join(CHARBBOX_OUTPUT_PATH ,real_filename),'w')
        char_output_file_object = open(os.path.join(CHAR_OUTPUT_PATH ,real_filename),'w')
        for i in range(imagebboxsNum):
            break
            for bi in range(chars_num_of_cluster[i]):
                char_output_file_object.write(str(chars_of_cluster[i][bi])+'\n')
                charProjectedString = client.request("vget /object/PugTextPawn/charProjected "+str(i)+" "+str(bi))
                linArr = charProjectedString.strip().split(",")
                for wi in range(4):
                    charbbox_output_file_object.write(str(int(float(linArr[wi*2])))+","+str(int(float(linArr[wi*2+1])))+"\n" )
                    charbbox.append([int(float(linArr[wi*2])),int(float(linArr[wi*2+1]))])
        char_output_file_object.close()
        charbbox_output_file_object.close()
        outputCnt+=1

    for viewpoint_i in range(0,MAXANGLE):
        xShift=40.0*(2.0*random.random()-1.0)
        yShift=40.0*(2.0*random.random()-1.0)
        zShift=40.0*(2.0*random.random()-1.0)
        movedLocation[0]=curLoc[0] + xShift
        movedLocation[1]=curLoc[1] + yShift
        movedLocation[2]=curLoc[2] + zShift

        tmpwordloc1 = word0Location[0] - movedLocation[0]
        tmpwordloc2 = word0Location[1] - movedLocation[1]
        tmpwordloc3 = word0Location[2] - movedLocation[2]
        theta_yaw=math.atan2(tmpwordloc2, tmpwordloc1)
        theta_pitch=math.atan2(tmpwordloc3, math.sqrt(tmpwordloc1*tmpwordloc1+tmpwordloc2*tmpwordloc2))

        client.request('vset /camera/0/location '+str(movedLocation[0])+' '+str(movedLocation[1])+' '+str(movedLocation[2]))

        rShift=random.random()*0.8-0.4
        crotArr[0]=theta_pitch+rShift
        rShift=random.random()*0.8-0.4
        crotArr[1]=theta_yaw+rShift
        crotArr[2]=crotArr_backup[2]

        crotArr[1]+=3.1415926*2.0
        crotArr[1]%=3.1415926*2.0
        crotArr[0]+=3.1415926*2.0
        crotArr[0]%=3.1415926*2.0
        if(crotArr[0]>(3.1415926) ):
            crotArr[0]=crotArr[0]-(3.1415926*2.0)
        client.request('vset /camera/0/rotation '+str((crotArr[0])*180.0/3.1415926)+' '+str(crotArr[1]*180.0/3.1415926)+' '+str(crotArr[2]*180.0/3.1415926))
        time.sleep(0.5)
        client.request('vget /camera/0/screenshot')        

        real_filename = str(outputCnt+1)+'.txt'
        bbox_output_file_object = open(os.path.join(BBOX_OUTPUT_PATH, real_filename),'w')
        text_output_file_object = open(os.path.join(TEXT_OUTPUT_PATH, real_filename),'w')
        for i in range(imagebboxsNum):
            for bi in range(bboxs_num_of_cluster[i]):
                text_output_file_object.write(str(texts_of_cluster[i][bi]+'\n'))
                bboxProjectedString = client.request("vget /object/PugTextPawn/bboxProjected "+str(i)+" "+str(bi))
                linArr = bboxProjectedString.strip().split(",")
                for wi in range(4):
                    bbox_output_file_object.write(str(int(float(linArr[wi*2])))+","+str(int(float(linArr[wi*2+1])))+"\n")
        
        bbox_output_file_object.close()
        text_output_file_object.close()

        charbbox=[]
        charbbox_output_file_object = open(os.path.join(CHARBBOX_OUTPUT_PATH ,real_filename),'w')
        char_output_file_object = open(os.path.join(CHAR_OUTPUT_PATH ,real_filename),'w')
        for i in range(imagebboxsNum):
            for bi in range(chars_num_of_cluster[i]):
                char_output_file_object.write(str(chars_of_cluster[i][bi])+'\n')
                charProjectedString = client.request("vget /object/PugTextPawn/charProjected "+str(i)+" "+str(bi))
                linArr = charProjectedString.strip().split(",")
                for wi in range(4):
                    charbbox_output_file_object.write(str(int(float(linArr[wi*2])))+","+str(int(float(linArr[wi*2+1])))+"\n" )
                    charbbox.append([int(float(linArr[wi*2])),int(float(linArr[wi*2+1]))])
        char_output_file_object.close()
        charbbox_output_file_object.close()

        outputCnt+=1
        
    time3=time.clock()
    print("pre-process time:"+str(time2-time1))
    print("pre-process time:"+str(time3-time2))
    curCnt=random.choice(allIndex)
client.request('vrun quit')
client.disconnect()
