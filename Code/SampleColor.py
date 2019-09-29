import os
import sys
import cv2
import random
import numpy as np
import math
import argparse
from PIL import Image, ImageDraw
import time
import tqdm
from skimage.measure import block_reduce


def main():
    root = '../Data/SceneInfo'
    if len(sys.argv) > 1:
        scenes = [sys.argv[1]]
    else:
        scenes = ['Cabble']
    for scene in scenes:
        sceneRoot = os.path.join(root, scene)
        img_dir = os.path.join(sceneRoot, 'Img')
        normal_dir  = os.path.join(sceneRoot, 'Normal')
        bbox_dir = os.path.join(sceneRoot, 'BoxProposals')
        vis_dir = os.path.join(sceneRoot, 'Vis')
        color_dir = os.path.join(sceneRoot, 'Color')
        if not os.path.exists(color_dir):
            os.mkdir(color_dir)
        for img_id in tqdm.tqdm(range(900)):
            img_name = os.path.join(img_dir, f'{img_id}.png')
            normal_name = os.path.join(normal_dir, f'{img_id}.png')
            if not(os.path.isfile(img_name) and os.path.isfile(normal_name)):
                continue
            IMG = cv2.imread(img_name)
            boxes = []
            colors = []
            for line in open(os.path.join(bbox_dir, f'{img_id}.txt'), 'r').readlines():
                if len(line)>0:
                    line = line.split(',')
                    x0 = int(line[0])
                    y0 = int(line[1])
                    x1 = int(line[2])
                    y1 = int(line[3])
                    AvgColor = np.mean(np.mean(IMG.astype(np.float)[y0:y1, x0:x1], axis=0), axis=0, keepdims=True).astype(np.float)[:3] # (1, 3)
                    while True:
                        if random.random() < 0.1:
                            randomColor = np.random.uniform(235, 254, size=(1, 3)).astype(np.float) 
                        else:
                            randomColor = np.random.uniform(0, 255, size=(1, 3)).astype(np.float) 
                            randomColor /= np.max(randomColor)
                            randomColor *= np.random.random()**1.7 * 255
                        Distance = np.sum(np.abs(randomColor - AvgColor[:, :3]), axis=1)
                        if int(np.sum(Distance)) > 30 :
                            break
                    ColorChoice = int(np.argmax(Distance))
                    color = np.reshape(randomColor[ColorChoice, :3], (3,))
                    boxes.append([x0, y0, x1, y1])
                    colors.append([min(int(color[0]), 255), min(int(color[1]), 255),min(int(color[2]), 255)])
            with open(os.path.join(bbox_dir, f'{img_id}_.txt'), 'w') as f:
                for b in boxes:
                    gt=','.join(map(str,b))+'\n'
                    f.write(gt)
            with open(os.path.join(color_dir, f'{img_id}.txt'), 'w') as f:
                for b in colors:
                    gt=','.join(map(str,b))+'\n'
                    f.write(gt)
main()