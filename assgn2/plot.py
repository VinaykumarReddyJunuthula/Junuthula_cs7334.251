import numpy as np
import matplotlib.pyplot as plt

producer_consumer = {
    4:  [45361032, 45260314, 47260257, 45166890, 49104537],
    8:  [64472107, 75597521, 64367514, 75281488, 63878891],
    12: [82849393, 84296579, 81175521, 81695238, 82663307],
    16: [103828823, 104568937, 104170683, 104081159, 103833028],
}

workpool = {
    4:  [426455160, 428334813, 424533256, 429085051, 430928815],
    8:  [793891826, 792873868, 792274315, 792512425, 795878720],
    12: [1432165961, 1414381782, 1431029220, 1469403277, 1442434593],
    16: [1947733234, 1975415813, 1964359181, 1987417465, 1956022692],
}

SECONDS = 120

def compute(data):
    procs = sorted(data.keys())
    avg, minv, maxv = [], [], []

    for p in procs:
        vals = np.array(data[p]) / SECONDS
        avg.append(np.mean(vals))
        minv.append(np.min(vals))
        maxv.append(np.max(vals))

    return procs, np.array(avg), np.array(minv), np.array(maxv)

pc_p, pc_avg, pc_min, pc_max = compute(producer_consumer)
wp_p, wp_avg, wp_min, wp_max = compute(workpool)

x = np.arange(len(pc_p))
w = 0.35

pc_err = np.vstack([pc_avg - pc_min, pc_max - pc_avg])
wp_err = np.vstack([wp_avg - wp_min, wp_max - wp_avg])

plt.figure()

plt.bar(x - w/2, pc_avg, w, yerr=pc_err, capsize=5, label='Producer-Consumer')
plt.bar(x + w/2, wp_avg, w, yerr=wp_err, capsize=5, label='Work-Pool')

plt.xticks(x, pc_p)
plt.xlabel("Number of Processes")
plt.ylabel("Throughput (messages/sec)")
plt.title("Assignment 2 Performance Comparison")
plt.legend()

plt.savefig("chart.png", dpi=300)
plt.show()
