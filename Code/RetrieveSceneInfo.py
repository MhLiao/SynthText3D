import os
import sys
import glob
import numpy as np
from unrealcv import Client
import cv2
import matplotlib.pyplot as plt
import time
import subprocess
from io import BytesIO as BytesIO
import PIL.Image as Image
import tqdm


def execute_binary_engine(engine):
    engine_root = f"{engine}/LinuxNoEditor/UnrealText/Binaries/Linux/"
    # temporary
    os.system(f"chmod +x {engine_root+'UnrealText'}")
    command = engine_root+'UnrealText > engine_log.txt &'
    print(command)
    subprocess.call(command, shell=True)
    print("start to run")

GameBinaryName = "RealisticRendering"
Env = 'normal'
Env = '_' + Env if Env else ""
print(f'working on {GameBinaryName+Env}')
execute_binary_engine(GameBinaryName+Env)
time.sleep(10.)
engine_path = f"{GameBinaryName+Env}/LinuxNoEditor/UnrealText/Binaries/Linux/"
TRAJ_PATH = f"../SceneInfo/trajectory/{GameBinaryName}.txt"
OUTPUT_NAME = f"../SceneInfo/{GameBinaryName}/"
# OUTPUT_NAME = '/home/megvii'
IMG_OUTPUT_NAME = os.path.join(OUTPUT_NAME, 'Img')
Normal_OUTPUT_NAME = os.path.join(OUTPUT_NAME, 'Normal')
obj_OUTPUT_NAME = os.path.join(OUTPUT_NAME, 'ObjMask')
dep_OUTPUT_NAME = os.path.join(OUTPUT_NAME, 'Depth')

client = Client(('localhost', 9999))

for path in [IMG_OUTPUT_NAME, Normal_OUTPUT_NAME, obj_OUTPUT_NAME, dep_OUTPUT_NAME]:
    os.makedirs(path, exist_ok=True)

def read_png(res):
    img = np.asarray(Image.open(BytesIO(res)))
    return img


shuqian = 0
location=[]
rotation=[]
currentLocation = np.zeros(3,dtype=np.float32)
currentRotation = np.zeros(3,dtype=np.float32)
traj_file = open(TRAJ_PATH)
for lineNum in traj_file:
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

        storeLoc = np.zeros(3,dtype=np.float32)
        storeRot = np.zeros(3,dtype=np.float32)
        storeLoc[0]=currentLocation[0]
        storeLoc[1]=currentLocation[1]
        storeLoc[2]=currentLocation[2]
        storeRot[0]=currentRotation[0]
        storeRot[1]=currentRotation[1]
        storeRot[2]=currentRotation[2]

        location.append(storeLoc)
        rotation.append(storeRot)

#==================================================


client.connect()
if not client.isconnected():
    print('UnrealCV server is not running.')

client.request('vrun r.setRes 1080x720w')

curCnt=0
for cur in tqdm.tqdm(location):
    curLoc = location[curCnt]
    curRot = rotation[curCnt]
    curCnt+=1
    try:
        client.request('vset /camera/0/location '+str(curLoc[0])+' '+str(curLoc[1])+' '+str(curLoc[2]))
        client.request('vset /camera/0/rotation '+str(curRot[0])+' '+str(curRot[1])+' '+str(curRot[2]))
        client.request('vset /camera/0/lit')
        client.request('vget /camera/0/screenshot')

        img = client.request('vget /camera/0/normal png')
        cv2.imwrite(Normal_OUTPUT_NAME+'/'+str(curCnt)+".png",read_png(img)) # float?
    except:
        print(f'Error at loc-{curCnt}')

client.request('vrun quit')
print('start to move pngs')
isOK = 'y'#input('OK? (y/n)')
if isOK == 'y': 
    lit_imgs = glob.glob(engine_path+ '*.png')
    lit_imgs.sort()
    for i, file in tqdm.tqdm(enumerate(lit_imgs)):
        subprocess.call(f'mv {file} {IMG_OUTPUT_NAME+"/"+file.split("/")[-1].strip("0")}', shell=True)


client.disconnect()