import math
import numpy as np

START = 'start'
END = 'end'
W = 'w'

def dist(a, b):
    #returns distance between any two points a and b

    return math.pow((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2, 0.5)

def totaldist(p, i):
    #returns sum of distances between any point p and 'i'th object and
    # 'i'th object and 'i'th endpoint

    return dist(p, o[i]) + dist(o[i], e[i])

def binary(n, nbits):
    #returns the indices of occurences of 0's and 1's in a binary number
    #in two separate lists
    
    b = bin(n)[2:]
    b = '0'*(nbits-len(b))+b
    ones = []
    zeros = []
    for i in range(len(b)):
        if int(b[i])==0:
            zeros.append(i)
        else:
            ones.append(i)
    return zeros, ones

class edge(object):
    #class to define an edge. This is used to create a dictionary
    def __init__(self, start, end, w):
        self.start = start
        self.end = end
        self.w = w

def grid_to_edges(grid):
    #extracts edges from the grid to help form a tree
    edges = []
    l = grid.shape[0]
    for i in range(1,l):
        for j in range(i):
            edges.append({START:i, END:j,W:grid[i,j]})
    return edges

def is_cycle(vs):
    check = False
    nv = len(vs.keys())
    count = 0
    i = 0
    while vs[i] != i:
        i = vs[i]
        count+=1
        if count>nv:
            check = True
            break

    return check

def traverse(p2c, root, visited=None, overallvisited = None):
    # this function will take the list of all parent nodesand its children
    # in the tree. root is the first element of the tree/subtree visited when
    # finding a route. Given a tree it is returning the order of nodes that
    # have to be visited. It gives the path for each robot. 
    if overallvisited is None:
        overallvisited = []
    if visited is None or len(visited)==0:
        visited=[]
        if root not in overallvisited:
            overallvisited.append(root)
    if len(overallvisited) == len(p2c.keys()):
        return overallvisited
    if len(visited) == len(p2c[root]):
        return overallvisited
    for v in p2c[root]:
        if v not in visited:
            visited.append(v)
            overallvisited.append(v)
            overallvisited = traverse(p2c, v, [], overallvisited)

    return overallvisited

def preorder(vs, root):
    #This is a method of traversal of the tree. p2c is a dictionary that contains
    #all the children nodes of a given parent node
    vertices = vs.keys()
    p2c = dict((v, []) for v in vertices)
    parents = []
    children = []
    for v in vs.keys():
        if vs[v]!=v:
            p2c[vs[v]].append(v)
    return traverse(p2c,root)

def kruskal(edges, vertices, root):
    #kruskal is creating a tree, given a grid
    edges = sorted(edges, key=lambda x: x[W])
    es = []
    vs = dict((i,i) for i in vertices)
    for i in range(len(edges)):
        tempvs = vs.copy()
        tempvs[edges[i][END]] = edges[i][START]
        if not is_cycle(tempvs):
            es.append(edges[i].copy())
            if edges[i][END] != root:
                vs[edges[i][END]] = edges[i][START]
            else:
                vs[edges[i][START]] = edges[i][END]


    return preorder(vs,root)

def cost(grid, vertices):
    sum = 0
    for i in range(len(vertices)-1):
        sum+=grid[vertices[i]][vertices[i+1]]
    return sum

def distance_angle(r):
    a_d = []
    scale = 20.3
    for i in range(len(r)-1):
        angle = math.atan2((r[i+1][0]-r[i][0]),(r[i+1][1]-r[i][1]))*180/3.1415
        a_d.append(angle)
        dist = ((r[i+1][0]-r[i][0])**2 + (r[i+1][1]-r[i][1])**2)**0.5
        a_d.append(dist*scale)
    for i in range(2,len(a_d),2):
        a_d[i] = a_d[i]-a_d[i-2]
    return a_d
        
        
def main(n,o,e,r):
    
    grid = np.zeros((n+2, n+2))
    for i in range(n):
        for j in range(n):
            if i != j:
                grid[i][j] = totaldist(e[i], j)


    rs = np.zeros((2, n))
    for i in range(n):
        for j in range(2):
            rs[j][i] = totaldist(r[j], i)
    grid[n,:n] = rs[0]
    grid[n+1, :n] = rs[1]
    grid[:n,n] = rs[0]
    grid[:n, n+1] = rs[1]

    mincost=999999
    minr0 = []
    minr1 = []
    minpath0 = []
    minpath1 = []
    for i in range(2**n):
        r0, r1 = binary(i, n)
        r0.append(n)
        r1.append(n+1)
        grid0 = grid[r0].transpose()[r0]
        grid1 = grid[r1].transpose()[r1]
        e0 = grid_to_edges(grid0)
        e1 = grid_to_edges(grid1)
        path_vertices0 = kruskal(e0,list(range(len(r0))),len(r0)-1)
        path_vertices1 = kruskal(e1,list(range(len(r1))),len(r1)-1)
        path0 = [r0[i] for i in path_vertices0]
        path1 = [r1[i] for i in path_vertices1]
        cost0 = cost(grid0, path_vertices0)
        cost1 = cost(grid1, path_vertices1)
        if max(cost0,cost1)<mincost:
            mincost = max(cost1,cost0)
            minr0 = r0
            minr1 = r1
            minpath0 = path0
            minpath1 = path1
    del minpath0[0]
    del minpath1[0]
    route0 = [r[0]]
    route1 = [r[1]]
    for i in minpath0:
        route0.append(o[i])
        route0.append(e[i])
    for i in minpath1:
        route1.append(o[i])
        route1.append(e[i])
    print(route0);
    print(route1);
    a_d0=distance_angle(route0)
    a_d1=distance_angle(route1)
    return(route0,route1,a_d0,a_d1)

if __name__ == "__main__" :
    n = 2
    o = [[7,2],[6,4]]
    e = [[7,3],[6,5]]
    r = [[7,0],[0,0]]
    route0,route1,a_d0,a_d1 = main(n,o,e,r)
    print(a_d0,a_d1)
    
