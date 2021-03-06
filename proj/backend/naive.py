import json, time

class searcher:
    def __init__(self):
        self.db = []
        self.load_data('addrdb.json')

    def load_data(self, filename):
        with open(filename, 'r') as f:
            for line in f:
                self.db.append(json.loads(line))
        print 'server: finish loading data'

    def query(self, qstr, center_lat, center_lng):
        print 'server: query %s' % qstr

        t0 = time.clock()

        qstr = qstr.lower();

        dist = []
        count = 0

        for loc in self.db:
            if loc['name'].lower().startswith(qstr):
                # record the distance
                lat = float(loc['latlng'][0])
                lng = float(loc['latlng'][1])
                d = (lat-center_lat)**2 + (lng-center_lng)**2
                dist.append((d, count))
            count += 1

        print 'server: result length %d' % len(dist)

        # Trim the most nearest 10 results
        dist.sort()

        n_filter = 10;
        ret = []
        for i in range(0, min(n_filter, len(dist))):
            ret.append(self.db[dist[i][1]]);

        t1 = time.clock()

        return json.dumps({'time': t1-t0, 'result':ret}, separators=(',',':'))
