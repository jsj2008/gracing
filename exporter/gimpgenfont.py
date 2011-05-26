#!/usr/bin/env python

from gimpfu import *
import math

num_col=16

def get_closer_power_of_two(i):
  if i<=2:
    return 2
  if i<=4:
    return 4
  if i<=8:
    return 8
  if i<=16:
    return 16
  if i<=32:
    return 32
  if i<=64:
    return 64
  if i<=128:
    return 128
  if i<=256:
    return 256
  if i<=1024:
    return 1024
  if i<=2048:
    return 2048
  return i

def calc_image_size(char_begin,char_end,fontname, font_size):    
    img_width=0
    img_height=0
    rows = int( math.ceil((char_end - char_begin) / num_col) )
    c=char_begin
    for row in range(0,rows):
      row_width=0
      row_height=0
      for col in range(0,num_col):
        if c > char_end:
          break
        string= '%c' % c
        width, height, asc, desc =pdb.gimp_text_get_extents_fontname(string,font_size,0,fontname)
        row_width = row_width + width
        if height > row_height:
          row_height = height
        c=c+1
      img_height = img_height + row_height
      if row_width > img_width:
        img_width = row_width

    img_width = get_closer_power_of_two(img_width)
    img_height = get_closer_power_of_two(img_height)
    return img_width, img_height


def python_generate_font(font,font_size,filename,color):
    char_begin = ord('!')
    char_end = ord('~')+1
    num_chars = char_end - char_begin

    if font == "":
      return

    img_width, img_height = calc_image_size(char_begin,char_end,font,font_size)

    if img_width > 1024:
      return

    if img_height > 1024:
      return
   
    img=gimp.Image(img_width, img_height, RGB)

    img.disable_undo()

    gimp.set_foreground(color)

    disp = gimp.Display(img)

    gimp.displays_flush()

    elements=[ ]
    x_pos=0
    y_pos=0
    max_height=0
    cnt=0
    for i in range(char_begin, char_end):
      string = '%c' % i
      offset = i - char_begin

      width, height, asc, desc = pdb.gimp_text_get_extents_fontname(string,font_size,0,font)

      if max_height < height:
        max_height = height

      text_layer = pdb.gimp_text_fontname(img, None, x_pos, y_pos, string, -1, False, font_size, PIXELS, font)
      gimp.progress_update(float(offset) / float(num_chars))

      rect = "%d,%d,%d,%d" % (x_pos, y_pos, x_pos + width, y_pos + height)

      el = {
        "c": string,
        "r": rect,
        "i": 0
      }

      elements.append(el)
      
      if cnt and cnt % num_col == 0:
        x_pos=0
        y_pos = y_pos + max_height
        max_height = 0
      else:
        x_pos = x_pos + width 

      cnt = cnt + 1
    
    img.enable_undo()
    
    filename = "/tmp/digits.xml"
    if filename != "":
      file=open(filename,"w")
      for el in elements:
        line = "<c c=\"%s\" r=\"%s\" i=\"%d\"/>\n" % (el["c"], el["r"], el["i"])
        print line
        file.write(line)
      file.close

    return

register(
  "python_fu_generate_font",
	"Generate bitmap font",
	"Generate bitmap font",
	"Your name",
	"Your name",
	"2009",
	"Generate font...",
	"",
	[
  (PF_FONT, "font", "Font to use", ""),
  (PF_INT, "fontsize", "Font size", 32),
  (PF_FILE, "filename", "Filename", ""),
  (PF_COLOR, "color", "Color", (0,0,0))
	],
	[],
	python_generate_font,
  menu="<Image>/File/Create")

main()

