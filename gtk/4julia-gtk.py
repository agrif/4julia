#!/usr/bin/env python

import sys, subprocess, os, time
try:
 	import pygtk
  	pygtk.require("2.0")
except:
  	pass
try:
	import gtk
  	import gtk.glade
except:
	sys.exit(1)

class JuliaGtk:
	def __init__(self):
		#Set the Glade file
		self.gladefile = "4julia-gtk.glade"  
	        self.wTree = gtk.glade.XML(self.gladefile) 
		
		self.wTree.get_widget("light1_color").set_color(gtk.gdk.Color(*map(int, (0.3 * 65535,) * 3)))
		
		#Create our dictionay and connect it
		dic = { "update_render" : self.render,
			"button_save" : self.save,
			"button_load" : self.load,
			"window_destroy" : gtk.main_quit }
		self.wTree.signal_autoconnect(dic)
		
		self.savefile = None
		
		self.julia = subprocess.Popen(sys.argv[1:], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		
		self.render()
	
	def send(self, s):
		if self.savefile == None:
			print os.read(self.julia.stdout.fileno(), 256),
			print s
			self.julia.stdin.write(s + "\n")
		else:
			self.savefile.write(s + "\n")
			print "saving > ", s
	
	def render(self, *args, **kwargs):
		self.send("set-constant %f %f %f %f" % tuple(map(lambda x: self.wTree.get_widget("constant_" + x).get_value(), ["r", "i", "j", "k"])))
		self.send("set-slice %f %f %f %f %f" % tuple(map(lambda x: self.wTree.get_widget("slice_" + x).get_value(), ["x", "y", "z", "w", "d"])))
		self.send("set-iterations %i" % (self.wTree.get_widget("iterations").get_value(),))
		self.send("set-camera %f %f %f %f" % tuple(map(lambda x: self.wTree.get_widget("camera_" + x).get_value(), ["radius", "theta", "phi", "fov"])))
		self.send("set-epsilon %f %f" % tuple(map(lambda x: self.wTree.get_widget("epsilon_" + x).get_value(), ["ratio", "minimum"])))
		self.send("set-size %i" % (self.wTree.get_widget("raytracer_size").get_value(),))
		self.send("set-supersampling %i" % (self.wTree.get_widget("raytracer_supersampling").get_value(),))
		self.send("clear-lights")
		for i in ["light1_", "light2_", "light3_"]:
			theta = self.wTree.get_widget(i+"theta").get_value()
			phi = self.wTree.get_widget(i+"phi").get_value()
			color = self.wTree.get_widget(i+"color").get_color()
			r = color.red / 65535.0
			g = color.green / 65535.0
			b = color.blue / 65535.0
			self.send("add-light %f %f %f %f %f" % (theta, phi, r, g, b))
		self.send("start-view")
		time.sleep(0.01)
	
	def set_values(self, n, w, v):
		for i in range(len(w)):
			self.wTree.get_widget(n + w[i]).set_value(float(v[i]))
	
	def load(self, *args, **kwargs):
		chooser = gtk.FileChooserDialog(title="Open..",action=gtk.FILE_CHOOSER_ACTION_OPEN,
                	buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
                chooser.set_default_response(gtk.RESPONSE_OK)
		filt = gtk.FileFilter()
		filt.set_name("All Files")
		filt.add_pattern("*")
		chooser.add_filter(filt)
		
		response = chooser.run()
		if response == gtk.RESPONSE_CANCEL:
			chooser.destroy()
			return
		fname = chooser.get_filename()
		chooser.destroy()
		f = open(fname, "r")
		light = 2
		for i in f.readlines():
			i = i.strip()
			c = i.split(" ")
			args = c[1:]
			c = c[0]
			if c == "set-constant":
				self.set_values("constant_", ["r", "i", "j", "k"], args)
			elif c == "set-slice":
				self.set_values("slice_", ["x", "y", "z", "w", "d"], args)
			elif c == "set-iterations":
				self.wTree.get_widget("iterations").set_value(int(args[0]))
			elif c == "set-camera":
				self.set_values("camera_", ["radius", "theta", "phi", "fov"], args)
			elif c == "set-epsilon":
				self.set_values("epsilon_", ["ratio", "minimum"], args)
			elif c == "set-size":
				self.wTree.get_widget("raytracer_size").set_value(int(args[0]))
			elif c == "set-supersampling":
				self.wTree.get_widget("raytracer_supersampling").set_value(int(args[0]))
			elif c == "clear-lights":
				light = 1
			elif c == "add-light":
				if not light <= 3:
					print "read ! > ", i
					continue;
				n = "light%i_" % (light,)
				self.set_values(n, ["theta", "phi"], args[:2])
				clr = gtk.gdk.Color(*map(lambda x: int(float(x) * 65535), args[2:]))
				self.wTree.get_widget(n + "color").set_color(clr)
				light += 1
			else:
				print "read ? > ", i
				continue
			print "read   > ", i
		self.render()
	
	def save(self, *args, **kwargs):
		chooser = gtk.FileChooserDialog(title="Save..",action=gtk.FILE_CHOOSER_ACTION_SAVE,
                	buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_SAVE,gtk.RESPONSE_OK))
                chooser.set_default_response(gtk.RESPONSE_OK)
		filt = gtk.FileFilter()
		filt.set_name("All Files")
		filt.add_pattern("*")
		chooser.add_filter(filt)
		
		response = chooser.run()
		if response == gtk.RESPONSE_CANCEL:
			chooser.destroy()
			return
		fname = chooser.get_filename()
		chooser.destroy()
		self.savefile = open(fname, "w")
		self.render()
		self.savefile.close()
		self.savefile = None

if __name__ == "__main__":
	jg = JuliaGtk()
	gtk.main()

