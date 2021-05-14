#!/usr/bin/env python3
from PIL import Image
import PIL.ImageOps 
import numpy as np
import os
import sys
import zipfile
import shutil
import glob
import re

if len(sys.argv)!=2:
  print("Error, give one .cbz as argument")
  sys.exit(1)

cbz_name = sys.argv[1]
print('Working on', cbz_name)

shutil.rmtree("tmp", True)
os.mkdir("tmp")

with zipfile.ZipFile(cbz_name, 'r') as zip_ref:
    zip_ref.extractall("tmp")

all_img_path=glob.glob('tmp/*/*.jpg')
all_img_path.sort(key=lambda f: int(re.sub('\D', '', os.path.basename(f))))
print('Images found:',len(all_img_path))

os.mkdir("tmp/out")

def input2outputFileName(path, isUp):
    a = re.match('^(.*-)(\d+)(_.*).jpg$',os.path.basename(path))
    fixed = f'{a.group(1)}{int(a.group(2)):04}{a.group(3)}'
    if isUp:
        return fixed+"_A.jpg"
    else:
        return fixed+"_B.jpg"

all_jpg = None
all_img = None
all_v_avg = None
all_v_sum1 = None
all_v_sum2 = None
all_corr_v = None
all_h_avg = None
all_h_sum1 = None
all_h_sum2 = None
all_corr_h = None
all_best_m = None
all_best_b = None
all_best_t = None
all_best_l = None
all_best_r = None
all_marginU_w = None
all_marginU_h = None
all_marginL_w = None
all_marginL_h = None



blank_sz = 15
blank = np.ones(blank_sz)/blank_sz
half_blank = np.ones(blank_sz//2)
epaper_prop = 157/118 #4/3
overlap = 0.01 #relative to page height


def getBestMid():
    lines_i = np.arange(len(all_v_avg))
    corr = all_corr_v
    sz = len(corr)
    all_scores = []
    min_score = 10000
    min_score_index = 0
    for j in range(sz):
        score = np.abs(np.square(5*(j-sz/2)/sz)) + (1-corr[j])
        all_scores.append(score)
        if score < min_score:
            min_score = score
            min_score_index = j
    return min_score_index

def getBestBottom():
    lines_i = np.arange(len(all_v_avg))
    corr = all_corr_v
    v_sum2 = all_v_sum2
    sz = len(corr)
    all_scores = []
    min_score = 10000
    min_score_index = 0
    for j in range(sz):
        score = j/sz + (1-corr[j]) + 20*v_sum2[j]/sz/256
        all_scores.append(score)
        if score < min_score:
            min_score = score
            min_score_index = j
    return min_score_index

def getBestTop():
    lines_i = np.arange(len(all_v_avg))
    corr = all_corr_v
    v_sum1 = all_v_sum1
    sz = len(corr)
    all_scores = []
    min_score = 10000
    min_score_index = 0
    for j in range(sz):
        score = (sz-j)/sz + (1-corr[j]) + 20*v_sum1[j]/sz/256
        all_scores.append(score)
        if score < min_score:
            min_score = score
            min_score_index = j
    return min_score_index

def getBestSides():
    corr = all_corr_h
    h_sum1 = all_h_sum1
    h_sum2 = all_h_sum2
    sz = len(corr)
    all_scores_l = []
    min_score_l = 10000
    min_score_l_index = 0
    all_scores_r = []
    min_score_r = 10000
    min_score_r_index = 0
    for j in range(sz):
        score_l = (sz-j)/sz + (1-corr[j]) + 100*h_sum1[j]/sz/256
        all_scores_l.append(score_l)
        if score_l < min_score_l:
            min_score_l = score_l
            min_score_l_index = j
    for j in range(sz):
        score_r = j/sz + (1-corr[j]) + (1-corr[j]) + 100*h_sum2[j]/sz/256
        all_scores_r.append(score_r)
        if score_r < min_score_r:
            min_score_r = score_r
            min_score_r_index = j
    return min_score_l_index,min_score_r_index

print("|",end='')
for i in range(len(all_img_path)):
  if i%10==0:
    print(" ",end='')
print("|")

print("|",end='')
with zipfile.ZipFile(cbz_name+"_A5.cbz", mode='w') as zf:
  for i in range(len(all_img_path)):
    #print(all_img_path[i])
    if i%10==0:
      print("X",end='', flush=True)
    image = Image.open(all_img_path[i]).convert('L')
    all_jpg=image
    all_img=np.array(image)
    image_inv = PIL.ImageOps.invert(image)
    img_inv = np.array(image_inv)
    all_v_avg=np.average(img_inv, axis=1)
    all_v_sum1=np.cumsum(all_v_avg)
    all_v_sum2=np.flip(np.cumsum(np.flip(all_v_avg)))
    all_h_avg=np.average(img_inv, axis=0)
    all_h_sum1=np.cumsum(all_h_avg)
    all_h_sum2=np.flip(np.cumsum(np.flip(all_h_avg)))

    all_corr_v=np.concatenate( (half_blank,
                      (np.correlate(blank,(np.flip(all_v_avg)<10))>0.9),
                      half_blank) )
    all_corr_h=np.concatenate( (half_blank,
                      (np.correlate(blank,(np.flip(all_h_avg)<10))>0.9),
                      half_blank) )
    
    height = len(all_v_avg)
    width = len(all_h_avg)
    all_best_m=getBestMid()
    all_best_t=getBestTop() - overlap*height
    all_best_b=getBestBottom()
    l,r = getBestSides()
    all_best_l=l - overlap*height
    all_best_r=r + overlap*height
    if all_best_r >= width:
        all_best_r = width - 1
    if all_best_l < 0:
        all_best_l = 0
    if all_best_t < 0:
        all_best_t = 0
    if all_best_b >= height:
        all_best_b = height - 1
    for is_up in (True, False):
        margin_w = 0
        margin_h = 0
        selection_w = all_best_r-all_best_l+2*margin_w
        if is_up:
            selection_h = all_best_m+overlap*height-all_best_t+margin_h
        else:
            selection_h = all_best_b-all_best_m+overlap*height+margin_h
        selection_prop = selection_w / selection_h
        #print("Before adaptation:",selection_prop, epaper_prop)
        if selection_prop < epaper_prop:
            margin_w = (epaper_prop-selection_prop)*selection_h/2
        if selection_prop > epaper_prop:
            margin_h = selection_h*(selection_prop/epaper_prop-1)

        selection_w_test = all_best_r-all_best_l+2*margin_w
        if is_up:
            selection_h_test = all_best_m+overlap*height-all_best_t+margin_h
        else:
            selection_h_test = all_best_b-all_best_m+overlap*height+margin_h
        selection_prop_test = selection_w_test / selection_h_test
        #print("After adaptation:",selection_prop_test, epaper_prop, margin_w, margin_h)
        if is_up:
            all_marginU_w=margin_w
            all_marginU_h=margin_h
        else:
            all_marginL_w=margin_w
            all_marginL_h=margin_h
    
    height = len(all_v_avg)
    cut_l = all_best_l-all_marginU_w
    cut_r = all_best_r+all_marginU_w
    cut_t = all_best_t
    cut_m = all_best_m+all_marginU_h+overlap*height
    if cut_r >= width:
        cut_r = width - 1
    if cut_l < 0:
        cut_l = 0
    if cut_t < 0:
        cut_t = 0
    #print(all_img_path[i],"=>",input2outputFileName(all_img_path[i],True))
    if (cut_m>cut_t)and(cut_r>cut_l):
      crop = all_jpg.crop((cut_l, cut_t, cut_r, cut_m))
      crop = crop.rotate(90, expand=True)
      crop.save("tmp/out/"+input2outputFileName(all_img_path[i],True))
      zf.write("tmp/out/"+input2outputFileName(all_img_path[i],True),input2outputFileName(all_img_path[i],True))
      
    cut_l = all_best_l-all_marginL_w
    cut_r = all_best_r+all_marginL_w
    cut_m = all_best_m-all_marginL_h-overlap*height
    cut_b = all_best_b
    if cut_r >= width:
        cut_r = width - 1
    if cut_l < 0:
        cut_l = 0
    if cut_b >= height:
        cut_b = height - 1
    #print(all_img_path[i],"=>",input2outputFileName(all_img_path[i],False))
    if (cut_b>cut_m)and(cut_r>cut_l):
      crop = all_jpg.crop((cut_l, cut_m, cut_r, cut_b))
      crop = crop.rotate(90, expand=True)
      crop.save("tmp/out/"+input2outputFileName(all_img_path[i],False))
      zf.write("tmp/out/"+input2outputFileName(all_img_path[i],False),input2outputFileName(all_img_path[i],False))

print("|")



print("Clean")
shutil.rmtree("tmp", True)

