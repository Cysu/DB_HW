import json

objs = [];

with open('backend/addrdb.json', 'r') as fid:
    for line in fid:
        objs.append(json.loads(line))
    print len(objs)

print objs[0]

obj = objs[0]
print obj['name'], obj['latlng']

