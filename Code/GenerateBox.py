import os
import sys
import cv2
import pdb
import numpy as np
import math
import argparse
from PIL import Image, ImageDraw
from box_proposing import box_proposing
import time
import tqdm
from skimage.measure import block_reduce

def get_neighbor_val(img, y, x, w, h):
    ps = []
    if y > 0:
        ps.append(img[y-1, x])
    if y < h-1:
        ps.append(img[y+1, x])
    if x > 0:
        ps.append(img[y, x-1])
    if x < w-1:
        ps.append(img[y, x+1])
    return ps

def generate_normal_deri(normal_img, d_th):
    normal_img = normal_img.copy().astype(float)
    h, w, _ = normal_img.shape
    dd_img = np.zeros((h, w))
    # ipdb.set_trace()
    for i in range(h):
        for j in range(w):
            ps = np.array(get_neighbor_val(normal_img, i, j, w, h))
            ds = []
            for p in ps:
                ds.append(np.linalg.norm(np.array(p) - normal_img[i, j], 1))
            dd_img[i, j] = max(ds)
    dd = (dd_img > d_th).astype(np.uint8)
    return dd

def generate_normal_deri_v2(normal_img, d_th):
    normal_img = normal_img.copy().astype(float)
    sky = (np.sum(normal_img, axis=2) < 10).astype(np.uint8)
    h, w, _ = normal_img.shape
    up_img = np.zeros_like(normal_img)
    up_img[1:, :] = normal_img[:h-1, :]
    up_img[0, :] = normal_img[0, :]
    down_img = np.zeros_like(normal_img)
    down_img[:h-1, :] = normal_img[1:, :]
    down_img[h-1, :] = normal_img[h-1, :]
    left_img = np.zeros_like(normal_img)
    left_img[:, 1:]=normal_img[:, :w-1]
    left_img[:, 0] = normal_img[:, 0]
    right_img = np.zeros_like(normal_img)
    right_img[:, :w-1] = normal_img[:, 1:]
    right_img[:, w - 1] = normal_img[:, w-1]
    d_left = np.linalg.norm(left_img-normal_img, 1, axis=2)
    d_right = np.linalg.norm(right_img-normal_img, 1, axis=2)
    d_up = np.linalg.norm(up_img-normal_img, 1, axis=2)
    d_down = np.linalg.norm(down_img-normal_img, 1, axis=2)
    a_img = np.stack([d_up,d_down,d_left,d_right], axis=2)
    dd_img = np.max(a_img, axis=2)
    dd = ((dd_img + sky*10000) > d_th).astype(np.float)
    return dd

def vis_image(normal_img, boxes, img_path, save_path):
    h, w = normal_img.shape
    new_im = Image.new('RGB', (w * 2, h))
    n_img = Image.fromarray(normal_img*255).convert('RGB')
    img = Image.open(img_path)
    img_draw = ImageDraw.Draw(img)
    n_draw = ImageDraw.Draw(n_img)
    #ipdb.set_trace()
    if boxes is not None:
        for box in boxes:
            x0, y0, x1, y1 = box
            bb = [x0, y0, x1, y0, x1, y1, x0, y1, x0, y0]
            img_draw.line(bb, fill=(0, 255, 0), width=3)
            n_draw.line(bb, fill=(0, 255, 0), width=3)
    new_im.paste(n_img, (0, 0))
    new_im.paste(img, (w, 0))
    new_im.save(save_path)

def vis_image_all(normal_dd, normal_img, boxes, img_path, save_path, blank=40):
    h, w = normal_dd.shape
    new_im = Image.new('RGB', (w * 5 + blank*4, h),color=(255, 255, 255))
    w += blank
    n_img = Image.fromarray(normal_dd*255).convert('RGB')
    normal_img=Image.fromarray(normal_img)
    img = Image.open(img_path)
    new_im.paste(img, (0, 0))
    new_im.paste(normal_img, (w, 0))
    new_im.paste(n_img, (w*2 ,0))
    img_draw = ImageDraw.Draw(img)
    n_draw = ImageDraw.Draw(n_img)
    #ipdb.set_trace()
    if boxes is not None:
        for box in boxes:
            x0, y0, x1, y1 = box
            bb = [x0, y0, x1, y0, x1, y1, x0, y1, x0, y0]
            img_draw.line(bb, fill=(0, 255, 0), width=3)
            n_draw.line(bb, fill=(0, 255, 0), width=3)
    new_im.paste(n_img, (w*3, 0))
    new_im.paste(img, (w*4, 0))
    new_im.save(save_path)

def vis_dd_img(dd_img, save_path, boxes):
    img = Image.fromarray(dd_img*255).convert('RGB')
    img_draw = ImageDraw.Draw(img)
    if boxes is not None:
        for box in boxes:
            x0, y0, x1, y1 = box
            bb = [x0, y0, x1, y0, x1, y1, x0, y1, x0, y0]
            img_draw.line(bb, fill=(0, 255, 0), width=3)
    img.save(save_path)

def main(isVis=False, reduceRatio=1):
    root = '../Data/SceneInfo'
    scene = "RealisticRendering" if len(sys.argv)<2 else sys.argv[1]
    sceneRoot = os.path.join(root, scene)
    img_dir = os.path.join(sceneRoot, 'Img')
    normal_dir  = os.path.join(sceneRoot, 'Normal')
    bbox_dir = os.path.join(sceneRoot, 'BoxProposals')
    vis_dir = os.path.join(sceneRoot, 'Vis')

    if not os.path.exists(bbox_dir):
        os.mkdir(bbox_dir)
    if not os.path.exists(vis_dir):
        os.mkdir(vis_dir)

    for img_id in tqdm.tqdm(range(50)):
        while True:
            img_name = os.path.join(img_dir, f'{img_id}.png')
            normal_name = os.path.join(normal_dir, f'{img_id}.png')
            vis_name = os.path.join(vis_dir, f'{img_id}.png')
            if not(os.path.isfile(img_name) and os.path.isfile(normal_name)):
                break

            normalimg = cv2.imread(normal_name)
            img = cv2.imread(img_name)
            normalimg = block_reduce(normalimg, (reduceRatio, reduceRatio, 1), np.mean).astype(np.float)
            normalimg = normalimg.repeat(reduceRatio, axis=0).repeat(reduceRatio, axis=1)
            
            normal_dd = generate_normal_deri_v2(normalimg, d_th=100)
            
            img_blurred = block_reduce(img, (reduceRatio, reduceRatio, 1), np.mean).astype(np.uint8)
            img_blurred = img_blurred.repeat(reduceRatio, axis=0).repeat(reduceRatio, axis=1)
            IMG = img.astype(np.float)
            AvgColor = np.mean(IMG, axis=2)
            AvgLightness = float(np.mean(AvgColor))
            normal_dd = (normal_dd + (AvgColor < min(12, AvgLightness)).astype(np.float)).astype(np.float)
            
            box_proposals = box_proposing(normal_dd, img, False, True, False)
            boxes = box_proposals
            gt_path =os.path.join(bbox_dir, f'{img_id}.txt')
            with open(gt_path,'w') as f:
                for b in boxes:
                    gt=','.join(map(str,b))+'\n'
                    f.write(gt)
            if isVis:
                vis_image_all(normal_dd, normal_dd, boxes, img_name, vis_name)
            break

if __name__ == "__main__":
    main(True, 2)