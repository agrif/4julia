#!/usr/bin/python

import sys, os, os.path

infile = sys.argv[1]
outdir = sys.argv[2]

subst = "" # original file
varvals = {} # var : val
animlist = [] # (frames, [(varname, new), ...])
frame = 0

f = open(infile, "r")

while 1:
    s = f.readline()
    if s == '':
        sys.exit(1)
    if s.startswith('%%'):
        break
    subst += s

while 1:
    s = f.readline()
    if s == '':
        sys.exit(1)
    if s.startswith('%%'):
        break
    name, val = s.split(' ', 1)
    val = float(val)
    varvals[name] = val

while 1:
    s = f.readline()
    if s == '':
        break
    frames, s = s.split(' ', 1)
    s = s.split(' ')
    frames = int(frames)
    anims = []
    while len(s) > 0:
        name = s[0]
        s.remove(name)
        val = s[0]
        s.remove(val)
        val = float(val)
        anims.append((name, val))
    animlist.append((frames, anims))

#print subst.__repr__()
#print varvals
#print animlist

outdir = outdir.rstrip("/")
try:
    os.mkdir(outdir)
except OSError:
    # probably fine? maybe?
    pass

for anim in animlist:
    startframe = frame
    animsteps = {}
    for i in anim[1]:
        animsteps[i[0]] = (i[1] - varvals[i[0]]) / anim[0]
    while frame < startframe + anim[0]:
        for i in animsteps.keys():
            varvals[i] += animsteps[i]
        # output
        news = subst
        for j in varvals.keys():
            news = news.replace("${%s}" % (j.upper(),), str(varvals[i]))
        news = news.replace("${FRAME}", "%06i" % (frame,))
        news = news.replace("${OUT}", outdir)
        
        print news
        
        frame += 1

#os.system("rm -f %s/out.avi" % (outdir,))
#os.system("ffmpeg -r 30 -i %s/%%06d.png %s/out.avi" % (outdir, outdir))

