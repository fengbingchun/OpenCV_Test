import os
import sys
import cv2
from inspect import currentframe, getframeinfo
import argparse
import numpy as np

'''
Blog:
    https://blog.csdn.net/fengbingchun/article/details/131152103
    https://blog.csdn.net/fengbingchun/article/details/131493028
'''

def get_image_list(path, image_suffix):
    image_list = []
    for x in os.listdir(path):
        if x.endswith(image_suffix):
            image_list.append(path+"/"+x)

    return image_list

def get_image_name(image_name):
    pos = image_name.rfind("/")
    image_name = image_name[pos+1:]

    return image_name

def get_image_name2(image_name, image_suffix):
    name = get_image_name(image_name)
    #name = name[:len(image_suffix)-1] # sometimes, the fetched name is incorrect, for example: 10-1.JPG
    pos =name.rfind(".")

    return name[:pos]

def image_rotate_clockwise(image_list, degrees, result_path):
    print("image rotation ...")
    os.makedirs(result_path, exist_ok=True)

    if degrees == 90:
        rotate_code = cv2.ROTATE_90_CLOCKWISE
    elif degrees == 180:
        rotate_code = cv2.ROTATE_180
    elif degrees == 270:
        rotate_code = cv2.ROTATE_90_COUNTERCLOCKWISE
    else:
        raise Exception("Unsupported rotat degrees: {}, it only supports: clockwise 90, 180, 270; Error Line: {}".format(degrees, getframeinfo(currentframe()).lineno))

    for name in image_list:
        print(f"\t{name}")
        image_name = get_image_name(name)
        #print(f"image name:{image_name}"); sys.exit(1)

        mat = cv2.imread(name, -1)
        mat = cv2.rotate(mat, rotateCode=rotate_code)
        cv2.imwrite(result_path+"/"+image_name, mat)

def image_resize(image_list, dst_width, dst_height, result_path):
    print("image resize ...")
    os.makedirs(result_path, exist_ok=True)

    if dst_width == 0 or dst_height == 0:
        raise Exception("Image width or height cannot be 0: {}, {}; Error Line: {}".format(dst_width, dst_height, getframeinfo(currentframe()).lineno))

    mat = cv2.imread(image_list[0], -1)
    h, w, _ = mat.shape
    if h > dst_height and w > dst_width:
        interpolation = cv2.INTER_AREA
    else:
        interpolation = cv2.INTER_CUBIC

    for name in image_list:
        print(f"\t{name}")
        image_name = get_image_name(name)
        #print(f"image name:{image_name}"); sys.exit(1)

        mat = cv2.imread(name, -1)
        mat = cv2.resize(mat, (dst_width, dst_height), interpolation=interpolation)
        cv2.imwrite(result_path+"/"+image_name, mat)

def image_segment(image_list, image_suffix, path2, image_suffix2, degrees, dst_width, dst_height, result_path):
    print("image segment ...")
    os.makedirs(result_path, exist_ok=True)

    for name in image_list:
        print(f"\t{name}")
        image_name = get_image_name2(name, image_suffix)
        #print(f"image name:{image_name}"); sys.exit(1)
        name2 = path2 + "/" + image_name + image_suffix2
        print(f"\t{name2}")

        bgr = cv2.imread(name, -1)
        h1, w1, c1 = bgr.shape
        mask = cv2.imread(name2, -1)
        h2, w2 = mask.shape
        if c1 != 3:
            raise Exception("Unsupported number of image channels: {}, {}; Error Line: {}".format(c1, c2, getframeinfo(currentframe()).lineno))

        if h1 != h2 or w1 != w2: # rotate
            if h1 != w2 or w1 != h2:
                raise Exception("Inconsistent image size: {},{}:{},{}; Error Line: {}".format(h1, w1, h2, w2, getframeinfo(currentframe()).lineno))

            if degrees == 90:
                rotate_code = cv2.ROTATE_90_CLOCKWISE
            elif degrees == 180:
                rotate_code = cv2.ROTATE_180
            elif degrees == 270:
                rotate_code = cv2.ROTATE_90_COUNTERCLOCKWISE
            else:
                raise Exception("Unsupported rotat degrees: {}, it only supports: clockwise 90, 180, 270; Error Line: {}".format(degrees, getframeinfo(currentframe()).lineno))
            bgr = cv2.rotate(bgr, rotateCode=rotate_code)

        bgra = cv2.cvtColor(bgr, cv2.COLOR_BGR2BGRA)

        if dst_width != 0 or dst_height != 0: # resize
            if h1 > dst_height and w1 > dst_width:
                interpolation = cv2.INTER_AREA
            else:
                interpolation = cv2.INTER_CUBIC

            bgra = cv2.resize(bgra, (dst_width, dst_height), interpolation=interpolation)
            mask = cv2.resize(mask, (dst_width, dst_height), interpolation=interpolation)

        #h3, w3, c3 = bgra.shape
        #result = np.zeros((h3, w3, c3), dtype="uint8")
        result = cv2.bitwise_and(bgra, bgra, mask=mask)
        cv2.imwrite(result_path+"/"+image_name+".png", result)

def image_border(image_list, border_width, border_height, value, result_path):
    print("image border ...")
    os.makedirs(result_path, exist_ok=True)

    mat = cv2.imread(image_list[0], -1)
    h, w, _ = mat.shape

    top = border_height
    bottom = border_height
    left = border_width
    right = border_width

    for name in image_list:
        print(f"\t{name}")
        image_name = get_image_name(name)
        #print(f"image name:{image_name}"); sys.exit(1)

        mat = cv2.imread(name, -1)
        result = cv2.copyMakeBorder(mat, top, bottom, left, right, cv2.BORDER_CONSTANT, np.array(value))
        cv2.imwrite(result_path+"/"+image_name, result)

def parse_args():
    parser = argparse.ArgumentParser(description="image generic operations", add_help=True)

    parser.add_argument("--image_src_path", required=True, type=str, help="the path of the image to be operated, for example: ../../test_images")
    parser.add_argument("--image_src_path2", default="", type=str, help="the path of the image to be operated, for example: ../../test_images")
    parser.add_argument("--operation", required=True, type=str, choices=["rotate", "resize", "segment", "border"], help="specifies the operation to take on the image")
    parser.add_argument("--image_dst_path", required=True, type=str, help="the path where the resulting image is saved, for example: ../../test_images/result")

    parser.add_argument("--degrees", default=90, type=int, choices=[90, 180, 270], help="the degrees by which the image is rotated clockwise")

    parser.add_argument("--width", default=0, type=int, help="the width of the image after scaling")
    parser.add_argument("--height", default=0, type=int, help="the height of the image after scaling")

    parser.add_argument("--image_suffix", default=".png", type=str, help="the suffix of the processed image")
    parser.add_argument("--image_suffix2", default=".png", type=str, help="the suffix of the processed image")

    parser.add_argument("--value", default="", type=str, help="pixel value, order: b g r a, the length can be 1 2 3 4, for example: 128 128 255 255")
    
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    args = parse_args()
    if args.value != "":
        value = []
        for v in args.value.split():
            value.append(int(v))

    image_list = get_image_list(args.image_src_path, args.image_suffix)
    if len(image_list) == 0:
        print(f"Warning: no image, directory: {args.image_src_path}")
        sys.exit(1)

    if args.operation == "rotate":
        image_rotate_clockwise(image_list, args.degrees, args.image_dst_path)

    if args.operation == "resize":
        image_resize(image_list, args.width, args.height, args.image_dst_path)

    if args.operation == "segment":
        image_segment(image_list, args.image_suffix, args.image_src_path2, args.image_suffix2, args.degrees, args.width, args.height, args.image_dst_path)

    if args.operation == "border":
        image_border(image_list, args.width, args.height, value, args.image_dst_path) 

    print("test finish")
