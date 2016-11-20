# some helper functions, for reading/writing slabfields and generating animations 

import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import matplotlib.animation as animation


width, height  = 1024,1024
slabs = np.ones((width,height))*4

def slabs_from_file(f):
    return np.loadtxt(f,skiprows=1)

def slabs_to_file(s,f):
    of = open(f, 'w+')
    n,m = s.shape
    of.write("%d %d\n" % (n,m))
    for i in range(n):
        for j in range(m):
            of.write("%d " % s[i,j])
        of.write("\n")
    of.close()


def load_sequence(tmpdir):
    files = sorted([f for f in os.listdir(tmpdir) if os.path.isfile(os.path.join(tmpdir,f))])

    min,max = 1000000,0
    seq = []
    for f in files:
        s = slabs_from_file(os.path.join(tmpdir,f))
        seq.append(s)
    return seq
    
def make_anim(seq, interval=50, cmap=plt.cm.coolwarm):
    fig = plt.figure(frameon=False, figsize=(5,5))
    ax = fig.add_subplot(1,1,1)
    ax.set_aspect("equal")
    ax.axis('off')

    min = np.min(seq)
    max = np.max(seq)
    
    def init():
        ax.imshow(seq[0],vmin=min, vmax=max+1,cmap=cmap)
        return ax,

    def update(j):
        ax.imshow(seq[j],vmin=min,vmax=max+1,cmap=cmap)
        return ax,
    
    anim = animation.FuncAnimation(fig, update, init_func = init, frames=len(seq), interval=interval)
    return anim

def make_video(tmpdir,vname):
    seq = load_sequence(tmpdir)
    plt.ioff()
    a = make_anim(seq,interval=100)
    video = a.to_html5_video()
    vfile = open(vname,"w")
    vfile.write(video)
    vfile.close()
    
