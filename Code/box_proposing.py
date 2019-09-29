import glob, os, tqdm, time
import numpy as np
import cv2 as cv
from PIL import Image
import random
import pdb

# hyper parameters
min_box_size = 48
minimum_width = 64 
min_aspect_ratio = 0.4
max_aspect_ratio = 6
anchor_x_step = 12
anchor_y_step = 12
overlap_threshold = 1

def read(file):
    """
    ndarray (h, w), 0/1, int32
    """
    img = (cv.imread(file)[:, :, 0] > 0).astype('float32')
    return img

def box_fit(normal_map, box_x, box_y, shrink=2):
    h, w = normal_map.shape

    x_left_min = 0
    x_left_max = box_x - min_box_size // 2
    x_right_min = box_x + min_box_size // 2
    x_right_max = w

    y_down_min = 0
    y_down_max = box_y - min_box_size // 4
    y_up_min = box_y + min_box_size // 4
    y_up_max = h

    if (x_left_max < 0) or (x_right_min >= w) or (y_down_max < 0) or (y_up_min >= h) or (np.sum(normal_map[y_down_max:y_up_min,
                                    x_left_max:x_right_min]) > overlap_threshold):
        return -1

    enlargeable_flag = True
    direction = [1, 2, 3, 4]# up, down, left, right

    while enlargeable_flag:
        random.shuffle(direction)
        enlargeable_flag = False
        cur_width = x_right_min - x_left_max
        for d in direction:
            if d==4: # up
                if cur_width < minimum_width: continue
                if y_up_max-y_up_min < 2: continue
                mid = (y_up_min+y_up_max) // 2
                if np.sum(normal_map[y_down_max:mid,
                                    x_left_max:x_right_min]) > overlap_threshold: 
                    y_up_max = mid
                else:
                    y_up_min = mid
                enlargeable_flag = True
                break
            elif d == 3: # down
                if cur_width < minimum_width: continue
                if y_down_max-y_down_min < 2: continue
                mid = (y_down_max+y_down_min) // 2
                if np.sum(normal_map[mid:y_up_min,
                                    x_left_max:x_right_min]) > overlap_threshold: 
                    y_down_min = mid
                else:
                    y_down_max = mid
                enlargeable_flag = True
                break
            elif d == 2: # left
                if x_left_max-x_left_min < 2: continue
                mid = (x_left_max+x_left_min) // 2
                if np.sum(normal_map[y_down_max:y_up_min,
                                    mid:x_right_min]) > overlap_threshold: 
                    x_left_min = mid
                else:
                    x_left_max = mid
                enlargeable_flag = True
                break
            elif d == 1: # right
                if x_right_max-x_right_min < 2: continue
                mid = (x_right_max+x_right_min) // 2
                if np.sum(normal_map[y_down_max:y_up_min,
                                    x_left_max:mid]) > overlap_threshold: 
                    x_right_max = mid
                else:
                    x_right_min = mid
                enlargeable_flag = True
                break

        if x_right_min - x_left_max > 300 and random.random() < 0.7: break
        if abs(y_down_max - y_up_min) > 150 and random.random() < 0.7: break

    return x_left_max+shrink, y_down_max+shrink, x_right_min-shrink, y_up_min-shrink

def box_proposing(NORM, IMG, 
                  vis=True, 
                  filter=True,
                  print_message=True):
    """
    normal_map: HxW binary maps
    vis: visualize and save the bbox as files
    filter: apply IoU based filters
    """
    normal_map = NORM.copy()
    
    H, W = normal_map.shape
    boxes = []
    hs = [h for h in range(6, H-6, anchor_y_step)]
    random.shuffle(hs)
    ws = [w for w in range(6, W-6, anchor_x_step)]
    random.shuffle(ws)
    for h in hs[:]: 
        for w in ws[:]:
            new_box = box_fit(normal_map, w+random.randint(-6,6), h+random.randint(-6,6))
            if new_box != -1:
                UL_x, UL_y, BR_x, BR_y = new_box
                
                boxes.append(new_box)

    if print_message:
        print(f'Found {len(boxes)} box proposals.')

    if filter:
        # filtering
        #ipdb.set_trace()
        filtered_boxes = []
        random.shuffle(boxes)
        for i, (x_left_max, y_down_max, x_right_min, y_up_min) in tqdm.tqdm(enumerate(boxes)) if print_message else enumerate(boxes):
            #ipdb.set_trace()
            aspect_ratio = (x_right_min-x_left_max)/(y_up_min-y_down_max)
            if aspect_ratio>max_aspect_ratio or aspect_ratio<min_aspect_ratio:
                continue
            flag = True
            for j, (x_left_max_, y_down_max_, x_right_min_, y_up_min_) in enumerate(filtered_boxes):
                if (x_left_max > x_right_min_) or (x_left_max_ > x_right_min): continue
                if (y_up_min < y_down_max_) or (y_up_min_ < y_down_max): continue
                canvas_1 = np.zeros(shape=(H, W), dtype=np.float32)
                canvas_2 = np.zeros(shape=(H, W), dtype=np.float32)
                canvas_1[y_down_max:y_up_min, x_left_max:x_right_min] = 1
                canvas_2[y_down_max_:y_up_min_, x_left_max_:x_right_min_] = 1
                if np.sum(canvas_1 * canvas_2) / np.sum(((canvas_1 + canvas_2) > 0).astype('float32')) > 0:
                    flag = False
                    break
            if flag:
                filtered_boxes.append((x_left_max, y_down_max, x_right_min, y_up_min))
        if print_message:
            print(f'{len(filtered_boxes)} proposals after filtering.')
    else:
        filtered_boxes = boxes
        random.shuffle(filtered_boxes)
        filtered_boxes = filtered_boxes[:30]

    # visualization
    if vis:
        os.makedirs('./samples', exist_ok=True)
        vis_map_canvas = np.stack([normal_map, normal_map, normal_map], axis=2)
        for i, (x_left_max, x_right_min, y_down_max, y_up_min) in enumerate(filtered_boxes):
            vis_map = vis_map_canvas + 0.
            vis_map[y_down_max:y_up_min, x_left_max-3:x_left_max+3, :2] += 1
            vis_map[y_down_max:y_up_min, x_right_min-3:x_right_min+3, :2] += 1
            vis_map[y_down_max-3:y_down_max+3, x_left_max:x_right_min, :2] += 1
            vis_map[y_up_min-3:y_up_min+3, x_left_max:x_right_min, :2] += 1
            cv.imwrite(f'samples/output-{i}.png', (vis_map*255).astype('uint8'))

    return filtered_boxes


if __name__ == "__main__":

    normal_map = read(input('filename='))

    # timer
    t_start = time.time()

    # usage:
    # from bo_proposing import box_proposing
    box_proposals = box_proposing(normal_map, True, True, False)

    # timer
    t_end = time.time()
    print(f'Takes {t_end-t_start} sec.')