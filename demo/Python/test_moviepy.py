import moviepy.editor as mp
import cv2

# Blog: https://blog.csdn.net/fengbingchun/article/details/131026198

def add_text(filename, text, font, fontsize, color, pos, result):
	clip = mp.VideoFileClip(filename)
	#clip = clip.subclip(10, 20) # 仅clip 10到20秒之间的视频
	#print(f"duration: {clip.duration} seconds") # 视频文件clip的持续时间
 
	txt_clip = (mp.TextClip(text, font=font, fontsize=fontsize, color=color)
					.set_position(pos)
					.set_duration(clip.duration))
	#print(f"color list:\n {mp.TextClip.list('color')}") # 支持的color列表
	#print(f"font list:\n {mp.TextClip.list('font')}") # 支持的font列表

	final = mp.CompositeVideoClip([clip, txt_clip]) # 将文本叠加在视频上
	final.write_videofile(result, fps=clip.fps, codec="libx264")

def add_image(filename, image, pos, width, height, result):
	clip = mp.VideoFileClip(filename)
	#clip = clip.subclip(10, 20)

	mat = cv2.imread(image)
	mat = cv2.resize(mat, (width, height))

	#img_clip = (mp.ImageClip(image)
	img_clip = (mp.ImageClip(mat)
					.set_position(pos)
					.set_duration(clip.duration)
					.set_opacity(0.5)) # 设置不透明度/透明度级别

	final = mp.CompositeVideoClip([clip, img_clip]) # 将图像叠加在视频上
	final.write_videofile(result, fps=clip.fps, codec="libx264")

if __name__ == "__main__":
	filename = "../../test_images/123.mp4"
	pos = (100, 100) # 左上角坐标(x, y)
	#pos = ("left", "center") # center, right, left, bottom, top

	text = "北京 fengbingchun"
	font = "Simhei" # "华文彩云" ...
	fontsize = 75
	color = "red" # green, black, blue, red ...
	result = "../../test_images/result_text.mp4"
	add_text(filename, text, font, fontsize, color, pos, result)

	image = "../../test_images/1.jpg"
	result = "../../test_images/result_image.mp4"
	width = 128
	height = 64
	add_image(filename, image, pos, width, height, result)

	print("test finish")
