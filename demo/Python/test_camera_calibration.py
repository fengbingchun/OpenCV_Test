import cv2
import numpy as np
import glob
from sys import platform

# Blog: https://blog.csdn.net/fengbingchun/article/details/130174805

def get_image_name(path):
	if platform == "win32":
		pos = path.rfind("\\")
	elif platform == "linux":
		pos = path.rfind("/")
	else:
		raise Exception(f"Error: Unsupported platform: {platform}")
	
	return path[pos+1:len(path)-4]

def camera_calibration(checkerboard_size, path):
	images = glob.glob(path)
	if len(images) == 0:
		raise Exception(f"Error: the requested images were not found: {path}")

	if platform == "win32":
		pos = images[0].rfind("\\")
	elif platform == "linux":
		pos = images[0].rfind("/")
	else:
		raise Exception(f"Error: Unsupported platform: {platform}")

	path_result = images[0][0:pos+1]

	# the world coordinates for 3D points
	pts_3d_world_coord = np.zeros((1, checkerboard_size[0] * checkerboard_size[1], 3), np.float32)
	pts_3d_world_coord[0,:,:2] = np.mgrid[0:checkerboard_size[0], 0:checkerboard_size[1]].T.reshape(-1, 2)
	#print(f"pts_3d_world_coord: {pts_3d_world_coord}")

	# store vectors of 3D points for each checkerboard image
	pts_3d = []
	# store vectors of 2D points for each checkerboard image
	pts_2d = []

	for name in images:
		frame = cv2.imread(name)
		if frame is None:
			raise Exception(f"Error: fail to read image: {frame}")

		gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
		ret, pts_corners = cv2.findChessboardCorners(gray, checkerboard_size, cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_FAST_CHECK + cv2.CALIB_CB_NORMALIZE_IMAGE)
		if ret != True:
			raise Exception(f"Error: fail to find chess board corners: {name}")
		
		criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
		# refining pixel coordinates for given 2d points
		pts_corners = cv2.cornerSubPix(gray, pts_corners, (11,11), (-1,-1), criteria)

		# displaying the detected corner points on the checker board
		frame = cv2.drawChessboardCorners(frame, checkerboard_size, pts_corners, ret)

		pts_3d.append(pts_3d_world_coord)
		pts_2d.append(pts_corners)

		#cv2.imshow("Image", frame)
		#cv2.waitKey(0)
		cv2.imwrite(path_result + "result_" + get_image_name(name) + ".png", frame)

	ret, camera_matrix, dist_coeffs, R, t = cv2.calibrateCamera(pts_3d, pts_2d, gray.shape[::-1], None, None)
	print(f"Camera matrix:\n{camera_matrix}")
	print(f"dist_coeffs:\n{dist_coeffs}")
	print(f"R:\n{R}")
	print(f"t:\n{t}")

if __name__ == "__main__":
	# the dimensions of checkerboard
	CHECKERBOARD = (11, 13)
	# images path
	path = "../../test_images/camera_calibration/*.jpg"
	camera_calibration(CHECKERBOARD, path)

	print("test finish")
