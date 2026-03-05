import pandas as pd
import matplotlib.pyplot as plt

aff = pd.read_csv("results/results_affinity.csv")
aff["policy"] = aff["places"].astype(str) + "-" + aff["bind"].astype(str)

plt.figure()
plt.bar(aff["policy"], aff["time_s"])
plt.xlabel("OMP_PLACES - OMP_PROC_BIND")
plt.ylabel("Execution Time (s)")
plt.title("Affinity Policies (N=10,000,000)")
plt.xticks(rotation=20, ha="right")
plt.tight_layout()
plt.savefig("results/plot_affinity.png", dpi=300)
plt.savefig("results/plot_affinity.pdf")
plt.close()

sc = pd.read_csv("results/results_scaling.csv")
plt.figure()
plt.plot(sc["threads"], sc["time_s"], marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (s)")
plt.title("Strong Scaling (N=10,000,000)")
plt.xticks(sc["threads"])
plt.tight_layout()
plt.savefig("results/plot_scaling.png", dpi=300)
plt.savefig("results/plot_scaling.pdf")
plt.close()

sch = pd.read_csv("results/results_schedule.csv")
pivot = sch.pivot(index="chunk", columns="policy", values="time_s")

plt.figure()
pivot.plot(kind="bar")
plt.xlabel("Chunk Size")
plt.ylabel("Execution Time (s)")
plt.title("Scheduling Policies (Threads = Max, N=10,000,000)")
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig("results/plot_schedule.png", dpi=300)
plt.savefig("results/plot_schedule.pdf")
plt.close()

print("Saved plots to results/: plot_affinity, plot_scaling, plot_schedule (.png and .pdf)")
