#!/usr/bin/env python

from gimpfu import *

def python_generate_font():
    # Actual plug-in code will go here
    print "Eccolo !!!"
    return

register(
  "python_fu_generate_font",
	"Does something",
	"Does something terribly useful",
	"Your name",
	"Your name",
	"2009",
	"Generate font...",
	"",
	[
	],
	[],
	python_generate_font,
  menu="<Image>/File/Create")

main()

