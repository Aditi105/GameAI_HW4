import csv
import math
from collections import Counter, defaultdict

# load samples
with open('monster_data.csv') as f:
    reader = csv.DictReader(f)
    data = list(reader)

# convert fields
for row in data:
    row['room'] = int(row['room'])
    row['dist'] = float(row['dist'])
    row['aggro'] = int(row['aggro'])
    row['wall']  = int(row['wall'])

# possible attributes and splits:
ATTRS = ['room', 'aggro', 'wall']
# for 'dist', we'll threshold at, e.g., 200
def split_dist(rows, thresh):
    return [r for r in rows if r['dist']<=thresh], [r for r in rows if r['dist']>thresh]

# entropy and info gain
def entropy(rows):
    cnt = Counter(r['action'] for r in rows)
    total = len(rows)
    return -sum((c/total)*math.log2(c/total) for c in cnt.values())

def info_gain(parent, subsets):
    H0 = entropy(parent)
    total = len(parent)
    H1 = sum((len(s)/total)*entropy(s) for s in subsets)
    return H0 - H1

# simple ID3
def build_tree(rows, attributes):
    actions = [r['action'] for r in rows]
    if len(set(actions)) == 1:
        return {'label': actions[0]}
    if not attributes:
        # majority vote
        return {'label': Counter(actions).most_common(1)[0][0]}
    # find best split
    best = ('', None, -1)  # attr, split_value, gain
    base = entropy(rows)
    for attr in attributes:
        if attr=='dist':
            for thresh in [100,200,300,400]:
                left, right = split_dist(rows, thresh)
                if not left or not right: continue
                gain = info_gain(rows, [left, right])
                if gain > best[2]: best = (attr, thresh, gain)
        else:
            # nominal
            subsets = []
            for val in set(int(r[attr]) for r in rows):
                subsets.append([r for r in rows if int(r[attr])==val])
            gain = info_gain(rows, subsets)
            if gain > best[2]: best = (attr, None, gain)
    attr, thresh, gain = best
    if gain < 1e-6:
        return {'label': Counter(actions).most_common(1)[0][0]}
    node = {'attr': attr}
    if attr=='dist':
        node['thresh'] = thresh
        left, right = split_dist(rows, thresh)
        node['le'] = build_tree(left, attributes)
        node['gt'] = build_tree(right, attributes)
    else:
        node['branches'] = {}
        for val in set(int(r[attr]) for r in rows):
            subset = [r for r in rows if int(r[attr])==val]
            node['branches'][val] = build_tree(subset, [a for a in attributes if a!=attr])
    return node

tree = build_tree(data, ['room','aggro','wall','dist'])
import json
print(json.dumps(tree, indent=2))
